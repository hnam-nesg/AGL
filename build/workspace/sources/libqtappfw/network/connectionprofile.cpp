#include "connectionprofile.h"

ConnectionProfile::ConnectionProfile(const QString &address,
                                     const QString &security,
                                     const QString &service,
                                     const QString &state,
                                     const QString &ssid,
                                     const int &strength,
                                     const QString &netmask,
                                     const QString &gateway,
                                     const QString &amethod,
                                     const QString &ns,
                                     const QString &nsmethod)
    : m_address(address), m_security(security), m_service(service),
      m_state(state), m_ssid(ssid), m_strength(strength), m_netmask(netmask),
      m_gateway(gateway), m_addrmethod(amethod), m_nameservers(ns),
      m_nsmethod(nsmethod)
{
}

QString ConnectionProfile::address() const
{
    return m_address;
}

QString ConnectionProfile::security() const
{
    return m_security;
}

QString ConnectionProfile::service() const
{
    return m_service;
}

QString ConnectionProfile::state() const
{
    return m_state;
}

QString ConnectionProfile::ssid() const
{
    return m_ssid;
}

int ConnectionProfile::strength() const
{
    return m_strength;
}

QString ConnectionProfile::netmask() const
{
    return m_netmask;
}

QString ConnectionProfile::gateway() const
{
    return m_gateway;
}

QString ConnectionProfile::nameservers() const
{
    return m_nameservers;
}

QString ConnectionProfile::addrmethod() const
{
    return m_addrmethod;
}

QString ConnectionProfile::nsmethod() const
{
    return m_nsmethod;
}

void ConnectionProfile::setAddress(const QString address)
{
    m_address = address;
}

void ConnectionProfile::setState(const QString state)
{
    m_state = state;
}

void ConnectionProfile::setStrength(const int strength)
{
    m_strength = strength;
}

void ConnectionProfile::setNetmask(const QString netmask)
{
    m_netmask = netmask;
}

void ConnectionProfile::setGateway(const QString gateway)
{
    m_gateway = gateway;
}

void ConnectionProfile::setNameservers(const QString nameservers)
{
    m_nameservers = nameservers;
}

void ConnectionProfile::setAddrMethod(const QString method)
{
    m_addrmethod = method;
}

void ConnectionProfile::setNSMethod(const QString method)
{
    m_nsmethod = method;
}
