/*
 * Copyright (C) 2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <QDebug>
#include <QSettings>
#include <QUrl>
#include <QFile>
// ?
//#include <QSslKey>
//#include <QTimer>

#include "VehicleSignalsConfig.h"

VehicleSignalsConfig::VehicleSignalsConfig(const QString &hostname,
					   const unsigned port,
					   const bool useTls,
					   const QString &caCertFileName,
					   const QByteArray &caCert,
					   const QString &tlsServerName,
					   const QString &authToken) :
	m_hostname(hostname),
	m_port(port),
	m_useTls(useTls),
	m_caCertFileName(caCertFileName),
	m_caCert(caCert),
	m_tlsServerName(tlsServerName),
	m_authToken(authToken),
	m_valid(true),
	m_verbose(0)
{
	// Potentially could do some certificate validation here...
}

VehicleSignalsConfig::VehicleSignalsConfig(const QString &appname)
{
	m_valid = false;

	QSettings *pSettings = new QSettings("AGL", appname);
	if (!pSettings)
		return;

	m_hostname = pSettings->value("kuksa-client/hostname", "localhost").toString();
	if (m_hostname.isEmpty()) {
		qCritical() << "Invalid server hostname";
		return;
	}

	m_port = pSettings->value("kuksa-client/port", 55555).toInt();
	if (m_port == 0) {
		qCritical() << "Invalid server port";
		return;
	}

	m_useTls = pSettings->value("kuksa-client/use-tls", false).toBool();

	m_caCertFileName = pSettings->value("kuksa-client/ca-certificate", "").toString();
	if (!m_caCertFileName.isEmpty()) {
		QFile caCertFile(m_caCertFileName);
		if (!caCertFile.open(QIODevice::ReadOnly)) {
			qCritical() << "Could not open CA certificate file " << m_caCertFileName;
			return;
		}
		QByteArray caCertData = caCertFile.readAll();
		if (caCertData.isEmpty()) {
			qCritical() << "Invalid CA certificate file";
			return;
		}
		m_caCert = caCertData;
	}

	m_tlsServerName = pSettings->value("kuksa-client/tls-server-name", "").toString();

	QString authTokenFileName = pSettings->value("kuksa-client/authorization").toString();
	if (authTokenFileName.isEmpty()) {
		qCritical() << "Invalid authorization token filename";
		return;
	}
	QFile authTokenFile(authTokenFileName);
	if (!authTokenFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
		qCritical() << "Could not open authorization token file";
		return;
	}
	QTextStream in(&authTokenFile);
	QString authToken = in.readLine();
	if (authToken.isEmpty()) {
		qCritical() << "Invalid authorization token file";
		return;
	}
	m_authToken = authToken;

	m_verbose = 0;
	QString verbose = pSettings->value("kuksa-client/verbose").toString();
	if (!verbose.isEmpty()) {
		if (verbose == "true" || verbose == "1")
			m_verbose = 1;
		if (verbose == "2")
			m_verbose = 2;
	}

	m_valid = true;
}
