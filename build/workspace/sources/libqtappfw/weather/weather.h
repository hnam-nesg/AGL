/*
 * Copyright (C) 2018,2020-2021 Konsulko Group
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

#ifndef WEATHER_H
#define WEATHER_H

#include <memory>
#include <QObject>

class Weather : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString temperature READ temperature NOTIFY temperatureChanged)
	Q_PROPERTY(QString condition READ condition NOTIFY conditionChanged)

	public:
		explicit Weather(QObject * parent = Q_NULLPTR);
		virtual ~Weather();

		QString temperature() { return m_temperature; }
		QString condition() { return m_condition; }

	signals:
		void temperatureChanged(QString temperature);
		void conditionChanged(QString condition);

	private:
		QString m_temperature;
		QString m_condition;

		void onConnected();
		void onDisconnected();
};

#endif // WEATHER_H
