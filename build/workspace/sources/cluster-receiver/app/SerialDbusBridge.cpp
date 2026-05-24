#include "SerialDbusBridge.h"

#include <gio/gio.h>
#include <glib.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <poll.h>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <vector>

namespace {

constexpr char kDefaultSerialPort[] =
	"/dev/serial/by-id/usb-Arduino__www.arduino.cc__Arduino_Uno_12250481010676106650-if00";
constexpr char kBusName[] = "org.agl.ClusterReceiver";
constexpr char kObjectPath[] = "/org/agl/ClusterReceiver";
constexpr char kInterface[] = "org.agl.ClusterReceiver.Serial";

std::string envString(const char *key, const char *fallback)
{
	const char *value = std::getenv(key);
	if (value && value[0] != '\0')
		return value;
	return fallback;
}

int envInt(const char *key, int fallback)
{
	const char *value = std::getenv(key);
	if (!value || value[0] == '\0')
		return fallback;

	char *end = nullptr;
	const long parsed = std::strtol(value, &end, 10);
	if (end == value || parsed <= 0)
		return fallback;

	return static_cast<int>(parsed);
}

std::string trim(const std::string &value)
{
	const char *spaces = " \t\r\n";
	const std::string::size_type begin = value.find_first_not_of(spaces);
	if (begin == std::string::npos)
		return {};

	const std::string::size_type end = value.find_last_not_of(spaces);
	return value.substr(begin, end - begin + 1);
}

std::string lowerCopy(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
		return static_cast<char>(std::tolower(ch));
	});
	return value;
}

bool startsWithCaseInsensitive(const std::string &value, const std::string &prefix)
{
	if (value.size() < prefix.size())
		return false;

	return lowerCopy(value.substr(0, prefix.size())) == lowerCopy(prefix);
}

bool equalsCaseInsensitive(const std::string &left, const std::string &right)
{
	return lowerCopy(trim(left)) == lowerCopy(trim(right));
}

std::vector<std::string> split(const std::string &value, char separator)
{
	std::vector<std::string> parts;
	std::stringstream stream(value);
	std::string part;

	while (std::getline(stream, part, separator))
		parts.push_back(trim(part));

	return parts;
}

speed_t baudConstant(int baudRate)
{
	switch (baudRate) {
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
#ifdef B230400
	case 230400:
		return B230400;
#endif
	default:
		g_warning("Unsupported CLUSTER_SERIAL_BAUD=%d, fallback to 115200", baudRate);
		return B115200;
	}
}

} // namespace

SerialDbusBridge::SerialDbusBridge()
	: m_serialPort(envString("CLUSTER_SERIAL_PORT", kDefaultSerialPort)),
	  m_baudRate(envInt("CLUSTER_SERIAL_BAUD", 115200))
{
}

SerialDbusBridge::~SerialDbusBridge()
{
	stop();
}

bool SerialDbusBridge::start()
{
	bool expected = false;
	if (!m_running.compare_exchange_strong(expected, true))
		return true;

	ensureDbusConnection();
	m_worker = std::thread(&SerialDbusBridge::run, this);
	return true;
}

void SerialDbusBridge::stop()
{
	bool expected = true;
	if (!m_running.compare_exchange_strong(expected, false))
		return;

	if (m_worker.joinable())
		m_worker.join();

	closeSerialPort();

	std::lock_guard<std::mutex> lock(m_dbusMutex);
	if (m_connection) {
		g_object_unref(m_connection);
		m_connection = nullptr;
	}
}

bool SerialDbusBridge::ensureDbusConnection()
{
	std::lock_guard<std::mutex> lock(m_dbusMutex);
	if (m_connection)
		return true;

	GError *error = nullptr;
	m_connection = g_bus_get_sync(G_BUS_TYPE_SESSION, nullptr, &error);
	if (!m_connection) {
		g_warning("cluster-receiver D-Bus session connection failed: %s",
			  error ? error->message : "unknown error");
		g_clear_error(&error);
		m_connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, nullptr, &error);
	}

	if (!m_connection) {
		g_warning("cluster-receiver D-Bus system connection failed: %s",
			  error ? error->message : "unknown error");
		g_clear_error(&error);
		return false;
	}

	requestBusNameLocked();
	return true;
}

bool SerialDbusBridge::requestBusNameLocked()
{
	GError *error = nullptr;
	GVariant *reply = g_dbus_connection_call_sync(
		m_connection,
		"org.freedesktop.DBus",
		"/org/freedesktop/DBus",
		"org.freedesktop.DBus",
		"RequestName",
		g_variant_new("(su)", kBusName, 0u),
		G_VARIANT_TYPE("(u)"),
		G_DBUS_CALL_FLAGS_NONE,
		-1,
		nullptr,
		&error);

	if (!reply) {
		g_warning("cluster-receiver D-Bus RequestName failed: %s",
			  error ? error->message : "unknown error");
		g_clear_error(&error);
		return false;
	}

	guint32 result = 0;
	g_variant_get(reply, "(u)", &result);
	g_variant_unref(reply);
	g_message("cluster-receiver D-Bus name %s request result=%u", kBusName, result);
	return true;
}

