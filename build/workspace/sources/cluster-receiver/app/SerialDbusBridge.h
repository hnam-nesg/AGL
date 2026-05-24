#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

typedef struct _GDBusConnection GDBusConnection;
typedef struct _GVariant GVariant;

class SerialDbusBridge {
public:
	SerialDbusBridge();
	~SerialDbusBridge();

	SerialDbusBridge(const SerialDbusBridge &) = delete;
	SerialDbusBridge &operator=(const SerialDbusBridge &) = delete;

	bool start();
	void stop();

private:
	void run();
	bool ensureDbusConnection();
	bool requestBusNameLocked();
	void emitSignal(const char *signalName, GVariant *parameters);
	void emitSerialState(bool connected, const std::string &detail);
	void setSerialConnected(bool connected, const std::string &detail);
	void emitSpeed(int value);
	void emitCoolant(int value);
	void emitFuel(int value);
	void emitGear(const std::string &gear);
	void emitLights(bool right, bool left, bool high, bool low, bool fog, bool hazard);

	bool openSerialPort();
	void closeSerialPort();
	bool configureSerialPort();
	void processSerialBuffer();
	void handleSerialLine(const std::string &line);
	bool parseGaugeValue(const std::string &line,
			     const std::string &prefix,
			     int minValue,
			     int maxValue,
			     int *value) const;

	std::string m_serialPort;
	int m_baudRate = 115200;
	int m_serialFd = -1;
	std::string m_serialBuffer;
	bool m_serialConnected = false;

	std::atomic_bool m_running{false};
	std::thread m_worker;
	std::mutex m_dbusMutex;
	GDBusConnection *m_connection = nullptr;
};
