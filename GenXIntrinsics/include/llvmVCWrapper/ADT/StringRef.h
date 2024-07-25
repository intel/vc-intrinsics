/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_ADT_STRINGREF_H
#define VCINTR_ADT_STRINGREF_H

#include <llvm/ADT/StringRef.h>

namespace VCINTR {

namespace StringRef {

inline bool starts_with(llvm::StringRef S, llvm::StringRef Prefix) {
#if VC_INTR_LLVM_VERSION_MAJOR >= 16
  return S.starts_with(Prefix);
#else
  return S.startswith(Prefix);
#endif
}

} // namespace StringRef

} // namespace VCINTR

#endif
