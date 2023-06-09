// Copyright 2017 The Dawn Authors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "dawn/utils/SystemUtils.h"

#include "dawn/common/Platform.h"

#if DAWN_PLATFORM_IS(WINDOWS)
#include <Windows.h>
#elif DAWN_PLATFORM_IS(POSIX)
#include <unistd.h>
#else
#error "Unsupported platform."
#endif

namespace dawn::utils {

#if DAWN_PLATFORM_IS(WINDOWS)
void USleep(unsigned int usecs) {
    Sleep(static_cast<DWORD>(usecs / 1000));
}
#elif DAWN_PLATFORM_IS(POSIX)
void USleep(unsigned int usecs) {
    usleep(usecs);
}
#else
#error "Implement USleep for your platform."
#endif

}  // namespace dawn::utils
