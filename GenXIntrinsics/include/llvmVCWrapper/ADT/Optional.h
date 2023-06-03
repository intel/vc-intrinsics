/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2022 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_ADT_OPTIONAL_H
#define VCINTR_ADT_OPTIONAL_H

#if VC_INTR_LLVM_VERSION_MAJOR < 16
#include <llvm/ADT/Optional.h>

namespace VCINTR {

template <class T> using Optional = llvm::Optional<T>;
}
#else
#include <optional>

namespace VCINTR {
template <class T> using Optional = std::optional<T>;
}

#endif

#endif
