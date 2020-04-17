// Copyright 2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PLATFORM_API2_SCHEDULED_EXECUTOR_H_
#define PLATFORM_API2_SCHEDULED_EXECUTOR_H_

#include <cstdint>
#include <memory>

#include "platform/api2/executor.h"
#include "platform/cancelable.h"
#include "platform/runnable.h"
#include "absl/time/time.h"

namespace location {
namespace nearby {

// An Executor that can schedule commands to run after a given delay, or to
// execute periodically.
//
// https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/ScheduledExecutorService.html
class ScheduledExecutor : public Executor {
 public:
  ~ScheduledExecutor() override = default;
  virtual std::unique_ptr<Cancelable> Schedule(
      std::unique_ptr<Runnable> runnable, absl::Duration duration) = 0;
};

}  // namespace nearby
}  // namespace location

#endif  // PLATFORM_API2_SCHEDULED_EXECUTOR_H_
