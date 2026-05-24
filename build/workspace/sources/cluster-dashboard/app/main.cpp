/*
 * Copyright (C) 2016 The Qt Company Ltd.
 * Copyright (C) 2018,2019,2020,2022,2025 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QtCore/QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QHash>
#include <QtCore/QCommandLineParser>
#include <QUrl>
#include <QtCore/QUrlQuery>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlComponent>
#include <QtQml/qqml.h>
#include <QQuickWindow>
#include <QSGRendererInterface>
#include <QtWebEngineQuick>
#include <QtQuickControls2/QQuickStyle>
#include <qpa/qplatformnativeinterface.h>
#include <QTimer>
#include <glib.h>
#include <QDebug>
#include <QScreen>
#include <QStringList>
#include <QTextStream>
#include <QProcessEnvironment>

#include <wayland-client.h>
#include "agl-shell-client-protocol.h"

#include <vehiclesignals.h>

#include "myCluster/header_file/cluster.h"
#include "myCluster/header_file/fpslogger.h"
#include "myCluster/header_file/fuelarc.h"
#include "myCluster/header_file/goongrestclient.h"
#include "myCluster/header_file/startupnotifier.h"
#include "myCluster/header_file/ShimmerLogoItem.h"
#include "myCluster/header_file/PhoneNavigationBridge.h"

#include <cstdlib>
#include <cstring>
#include <memory>

// Global indicating whether canned animation should run
bool runAnimation = true;

namespace {

QString directoryUrl(const QString &path)
{
	QDir dir(path);
	QString absolutePath = dir.absolutePath();
	if (!absolutePath.endsWith(QLatin1Char('/')))
		absolutePath.append(QLatin1Char('/'));
	return QUrl::fromLocalFile(absolutePath).toString();
}

QString loadDashboardFontFamily()
{
	const int fontId = QFontDatabase::addApplicationFont(QStringLiteral(":/asset/fonts/Roboto-BoldCondensed.ttf"));
	if (fontId >= 0) {
		const QStringList families = QFontDatabase::applicationFontFamilies(fontId);
		if (!families.isEmpty())
			return families.first();
	}

	return QStringLiteral("Sans Serif");
}

bool has3DAssetLayout(const QString &path)
{
	const QDir dir(path);
	return dir.exists(QStringLiteral("meshes"))
		&& dir.exists(QStringLiteral("maps"))
		&& dir.exists(QStringLiteral("textures"));
}

QString resolveMyClusterAssetRootPath()
{
	QStringList candidates;
	const QByteArray envAssetRoot = qgetenv("MYCLUSTER_ASSET_ROOT");
	if (!envAssetRoot.isEmpty())
		candidates << QString::fromLocal8Bit(envAssetRoot);

	const QString appDir = QCoreApplication::applicationDirPath();
	candidates << appDir + QStringLiteral("/asset")
		   << appDir + QStringLiteral("/../share/cluster-dashboard/asset")
		   << appDir + QStringLiteral("/../asset")
#ifdef MYCLUSTER_INSTALL_ASSET_DIR
		   << QStringLiteral(MYCLUSTER_INSTALL_ASSET_DIR)
#endif
#ifdef MYCLUSTER_SOURCE_ASSET_DIR
		   << QStringLiteral(MYCLUSTER_SOURCE_ASSET_DIR)
#endif
		   << QStringLiteral("/fs/nvme/apps/asset")
		   << QStringLiteral("/fs/nvme/apps/myCluster/asset")
		   << QStringLiteral("/fs/nvme/asset");

	for (const QString &candidate : candidates) {
		if (has3DAssetLayout(candidate))
			return QDir(candidate).absolutePath();
	}

	return QDir(appDir + QStringLiteral("/asset")).absolutePath();
}

QString unquoteDefaultValue(const QString &rawValue)
{
	QString value = rawValue.trimmed();
	if (value.isEmpty())
		return value;

	const QChar first = value.at(0);
	if (first == QLatin1Char('"') || first == QLatin1Char('\'')) {
		const int close = value.indexOf(first, 1);
		if (close > 0)
			return value.mid(1, close - 1);
	}

	const int comment = value.indexOf(QLatin1Char('#'));
	if (comment >= 0)
		value = value.left(comment).trimmed();

	return value;
}

QHash<QString, QString> loadDefaultEnvironmentFile(const QString &path)
{
	QHash<QString, QString> values;
	QFile file(path);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return values;

	QTextStream in(&file);
	while (!in.atEnd()) {
		QString line = in.readLine().trimmed();
		if (line.isEmpty() || line.startsWith(QLatin1Char('#')))
			continue;

		if (line.startsWith(QStringLiteral("export ")))
			line = line.mid(7).trimmed();

		const int equals = line.indexOf(QLatin1Char('='));
		if (equals <= 0)
			continue;

		const QString key = line.left(equals).trimmed();
		if (key.isEmpty())
			continue;

		values.insert(key, unquoteDefaultValue(line.mid(equals + 1)));
	}

	return values;
}

QString configuredValue(const QProcessEnvironment &environment,
			const QHash<QString, QString> &defaults,
			const QString &key,
			const QString &fallback = QString())
{
	const QString envValue = environment.value(key);
	if (!envValue.isEmpty())
		return envValue;

	return defaults.value(key, fallback);
}

void appendChromiumFlag(QByteArray *flags, const QByteArray &flag)
{
	if (!flags || flags->contains(flag))
		return;

	if (!flags->isEmpty())
		flags->append(' ');
	flags->append(flag);
}

void configureMapLibreWebEngine()
{
	QByteArray chromiumFlags = qgetenv("QTWEBENGINE_CHROMIUM_FLAGS");
	appendChromiumFlag(&chromiumFlags, QByteArrayLiteral("--disable-vulkan"));
	appendChromiumFlag(&chromiumFlags, QByteArrayLiteral("--disable-features=Vulkan"));
	appendChromiumFlag(&chromiumFlags, QByteArrayLiteral("--use-gl=egl"));
	appendChromiumFlag(&chromiumFlags, QByteArrayLiteral("--ignore-gpu-blocklist"));
	appendChromiumFlag(&chromiumFlags, QByteArrayLiteral("--enable-webgl"));
	appendChromiumFlag(&chromiumFlags, QByteArrayLiteral("--disable-gpu-sandbox"));

	qputenv("QTWEBENGINE_CHROMIUM_FLAGS", chromiumFlags);
	qputenv("QTWEBENGINE_DISABLE_SANDBOX", QByteArrayLiteral("1"));
	qputenv("QSG_RHI_BACKEND", QByteArrayLiteral("opengl"));
	qputenv("QT_OPENGL", QByteArrayLiteral("es2"));

	QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
	QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
	QtWebEngineQuick::initialize();
}

} // namespace

static void
global_add(void *data, struct wl_registry *reg, uint32_t name,
	   const char *interface, uint32_t version)
{
	struct agl_shell **shell = static_cast<struct agl_shell **>(data);
	if (strcmp(interface, agl_shell_interface.name) == 0) {
		*shell = static_cast<struct agl_shell *>(wl_registry_bind(reg,
					name, &agl_shell_interface, 1)
		);
	}
}

static void
global_remove(void *data, struct wl_registry *reg, uint32_t id)
{
	(void) data;
	(void) reg;
	(void) id;
}

static const struct wl_registry_listener registry_listener = {
	global_add,
	global_remove,
};

static struct agl_shell *
register_agl_shell(QPlatformNativeInterface *native)
{
	struct wl_display *wl;
	struct wl_registry *registry;
	struct agl_shell *shell = nullptr;

	wl = static_cast<struct wl_display *>(native->nativeResourceForIntegration("display"));
	registry = wl_display_get_registry(wl);

	wl_registry_add_listener(registry, &registry_listener, &shell);
	wl_display_roundtrip(wl);
	wl_registry_destroy(registry);

	return shell;
}

static struct wl_surface *
getWlSurface(QPlatformNativeInterface *native, QWindow *window)
{
	void *surf = native->nativeResourceForWindow("surface", window);
	return static_cast<struct ::wl_surface *>(surf);
}

static struct wl_output *
getWlOutput(QPlatformNativeInterface *native, QScreen *screen)
{
	void *output = native->nativeResourceForScreen("output", screen);
	return static_cast<struct ::wl_output*>(output);
}

void read_config(void)
{
	GKeyFile* conf_file;
	gboolean value;

	// Load settings from configuration file if it exists
	conf_file = g_key_file_new();
	if(conf_file &&
	   g_key_file_load_from_dirs(conf_file,
				     "AGL.conf",
				     (const gchar**) g_get_system_config_dirs(),
				     NULL,
				     G_KEY_FILE_KEEP_COMMENTS,
				     NULL) == TRUE) {
		GError *err = NULL;
		value = g_key_file_get_boolean(conf_file,
					       "dashboard",
					       "animation",
					       &err);
		if(value) {
			runAnimation = true;
		} else {
			if(err == NULL) {
				runAnimation = false;
			} else {
				qWarning("Invalid value for \"animation\" key!");
			}
		}
	}

}

static struct wl_surface *
create_component(QPlatformNativeInterface *native, QQmlComponent *comp,
		QScreen *screen, QObject **qobj)
{
	QObject *obj = comp->create();
	if (!obj) {
		qCritical() << "Failed to create dashboard component:" << comp->errors();
		exit(EXIT_FAILURE);
	}

	obj->setParent(screen);

	QWindow *win = qobject_cast<QWindow *>(obj);
	if (!win) {
		qCritical() << "Dashboard component root is not a QWindow";
		exit(EXIT_FAILURE);
	}

	if (screen) {
		win->setScreen(screen);
		win->resize(screen->geometry().size());
	}

	*qobj = obj;

	return getWlSurface(native, win);
}

static QScreen *find_screen(const char *screen_name)
{
	QList<QScreen *> screens = qApp->screens();
	QString name(screen_name);

	for (QScreen *screen : screens) {
		if (name == screen->name())
			return screen;
	}

	return nullptr;
}

int main(int argc, char *argv[])
{
	QString myname = QString("cluster-dashboard");
	struct agl_shell *agl_shell;
	struct wl_output *output;

	QObject *qobj_bg;
	QScreen *screen;

	configureMapLibreWebEngine();

	QGuiApplication app(argc, argv);
	app.setDesktopFileName(myname);
	QPlatformNativeInterface *native = qApp->platformNativeInterface();

	agl_shell = register_agl_shell(native);
	if (!agl_shell) {
		exit(EXIT_FAILURE);
	}

	std::shared_ptr<struct agl_shell> shell{agl_shell, agl_shell_destroy};

	const char *screen_name = getenv("DASHBOARD_START_SCREEN");
	if (screen_name)
		screen = find_screen(screen_name);
	else
		screen = qApp->primaryScreen();
	output = getWlOutput(native, screen);

	read_config();

	QQmlApplicationEngine engine;
	QQmlContext *context = engine.rootContext();
	context->setContextProperty("runAnimation", runAnimation);

	VehicleSignalsConfig vsConfig(myname);
	context->setContextProperty("VehicleSignals", new VehicleSignals(vsConfig));

	const QString dashboardFontFamily = loadDashboardFontFamily();
	QGuiApplication::setFont(QFont(dashboardFontFamily));

	const QString assetRootPath = resolveMyClusterAssetRootPath();
	const bool external3DAssetsAvailable = has3DAssetLayout(assetRootPath);
	const QByteArray enable3DEnv = qgetenv("MYCLUSTER_ENABLE_3D");
	const bool external3DEnabled = MYCLUSTER_BUILD_3D && (enable3DEnv.isEmpty() || enable3DEnv != "0");
	const QProcessEnvironment processEnv = QProcessEnvironment::systemEnvironment();
	const QHash<QString, QString> goongDefaults =
		loadDefaultEnvironmentFile(QStringLiteral("/etc/default/navigation-goong"));
	const QString goongRestApiKey =
		configuredValue(processEnv, goongDefaults, QStringLiteral("GOONG_REST_API_KEY"));
	const QString goongMapTilesKey =
		configuredValue(processEnv, goongDefaults, QStringLiteral("GOONG_MAPTILES_KEY"));
	const QString goongStyleUrl =
		configuredValue(processEnv,
				goongDefaults,
				QStringLiteral("GOONG_STYLE_URL"),
				QStringLiteral("https://tiles.goong.io/assets/goong_map_web.json"));

	Cluster cluster;
	FpsLogger fpsLogger;
	GoongRestClient goongRestClient;
	StartupNotifier startupNotifier;
	goongRestClient.setApiKey(goongRestApiKey);

	PhoneNavigationBridge phoneNavigationBridge;
	phoneNavigationBridge.setPort(8765);
	phoneNavigationBridge.setGoongApiKey(goongRestApiKey);
	context->setContextProperty("phoneNavigationBridge", &phoneNavigationBridge);

	context->setContextProperty("Cluster", &cluster);
	context->setContextProperty("fpsLogger", &fpsLogger);
	context->setContextProperty("GoongRestClient", &goongRestClient);
	context->setContextProperty("startupNotifier", &startupNotifier);
	context->setContextProperty("dashboardTextFontFamily", dashboardFontFamily);
	context->setContextProperty("externalAssetRootPath", assetRootPath);
	context->setContextProperty("externalAssetRootUrl", directoryUrl(assetRootPath));
	context->setContextProperty("external3DAssetsAvailable", external3DAssetsAvailable);
	context->setContextProperty("external3DEnabled", external3DEnabled);
	context->setContextProperty(QStringLiteral("GoongMapTilesKey"), goongMapTilesKey);
	context->setContextProperty(QStringLiteral("GoongRestApiKey"), goongRestApiKey);
	context->setContextProperty(QStringLiteral("GoongStyleUrl"), goongStyleUrl);
	context->setContextProperty(
		QStringLiteral("ClusterMapCenterLat"),
		processEnv.value(QStringLiteral("CLUSTER_MAP_LAT")));
	context->setContextProperty(
		QStringLiteral("ClusterMapCenterLon"),
		processEnv.value(QStringLiteral("CLUSTER_MAP_LON")));
	context->setContextProperty(
		QStringLiteral("ClusterMapZoom"),
		processEnv.value(QStringLiteral("CLUSTER_MAP_ZOOM")));
	context->setContextProperty(
		QStringLiteral("ClusterMapPitch"),
		processEnv.value(QStringLiteral("CLUSTER_MAP_PITCH")));
	context->setContextProperty(
		QStringLiteral("ClusterMapBearing"),
		processEnv.value(QStringLiteral("CLUSTER_MAP_BEARING")));

	qmlRegisterType<FuelArcImageOverlay>("Fuel", 1, 0, "FuelShader");
	qmlRegisterType<CoolantArcImageOverlay>("Fuel", 1, 0, "CoolantShader");
	qmlRegisterType<ShimmerLogoItem>("ClusterSplash", 1, 0, "ShimmerLogo");

	QQmlComponent bg_comp(&engine, QUrl("qrc:/myCluster/Main.qml"));
	qDebug() << bg_comp.errors();
	struct wl_surface *bg = create_component(native, &bg_comp, screen, &qobj_bg);
	if (QQuickWindow *window = qobject_cast<QQuickWindow *>(qobj_bg))
		startupNotifier.setWindow(window);

	// set the surface as the background
	agl_shell_set_background(agl_shell, bg, output);

	// instruct the compositor it can display after Qt has a chance
	// to load everything
	QTimer::singleShot(500, [agl_shell](){
		qDebug() << "agl_shell ready!";
		agl_shell_ready(agl_shell);
	});

	return app.exec();
}
