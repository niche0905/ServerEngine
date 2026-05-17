#pragma once

#include "Utils/Types.h"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <cfloat>
#include <algorithm>

#ifndef NAVMESH_API
#define NAVMESH_API
#endif

#ifndef FORCEINLINE
#define FORCEINLINE inline
#endif

#ifndef UE_DEPRECATED
#define UE_DEPRECATED(Version, Message)
#endif

#ifndef TEXT
#define TEXT(x) x
#endif

#ifndef ensureMsgf
#define ensureMsgf(expr, format, ...) (expr)
#endif

#ifndef KINDA_SMALL_NUMBER
#define KINDA_SMALL_NUMBER 1.e-4f
#endif

#define check(expr) assert(expr)
#define checkf(expr, ...) assert(expr)
#define ensure(expr) (expr)

#define CA_ASSUME(x)
#define CA_SUPPRESS(x)

#define WITH_NAVMESH_SEGMENT_LINKS 0
#define WITH_NAVMESH_CLUSTER_LINKS 0

#ifndef STATS
#define STATS 0
#endif
