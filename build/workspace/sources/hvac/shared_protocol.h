// shared_protocol.h
#pragma once
#include <QtGlobal>

static const quint32 SHARED_MAGIC = 0x46474D49; // 'IMGF' little-endian readable
static const quint32 SHARED_VERSION = 1;

#pragma pack(push, 1)
struct GlobalHeader {
    quint32 magic;        // = SHARED_MAGIC
    quint32 version;      // = SHARED_VERSION
    quint32 imageCount;
    quint64 indexOffset;  // offset from start to first ImageHeader table
    quint64 pixelOffset;  // offset from start to pixel data start
    quint8  ready;        // 0 = not ready, 1 = ready
    // padding if needed...
};

struct ImageHeader {
    quint32 width;
    quint32 height;
    quint32 format;   // QImage::Format as int
    quint64 dataSize; // bytes for this image pixel data (sizeInBytes)
};
#pragma pack(pop)
