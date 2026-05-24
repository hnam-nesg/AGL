/*
 * Copyright (C) 2020-2022 Konsulko Group
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

#ifndef HVAC_H
#define HVAC_H

#include <memory>
#include <QObject>
#include <QJsonArray>
#include <QtQml/QQmlContext>
#include <QtQml/QQmlListProperty>

class VehicleSignals;

class HVAC : public QObject
{
	Q_OBJECT

	Q_PROPERTY(int fanSpeed READ get_fanspeed WRITE set_fanspeed NOTIFY fanSpeedChanged)
	Q_PROPERTY(int leftTemperature READ get_temp_left_zone WRITE set_temp_left_zone NOTIFY leftTemperatureChanged)
	Q_PROPERTY(int rightTemperature READ get_temp_right_zone WRITE set_temp_right_zone NOTIFY rightTemperatureChanged)
        ////////
        Q_PROPERTY(bool syncTemp READ getSyncTemp WRITE setSyncTemp NOTIFY syncTempChanged)
        Q_PROPERTY(bool rearDefrost READ getRearDefrost WRITE setRearDefrost NOTIFY rearDefrostChanged)
        Q_PROPERTY(bool frontDefrost READ getFrontDefrost WRITE setFrontDefrost NOTIFY frontDefrostChanged)

        Q_PROPERTY(QString airDistribution READ getAirDistribution WRITE setAirDistribution NOTIFY airDistributionChanged)

        Q_PROPERTY(bool airRecir READ getAirRecir WRITE setAirRecir NOTIFY airRecirChanged)
        
        Q_PROPERTY(bool airAuto READ getAirAuto WRITE setAirAuto NOTIFY airAutoChanged)

        Q_PROPERTY(bool airCondition READ getAirCondition WRITE setAirCondition NOTIFY airConditionChanged)

        Q_PROPERTY(int seatDriver READ getSeatDriver WRITE setSeatDriver NOTIFY seatDriverChanged)
        Q_PROPERTY(int seatPassenger READ getSeatPassenger WRITE setSeatPassenger NOTIFY seatPassengerChanged)
        Q_PROPERTY(int seatBehindDriver READ getSeatBehindDriver WRITE setSeatBehindDriver NOTIFY seatBehindDriverChanged)
        Q_PROPERTY(int seatBehindPassenger READ getSeatBehindPassenger WRITE setSeatBehindPassenger NOTIFY seatBehindPassengerChanged)

public:
	explicit HVAC(VehicleSignals *vs, QObject * parent = Q_NULLPTR);
        virtual ~HVAC();

        bool getRearDefrost() const{return m_reardefrost;};
        void setRearDefrost(bool active);

        bool getFrontDefrost() const{return m_frontdefrost;};
        void setFrontDefrost(bool active);

        QString getAirDistribution() const{return m_airDistribution;};
        void setAirDistribution(QString mode);

        bool getAirRecir() const {return m_airrecir;};
        void setAirRecir(bool active);

        bool getAirAuto() const {return m_airauto;};
        void setAirAuto(bool active);

        bool getAirCondition() const{return m_aircondition;};
        void setAirCondition(bool active);

        int getSeatDriver() const{return m_seatdriver;};
        void setSeatDriver(int level);
        int getSeatPassenger() const{return m_seatpassenger;};
        void setSeatPassenger(int level);
        int getSeatBehindDriver() const{return m_seatbehinddriver;};
        void setSeatBehindDriver(int level);
        int getSeatBehindPassenger() const{return m_seatbehindpassenger;};
        void setSeatBehindPassenger(int level);

signals:
        void fanSpeedChanged(int fanSpeed);
        void leftTemperatureChanged(int temp);
        void rightTemperatureChanged(int temp);
        void languageChanged(QString language);

        void syncTempChanged(bool sync);
        void rearDefrostChanged(bool active);
        void frontDefrostChanged(bool active);
        void airDistributionChanged(QString mode);
        void airRecirChanged(bool active);
        void airAutoChanged(bool active);
        void airConditionChanged(bool active);

        void seatDriverChanged(int level);
        void seatPassengerChanged(int level);
        void seatBehindDriverChanged(int level);
        void seatBehindPassengerChanged(int level);

private slots:
	void onConnected();

private:
	VehicleSignals *m_vs;
	bool m_connected;

        int m_fanspeed;
        int m_temp_left_zone;
        int m_temp_right_zone;

        bool m_synctemp;

        bool m_reardefrost;
        bool m_frontdefrost;

        QString m_airDistribution;
        bool m_airrecir;
        bool m_aircondition;
        bool m_airauto;

        int m_seatdriver;
        int m_seatpassenger;
        int m_seatbehinddriver;
        int m_seatbehindpassenger;

        int get_fanspeed() const { return m_fanspeed; };
        int get_temp_left_zone() const { return m_temp_left_zone; };
        int get_temp_right_zone() const { return m_temp_right_zone; };

        bool getSyncTemp() const{return m_synctemp;};


        void set_fanspeed(int speed);
        void set_temp_left_zone(int temp);
        void set_temp_right_zone(int temp);

        void setSyncTemp(bool sync);
};

#endif // HVAC_H
