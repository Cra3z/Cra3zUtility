#pragma once

#ifdef BUILD_CRA3Z_UTIL_MODULE

#define CRA3Z_MOD_EXPORT export
#define CRA3Z_MOD_EXPORT_BEGIN export {
#define CRA3Z_MOD_EXPORT_END }

import std;

#else

#define CRA3Z_MOD_EXPORT
#define CRA3Z_MOD_EXPORT_BEGIN
#define CRA3Z_MOD_EXPORT_END

#endif