// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (C) 2023 Konsulko Group
 */

#include <QDebug>
#include "RadioGrpcClient.h"
#include "RadioClient.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;

using automotivegradelinux::Radio;
using automotivegradelinux::SetBandRequest;
using automotivegradelinux::SetBandResponse;
using automotivegradelinux::SetFrequencyRequest;
using automotivegradelinux::SetFrequencyResponse;
using automotivegradelinux::StartRequest;
using automotivegradelinux::StartResponse;
using automotivegradelinux::StopRequest;
using automotivegradelinux::StopResponse;
using automotivegradelinux::ScanStartRequest;
using automotivegradelinux::ScanStartResponse;
using automotivegradelinux::ScanStopRequest;
using automotivegradelinux::ScanStopResponse;
using automotivegradelinux::GetBandParametersRequest;
using automotivegradelinux::GetBandParametersResponse;
using automotivegradelinux::StatusRequest;
using automotivegradelinux::StatusResponse;
using automotivegradelinux::BandStatus;
using automotivegradelinux::FrequencyStatus;
using automotivegradelinux::PlayStatus;
using automotivegradelinux::ScanStatus;

// Enum values used
using automotivegradelinux::BAND_AM;
using automotivegradelinux::BAND_FM;
using automotivegradelinux::SCAN_DIRECTION_FORWARD;
using automotivegradelinux::SCAN_DIRECTION_BACKWARD;

void RadioStatusEventReader::GetStatusEvents()
{
	ClientContext context;
	StatusRequest request;
	StatusResponse response;

	std::unique_ptr<ClientReader<StatusResponse> > reader(stub_->GetStatusEvents(&context, request));
	while (reader->Read(&response)) {
		if (response.has_band()) {
			BandStatus band_status = response.band();
			unsigned int band;
			if (band_status.band() == BAND_AM)
				band = 0;
			else if (band_status.band() == BAND_FM)
				band = 1;
			else
				continue;
			emit bandUpdate(band);
		} else if (response.has_frequency()) {
			FrequencyStatus frequency_status = response.frequency();
			emit frequencyUpdate(frequency_status.frequency());
		} else if (response.has_play()) {
			PlayStatus play_status = response.play();
			emit playingUpdate(play_status.playing());
		} else if (response.has_scan()) {
			ScanStatus scan_status = response.scan();
			emit scanningUpdate(scan_status.station_found());
		}
	}
	Status status = reader->Finish();
	if (!status.ok()) {
			qWarning() << "GetStatusEvents RPC failed";
	}

	emit finished();
}


RadioGrpcClient::RadioGrpcClient(QObject *parent) : QObject(parent)
{
	stub_ = Radio::NewStub(grpc::CreateChannel("localhost:50053", grpc::InsecureChannelCredentials()));

	// Create thread to read status events
	RadioStatusEventReader *reader = new RadioStatusEventReader(stub_);
	reader->moveToThread(&m_event_thread);
	connect(&m_event_thread, &QThread::started, reader, &RadioStatusEventReader::GetStatusEvents);
	connect(reader, &RadioStatusEventReader::finished, &m_event_thread, &QThread::quit);
	// FIXME: Normally the thread finishing would be connected per the below
	//        to trigger cleaning up the object.  That seems to trigger a crash
	//        for not entirely obvious reasons.  It seems unrelated to the signal
	//        connection to AppLauncherClient, as not connecting does not prevent
	//        the crash; further investigation is required.
	//connect(reader, &RadioStatusEventReader::finished, reader, &RadioStatusEventReader::deleteLater);
	//connect(&m_event_thread, &QThread::finished, &m_event_thread, &QThread::deleteLater);

	// To avoid having intermediary slot+signal's in this class, try
	// casting parent to Radio and connect directly to its slot.
	// Callers should set parent to ensure this works as required.
	if (parent) {
		RadioClient *radio = qobject_cast<RadioClient*>(parent);
		if (radio) {
			connect(reader,
				&RadioStatusEventReader::bandUpdate,
				radio,
				&RadioClient::updateBand);
			connect(reader,
				&RadioStatusEventReader::frequencyUpdate,
				radio,
				&RadioClient::updateFrequency);
			connect(reader,
				&RadioStatusEventReader::playingUpdate,
				radio,
				&RadioClient::updatePlaying);
			connect(reader,
				&RadioStatusEventReader::scanningUpdate,
				radio,
				&RadioClient::updateScanning);
		}
	}

	// Start status event handling
	m_event_thread.start();
}

void RadioGrpcClient::SetBand(unsigned int band)
{
	SetBandRequest request;

	ClientContext context;
	SetBandResponse response;
	if (band)
		request.set_band(BAND_FM);
	else
		request.set_band(BAND_AM);
	Status status = stub_->SetBand(&context, request, &response);
}

void RadioGrpcClient::SetFrequency(unsigned int frequency)
{
	SetFrequencyRequest request;

	ClientContext context;
	SetFrequencyResponse response;
	request.set_frequency(frequency);
	Status status = stub_->SetFrequency(&context, request, &response);
}

void RadioGrpcClient::Start()
{
	StartRequest request;

	ClientContext context;
	StartResponse response;
	Status status = stub_->Start(&context, request, &response);
}

void RadioGrpcClient::Stop()
{
	StopRequest request;

	ClientContext context;
	StopResponse response;
	Status status = stub_->Stop(&context, request, &response);
}

void RadioGrpcClient::ScanForward()
{
	ScanStartRequest request;

	ClientContext context;
	ScanStartResponse response;
	request.set_direction(SCAN_DIRECTION_FORWARD);
	Status status = stub_->ScanStart(&context, request, &response);
}

void RadioGrpcClient::ScanBackward()
{
	ScanStartRequest request;

	ClientContext context;
	ScanStartResponse response;
	request.set_direction(SCAN_DIRECTION_BACKWARD);
	Status status = stub_->ScanStart(&context, request, &response);
}

void RadioGrpcClient::ScanStop()
{
	ScanStopRequest request;

	ClientContext context;
	ScanStopResponse response;
	Status status = stub_->ScanStop(&context, request, &response);
}

void RadioGrpcClient::GetBandParameters(unsigned int band, unsigned int &min, unsigned int &max, unsigned int &step)
{
	GetBandParametersRequest request;

	ClientContext context;
	GetBandParametersResponse response;
	if (band)
		request.set_band(BAND_FM);
	else
		request.set_band(BAND_AM);
	Status status = stub_->GetBandParameters(&context, request, &response);
	if (status.ok()) {
		min = response.min();
		max = response.max();
		step = response.step();
	}
}
