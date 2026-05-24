#include "RuntimeContext.h"

nlu::VehicleContext RuntimeContext::toVehicleContext() const
{
    nlu::VehicleContext ctx;
    ctx.acOn = hvacAcOn;
    ctx.cabinTemp = cabinTemp;
    ctx.targetTemp = targetTemp;
    ctx.fanLevel = fanLevel;
    ctx.mediaPlaying = mediaPlaying;
    ctx.phoneIncomingCall = phoneIncomingCall;
    ctx.phoneActiveCall = phoneActiveCall;
    ctx.navigationActive = navigationActive;
    return ctx;
}
