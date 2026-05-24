#pragma once

#include <cstdio>

#define DEBUG

#if !defined(LOG) && defined(DEBUG)
#define LOG(fmt, ...) do { fprintf(stderr, "gRPC:proxy:%s() " fmt, __func__, ##__VA_ARGS__); } while (0)
#else
#define LOG(fmt, ...) do {} while (0)
#endif
