/*
 * Copyright (C) 2023 Konsulko Group
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef VEHICLE_SIGNALS_CONFIG_H
#define VEHICLE_SIGNALS_CONFIG_H

#include <QObject>

// Class to read/hold VSS server configuration

class VehicleSignalsConfig
{
public:
        explicit VehicleSignalsConfig(const QString &hostname,
				      const unsigned port,
				      const bool useTls,
				      const QString &caCertFileName,
				      const QByteArray &caCert,
				      const QString &tlsServerName,
				      const QString &authToken);
        explicit VehicleSignalsConfig(const QString &appname);
        ~VehicleSignalsConfig() {};

	QString hostname() { return m_hostname; };
	unsigned port() { return m_port; };
	bool useTls() { return m_useTls; };
	QString caCertFileName() { return m_caCertFileName; };
	QByteArray caCert() { return m_caCert; };
	QString tlsServerName() { return m_tlsServerName; };
	QString authToken() { return m_authToken; };
	bool valid() { return m_valid; };
	unsigned verbose() { return m_verbose; };

private:
	QString m_hostname;
	unsigned m_port;
	bool m_useTls;
	QString m_caCertFileName;
	QByteArray m_caCert;
	QString m_tlsServerName;
	QString m_authToken;
	bool m_valid = true;
	unsigned m_verbose = 0;
};

#endif // VEHICLE_SIGNALS_CONFIG_H
