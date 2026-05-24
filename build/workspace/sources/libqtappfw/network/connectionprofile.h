#ifndef CONNECTION_PROFILE_H
#define CONNECTION_PROFILE_H

#include <QString>

class ConnectionProfile
{
    public:
        ConnectionProfile(const QString &address,
                          const QString &security,
                          const QString &service,
                          const QString &ssid,
                          const QString &state,
                          const int &strength,
                          const QString &netmask,
                          const QString &gateway,
                          const QString &amethod,
                          const QString &ns,
                          const QString &nsmethod);

        QString address() const;
        QString service() const;
        QString ssid() const;
        QString security() const;
        QString state() const;
        int strength() const;
        QString netmask() const;
        QString gateway() const;
        QString nameservers() const;
        QString addrmethod() const;
        QString nsmethod() const;

        void setAddress(const QString address);
        void setState(const QString state);
        void setStrength(const int strength);
        void setNetmask(const QString netmask);
        void setGateway(const QString gateway);
        void setNameservers(const QString nameservers);
        void setAddrMethod(const QString method);
        void setNSMethod(const QString method);

    private:
        QString m_address;
        QString m_security;
        QString m_service;
        QString m_state;
        QString m_ssid;
        int m_strength;
        QString m_netmask;
        QString m_gateway;
        QString m_addrmethod;
        QString m_nameservers;
        QString m_nsmethod;
};

#endif // CONNECTION_PROFILE_H
