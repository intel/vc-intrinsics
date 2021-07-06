/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

#ifndef VCINTR_IR_INSTRTYPES_H
#define VCINTR_IR_INSTRTYPES_H

#include <llvm/IR/InstrTypes.h>

namespace VCINTR {
#if VC_INTR_LLVM_VERSION_MAJOR <= 7
using llvm::TerminatorInst;
#elif VC_INTR_LLVM_VERSION_MAJOR >= 8
using TerminatorInst = llvm::Instruction;
#endif
} // namespace VCINTR

#endif // VCINTR_IR_INSTRTYPES_H
