#include "handlermodel3d.h"

HandlerModel3D::HandlerModel3D(QObject *parent):QObject(parent) {
    m_trunk = false;
    m_capo = false;
    m_door = false;
}

void HandlerModel3D::setTrunk(bool value)
{
    m_trunk = value;
    emit trunkChanged(m_trunk);
}

void HandlerModel3D::setCapo(bool value)
{
    m_capo = value;
    emit capoChanged(m_capo);
}

void HandlerModel3D::setDoor(bool value)
{
    m_door = value;
    emit doorChanged(m_door);
}



