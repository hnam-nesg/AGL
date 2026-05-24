// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2023 Konsulko Group
 */

#ifndef RADIO_GRPC_CLIENT_H
#define RADIO_GRPC_CLIENT_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QThread>
#include <grpcpp/grpcpp.h>

#include "radio.grpc.pb.h"

using grpc::Channel;

class RadioStatusEventReader : public QObject
{
	Q_OBJECT
public:
	RadioStatusEventReader(std::shared_ptr<automotivegradelinux::Radio::Stub> &stub,
			       QObject *parent = Q_NULLPTR) : QObject(parent), stub_(stub) {}

public slots:
	void GetStatusEvents();

signals:
	void bandUpdate(int band);
	void frequencyUpdate(int frequency);
	void playingUpdate(bool status);
	void scanningUpdate(bool scanning);

	void finished();

private:
	std::shared_ptr<automotivegradelinux::Radio::Stub> stub_;
};

class RadioGrpcClient : public QObject
{
	Q_OBJECT

public:
	RadioGrpcClient(QObject *parent = Q_NULLPTR);

	void SetBand(unsigned int band);
	void SetFrequency(unsigned int frequency);
	void Start();
	void Stop();
	void ScanForward();
	void ScanBackward();
	void ScanStop();
	void GetBandParameters(unsigned int band,
			       unsigned int &min,
			       unsigned int &max,
			       unsigned int &step);

private:
	std::shared_ptr<automotivegradelinux::Radio::Stub> stub_;

	QThread m_event_thread;

};

#endif // RADIO_GRPC_CLIENT_H
