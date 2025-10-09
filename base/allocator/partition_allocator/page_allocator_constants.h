// Copyright (c) 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BASE_ALLOCATOR_PARTITION_ALLOCATOR_PAGE_ALLOCATOR_CONSTANTS_H_
#define BASE_ALLOCATOR_PARTITION_ALLOCATOR_PAGE_ALLOCATOR_CONSTANTS_H_

#include <stddef.h>

#include "build/build_config.h"

// For Linux ARM64 support, we need to include additional headers
#if (defined(OS_LINUX) || defined(OS_ANDROID)) && defined(ARCH_CPU_ARM64)
#include <unistd.h>
#include <atomic>
#endif

namespace base {

#if defined(OS_WIN) || defined(ARCH_CPU_PPC64)
static constexpr size_t kPageAllocationGranularityShift = 16;  // 64KB
#elif defined(_MIPS_ARCH_LOONGSON)
static constexpr size_t kPageAllocationGranularityShift = 14;  // 16KB
#elif (defined(OS_LINUX) || defined(OS_ANDROID)) && defined(ARCH_CPU_ARM64)
// ARM64 supports 4kb (shift = 12), 16kb (shift = 14), and 64kb (shift = 16)
// page sizes. We'll determine this at runtime.
static constexpr size_t kPageAllocationGranularityShift = 12;  // Default to 4KB, will be overridden
#else
static constexpr size_t kPageAllocationGranularityShift = 12;  // 4KB
#endif

// For Linux ARM64, we need to handle runtime page size
#if (defined(OS_LINUX) || defined(OS_ANDROID)) && defined(ARCH_CPU_ARM64)
// Page allocator constants are run-time constant for Linux ARM64
inline size_t PageAllocationGranularity() {
  static std::atomic<int> cached_size{0};
  int size = cached_size.load(std::memory_order_relaxed);
  if (size == 0) {
    size = getpagesize();
    cached_size.store(size, std::memory_order_relaxed);
  }
  return size;
}

inline size_t PageAllocationGranularityShift() {
  static std::atomic<int> cached_shift{0};
  int shift = cached_shift.load(std::memory_order_relaxed);
  if (shift == 0) {
    shift = __builtin_ctz((int)PageAllocationGranularity());
    cached_shift.store(shift, std::memory_order_relaxed);
  }
  return shift;
}

// Constants not defined for ARM64 - use runtime functions instead
// kPageAllocationGranularity -> PageAllocationGranularity()
// kPageAllocationGranularityOffsetMask -> PageAllocationGranularityOffsetMask()
// kPageAllocationGranularityBaseMask -> PageAllocationGranularityBaseMask()
#else
static constexpr size_t kPageAllocationGranularity =
    1 << kPageAllocationGranularityShift;
static constexpr size_t kPageAllocationGranularityOffsetMask =
    kPageAllocationGranularity - 1;
static constexpr size_t kPageAllocationGranularityBaseMask =
    ~kPageAllocationGranularityOffsetMask;
#endif

#if defined(_MIPS_ARCH_LOONGSON)
static constexpr size_t kSystemPageSize = 16384;
#elif defined(ARCH_CPU_PPC64)
// Modern ppc64 systems support 4KB and 64KB page sizes.
// Since 64KB is the de-facto standard on the platform
// and binaries compiled for 64KB are likely to work on 4KB systems,
// 64KB is a good choice here.
static constexpr size_t kSystemPageSize = 65536;
#elif (defined(OS_LINUX) || defined(OS_ANDROID)) && defined(ARCH_CPU_ARM64)
// ARM64 supports 4kb, 16kb, and 64kb page sizes. We'll determine this at runtime.
inline size_t SystemPageSize() {
  return PageAllocationGranularity();
}

inline size_t SystemPageShift() {
  return PageAllocationGranularityShift();
}

// Constants not defined for ARM64 - use runtime functions instead
// kSystemPageSize -> SystemPageSize()
// kSystemPageOffsetMask -> SystemPageOffsetMask()
// kSystemPageBaseMask -> SystemPageBaseMask()
#else
static constexpr size_t kSystemPageSize = 4096;
static constexpr size_t kSystemPageOffsetMask = kSystemPageSize - 1;
static_assert((kSystemPageSize & (kSystemPageSize - 1)) == 0,
              "kSystemPageSize must be power of 2");
static constexpr size_t kSystemPageBaseMask = ~kSystemPageOffsetMask;
#endif

// For Linux ARM64, we need to provide inline functions for the masks
#if (defined(OS_LINUX) || defined(OS_ANDROID)) && defined(ARCH_CPU_ARM64)
inline size_t SystemPageOffsetMask() {
  return SystemPageSize() - 1;
}

inline size_t SystemPageBaseMask() {
  return ~SystemPageOffsetMask();
}

inline size_t PageAllocationGranularityOffsetMask() {
  return PageAllocationGranularity() - 1;
}

inline size_t PageAllocationGranularityBaseMask() {
  return ~PageAllocationGranularityOffsetMask();
}
#endif

static constexpr size_t kPageMetadataShift = 5;  // 32 bytes per partition page.
static constexpr size_t kPageMetadataSize = 1 << kPageMetadataShift;

}  // namespace base

#endif  // BASE_ALLOCATOR_PARTITION_ALLOCATOR_PAGE_ALLOCATOR_CONSTANTS_H_
