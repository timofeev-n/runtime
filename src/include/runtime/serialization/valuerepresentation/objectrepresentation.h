#pragma once

#include "representationbase.h"
#include <runtime/meta/classinfo.h>


namespace Runtime {

template<typename T>
inline constexpr bool IsObjectRepresentable = Meta::AttributeDefined<T, Meta::ClassFieldsAttribute>;



} // Runtime
