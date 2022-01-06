/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file declares functions for rewriting single element vectors
// in GenXSPIRV adaptors.

#include "llvm/IR/Module.h"

namespace llvm {
namespace genx {

void rewriteSingleElementVectors(Module &M);
void restoreSingleElementVectors(Module &M);

} // namespace genx
} // namespace llvm
