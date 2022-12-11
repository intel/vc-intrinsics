/*========================== begin_copyright_notice ============================

Copyright (C) 2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_ADT_NONE_H
#define VCINTR_ADT_NONE_H

#include <llvm/ADT/None.h>

namespace VCINTR {

#if VC_INTR_LLVM_VERSION_MAJOR < 16
const llvm::NoneType None = llvm::None;
#else
inline constexpr std::nullopt_t None = std::nullopt;
#endif
} // namespace VCINTR

#endif // VCINTR_ADT_NONE_H
