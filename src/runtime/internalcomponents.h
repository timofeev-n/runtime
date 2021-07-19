#pragma once
#include "runtime/runtime/internal/runtimecomponent.h"


namespace Runtime {

RuntimeComponent::Factory PoolSchedulerInternalComponent();
RuntimeComponent::Factory RuntimeDefaultSchedulerInternalComponent();
RuntimeComponent::Factory TimerManagerInternalComponent();

}
