// SharedImageProvider.h
#pragma once
#include <QQuickImageProvider>
#include <QSharedMemory>
#include <QtGlobal>
#include "shared_protocol.h"

class SharedImageProvider : public QQuickImageProvider
{
public:
    QVector<QString> names;
    SharedImageProvider(const QString &key)
        : QQuickImageProvider(QQuickImageProvider::Image),
        m_shared(key)
    {
        qDebug() << "nam";
        names.append("textureData");
        names.append("textureData14");
        names.append("textureData16");
        names.append("textureData18");
        names.append("textureData41");
        names.append("textureData43");
        names.append("textureData48");
        names.append("textureData50");
        names.append("textureData55");
        names.append("textureData57");
        names.append("textureData59");
        names.append("textureData64");
        names.append("textureData66");
        names.append("textureData68");
        names.append("textureData73");
        names.append("textureData75");
        names.append("textureData77");
        names.append("textureData81");
        names.append("textureData86");
        names.append("textureData88");
        names.append("textureData90");
        names.append("textureData95");
        names.append("textureData97");
        names.append("textureData99");
        names.append("textureData104");
        names.append("textureData106");
        names.append("textureData108");
        names.append("textureData113");
        names.append("textureData193");
        names.append("textureData195");
        names.append("textureData197");
        names.append("textureData202");
        names.append("textureData204");
        names.append("textureData206");
        names.append("textureData218");
        names.append("textureData220");
        names.append("textureData222");
        names.append("textureData227");
        names.append("textureData229");
        names.append("textureData231");
        names.append("textureData281");
        names.append("textureData283");
        names.append("textureData285");
        names.append("textureData302");
        names.append("textureData304");
        names.append("textureData306");
        names.append("textureData403");
        names.append("textureData405");
        names.append("textureData407");
        names.append("textureData413");
        names.append("textureData415");
    }

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override
    {
        qDebug() << "ID received =" << id;

        if (!m_shared.attach()) {
            qDebug() << "SharedImageProvider: attach failed:" << m_shared.errorString();
            return QImage();
        }

        m_shared.lock();
        char *base = static_cast<char*>(m_shared.data());
        GlobalHeader gh;
        memcpy(&gh, base, sizeof(gh));
        int index = 0;
        for (int i = 0; i < gh.imageCount; ++i) {
            if (names[i] == id)
                index = i;
        }

        qDebug() << "imageCount =" << gh.imageCount;
        qDebug() << "index =" << index;

        if (gh.magic != SHARED_MAGIC || gh.version != SHARED_VERSION) {
            qWarning() << "SharedImageProvider: invalid magic/version";
            m_shared.unlock();
            m_shared.detach();
            return QImage();
        }
        if (gh.ready == 0) {
            qWarning() << "SharedImageProvider: data not ready yet";
            m_shared.unlock();
            m_shared.detach();
            return QImage();
        }
        if (index < 0 || index >= (int)gh.imageCount) {
            qWarning() << "SharedImageProvider: index out of range" << index;
            m_shared.unlock();
            m_shared.detach();
            return QImage();
        }

        // read the index table to find nth ImageHeader
        ImageHeader ih;
        char *p = base + gh.indexOffset + index * sizeof(ImageHeader);
        memcpy(&ih, p, sizeof(ih));

        // compute pixel offset for this image: sum previous sizes
        quint64 pixelPos = gh.pixelOffset;
        for (int i = 0; i < index; ++i) {
            ImageHeader tmp;
            char *pp = base + gh.indexOffset + i * sizeof(ImageHeader);
            memcpy(&tmp, pp, sizeof(tmp));
            pixelPos += tmp.dataSize;
        }

        if ((quint64)m_shared.size() < pixelPos + ih.dataSize) {
            qWarning() << "SharedImageProvider: shared memory size too small";
            m_shared.unlock();
            m_shared.detach();
            return QImage();
        }

        // Build QImage that points to shared memory pixels (no copy)
        uchar *pixelPtr = reinterpret_cast<uchar*>(base + pixelPos);
        QImage img(pixelPtr, ih.width, ih.height, (QImage::Format)ih.format);

        // QML expects ownership of returned QImage; to be safe we return copy
        QImage ret = img.copy();

        if (size) *size = ret.size();
        m_shared.unlock();
        m_shared.detach(); // detach when done

        return ret;
    }

private:
    QSharedMemory m_shared;
};
