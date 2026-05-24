#ifndef HANDLERMODEL3D_H
#define HANDLERMODEL3D_H

#include <QObject>

class HandlerModel3D: public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool trunk WRITE setTrunk READ getTrunk NOTIFY trunkChanged)
    Q_PROPERTY(bool capo WRITE setCapo READ getCapo NOTIFY capoChanged)
    Q_PROPERTY(bool door WRITE setDoor READ getDoor NOTIFY doorChanged)
public:
    explicit HandlerModel3D(QObject *parent = nullptr);

    bool getTrunk(){return m_trunk;}
    bool getCapo(){return m_capo;}
    bool getDoor(){return m_door;}

    void setTrunk(bool value);
    void setCapo(bool value);
    void setDoor(bool value);
private:
    bool m_trunk;
    bool m_capo;
    bool m_door;
signals:
    void trunkChanged(bool value);
    void capoChanged(bool value);
    void doorChanged(bool value);
};

#endif // HANDLERMODEL3D_H