void SerialDbusBridge::emitSignal(const char *signalName, GVariant *parameters)
{
	if (!ensureDbusConnection()) {
		if (parameters)
			g_variant_unref(g_variant_ref_sink(parameters));
		return;
	}

	GVariant *params = parameters ? g_variant_ref_sink(parameters) : nullptr;
	GError *error = nullptr;
	{
		std::lock_guard<std::mutex> lock(m_dbusMutex);
		if (m_connection) {
			const gboolean emitted = g_dbus_connection_emit_signal(
				m_connection,
				nullptr,
				kObjectPath,
				kInterface,
				signalName,
				params,
				&error);
			if (!emitted && !error)
				g_warning("cluster-receiver failed to emit D-Bus signal %s", signalName);
		}
	}

	if (error) {
		g_warning("cluster-receiver failed to emit D-Bus signal %s: %s",
			  signalName,
			  error->message);
		g_clear_error(&error);
	}

	if (params)
		g_variant_unref(params);
}

void SerialDbusBridge::emitSerialState(bool connected, const std::string &detail)
{
	emitSignal("SerialStateChanged", g_variant_new("(bs)", connected, detail.c_str()));
}

void SerialDbusBridge::setSerialConnected(bool connected, const std::string &detail)
{
	if (m_serialConnected == connected)
		return;

	m_serialConnected = connected;
	emitSerialState(connected, detail);
}

void SerialDbusBridge::emitSpeed(int value)
{
	g_message("cluster-receiver serial speed=%d", value);
	emitSignal("SpeedChanged", g_variant_new("(i)", value));
}

void SerialDbusBridge::emitCoolant(int value)
{
	g_message("cluster-receiver serial coolant=%d", value);
	emitSignal("CoolantChanged", g_variant_new("(i)", value));
}

void SerialDbusBridge::emitFuel(int value)
{
	g_message("cluster-receiver serial fuel=%d", value);
	emitSignal("FuelChanged", g_variant_new("(i)", value));
}

void SerialDbusBridge::emitGear(const std::string &gear)
{
	g_message("cluster-receiver serial gear=%s", gear.c_str());
	emitSignal("GearChanged", g_variant_new("(s)", gear.c_str()));
}

void SerialDbusBridge::emitLights(bool right, bool left, bool high, bool low, bool fog, bool hazard)
{
	g_message("cluster-receiver serial lights right=%d left=%d high=%d low=%d fog=%d hazard=%d",
		  right,
		  left,
		  high,
		  low,
		  fog,
		  hazard);
	emitSignal("LightsChanged", g_variant_new("(bbbbbb)", right, left, high, low, fog, hazard));
}

bool SerialDbusBridge::openSerialPort()
{
	if (m_serialFd >= 0)
		return true;

	m_serialFd = open(m_serialPort.c_str(), O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC);
	if (m_serialFd < 0) {
		g_warning("cluster-receiver serial open failed: port=%s error=%s",
			  m_serialPort.c_str(),
			  std::strerror(errno));
		return false;
	}

	if (!configureSerialPort()) {
		closeSerialPort();
		return false;
	}

	m_serialBuffer.clear();
	g_message("cluster-receiver serial connected: port=%s baud=%d",
		  m_serialPort.c_str(),
		  m_baudRate);
	setSerialConnected(true, m_serialPort);
	return true;
}

void SerialDbusBridge::closeSerialPort()
{
	if (m_serialFd >= 0) {
		close(m_serialFd);
		m_serialFd = -1;
	}
	setSerialConnected(false, m_serialPort);
}

bool SerialDbusBridge::configureSerialPort()
{
	struct termios tty = {};
	if (tcgetattr(m_serialFd, &tty) != 0) {
		g_warning("cluster-receiver serial tcgetattr failed: %s", std::strerror(errno));
		return false;
	}

	cfmakeraw(&tty);
	const speed_t speed = baudConstant(m_baudRate);
	cfsetispeed(&tty, speed);
	cfsetospeed(&tty, speed);

	tty.c_cflag |= CLOCAL | CREAD;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag |= CS8;
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
#ifdef CRTSCTS
	tty.c_cflag &= ~CRTSCTS;
#endif
	tty.c_cc[VMIN] = 0;
	tty.c_cc[VTIME] = 0;

	if (tcsetattr(m_serialFd, TCSANOW, &tty) != 0) {
		g_warning("cluster-receiver serial tcsetattr failed: %s", std::strerror(errno));
		return false;
	}

	tcflush(m_serialFd, TCIFLUSH);
	return true;
}

