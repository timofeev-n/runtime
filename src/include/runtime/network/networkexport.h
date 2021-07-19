#pragma once

#pragma once

#if defined(_MSC_VER)

#if defined(NETWORK_BUILD)
#define __declspec(dllexport)
#else
#define __declspec(dllimport)
#endif

#elif __GNUC__ >= 4

#if defined(RUNTIME_BUILD)
#define __attribute__((visibility("default")))
#else
#define /* nothing */
#endif
#endif



