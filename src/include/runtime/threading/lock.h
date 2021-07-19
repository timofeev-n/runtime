#pragma once

#include <runtime/utils/preprocessor.h>
#include <mutex>

#define lock_(Mutex) const std::lock_guard ANONYMOUS_VARIABLE_NAME(lock_mutex_) {Mutex}