void SerialDbusBridge::run()
{
	auto lastNoDataLog = std::chrono::steady_clock::now();

	while (m_running.load()) {
		if (!openSerialPort()) {
			setSerialConnected(false, m_serialPort);
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		struct pollfd pfd = {};
		pfd.fd = m_serialFd;
		pfd.events = POLLIN | POLLERR | POLLHUP;

		const int pollResult = poll(&pfd, 1, 500);
		if (!m_running.load())
			break;

		if (pollResult < 0) {
			if (errno == EINTR)
				continue;
			g_warning("cluster-receiver serial poll failed: %s", std::strerror(errno));
			closeSerialPort();
			continue;
		}

		if (pollResult == 0) {
			const auto now = std::chrono::steady_clock::now();
			if (now - lastNoDataLog > std::chrono::seconds(2)) {
				g_message("cluster-receiver serial waiting for data: port=%s",
					  m_serialPort.c_str());
				lastNoDataLog = now;
			}
			continue;
		}

		if (pfd.revents & POLLIN) {
			char chunk[512];
			while (true) {
				const ssize_t bytesRead = read(m_serialFd, chunk, sizeof(chunk));
				if (bytesRead > 0) {
					m_serialBuffer.append(chunk, static_cast<size_t>(bytesRead));
					processSerialBuffer();
					lastNoDataLog = std::chrono::steady_clock::now();
					continue;
				}

				if (bytesRead == 0 || errno == EAGAIN || errno == EWOULDBLOCK)
					break;

				if (errno == EINTR)
					continue;

				g_warning("cluster-receiver serial read failed: %s", std::strerror(errno));
				closeSerialPort();
				break;
			}
		}

		if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
			g_warning("cluster-receiver serial disconnected: revents=0x%x", pfd.revents);
			closeSerialPort();
		}
	}

	closeSerialPort();
}

void SerialDbusBridge::processSerialBuffer()
{
	std::string::size_type endIndex = std::string::npos;
	while ((endIndex = m_serialBuffer.find('\n')) != std::string::npos) {
		const std::string line = trim(m_serialBuffer.substr(0, endIndex));
		m_serialBuffer.erase(0, endIndex + 1);

		if (!line.empty())
			handleSerialLine(line);
	}
}

void SerialDbusBridge::handleSerialLine(const std::string &line)
{
	if (startsWithCaseInsensitive(line, "Lights:")) {
		const auto parts = split(line.substr(line.find(':') + 1), ',');
		if (parts.size() >= 6) {
			emitLights(equalsCaseInsensitive(parts[0], "Right"),
				   equalsCaseInsensitive(parts[1], "Left"),
				   equalsCaseInsensitive(parts[2], "High"),
				   equalsCaseInsensitive(parts[3], "Low"),
				   equalsCaseInsensitive(parts[4], "Fog"),
				   equalsCaseInsensitive(parts[5], "Hazard"));
		} else {
			g_warning("cluster-receiver invalid Lights line: %s", line.c_str());
		}
		return;
	}

	int value = 0;
	if (parseGaugeValue(line, "Speed:", 0, 220, &value)) {
		emitSpeed(value);
	} else if (parseGaugeValue(line, "Coolant:", 0, 100, &value)) {
		emitCoolant(value);
	} else if (parseGaugeValue(line, "Fuel:", 0, 100, &value)) {
		emitFuel(value);
	} else if (startsWithCaseInsensitive(line, "Gear:")) {
		const std::string gear = trim(line.substr(line.find(':') + 1));
		if (!gear.empty())
			emitGear(gear);
	} else {
		g_warning("cluster-receiver ignored serial line: %s", line.c_str());
	}
}

bool SerialDbusBridge::parseGaugeValue(const std::string &line,
				       const std::string &prefix,
				       int minValue,
				       int maxValue,
				       int *value) const
{
	if (!startsWithCaseInsensitive(line, prefix))
		return false;

	const std::string rawValue = trim(line.substr(prefix.size()));
	size_t numericLength = 0;
	for (; numericLength < rawValue.size(); ++numericLength) {
		const char ch = rawValue[numericLength];
		const bool signAtStart = numericLength == 0 && (ch == '+' || ch == '-');
		if (!std::isdigit(static_cast<unsigned char>(ch)) && ch != '.' && !signAtStart)
			break;
	}

	if (numericLength == 0) {
		g_warning("cluster-receiver invalid serial value: %s", line.c_str());
		return false;
	}

	try {
		const double parsedValue = std::stod(rawValue.substr(0, numericLength));
		const int roundedValue = static_cast<int>(std::lround(parsedValue));
		if (roundedValue < minValue || roundedValue > maxValue) {
			g_warning("cluster-receiver serial value out of range: %s", line.c_str());
			return false;
		}

		*value = roundedValue;
		return true;
	} catch (const std::exception &) {
		g_warning("cluster-receiver invalid serial value: %s", line.c_str());
		return false;
	}
}
