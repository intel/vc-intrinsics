/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

//===----------------------------------------------------------------------===//
//
// This file defines GenX kernel metadata operand numbers and other module
// metadata.
//
//===----------------------------------------------------------------------===//

#ifndef GENX_METADATA_H
#define GENX_METADATA_H

namespace llvm {
namespace genx {

namespace FunctionMD {
static constexpr const char GenXKernels[] = "genx.kernels";
static constexpr const char GenXByteOffset[] = "genx_byte_offset";
static constexpr const char GenXVolatile[] = "genx_volatile";
static constexpr const char CMGenXMain[] = "CMGenxMain";
static constexpr const char CMStackCall[] = "CMStackCall";
static constexpr const char CMCallable[] = "CMCallable";
static constexpr const char CMEntry[] = "CMEntry";
static constexpr const char CMFloatControl[] = "CMFloatControl";
static constexpr const char CMGenxSIMT[] = "CMGenxSIMT";
static constexpr const char CMGenxReplicateMask[] = "CMGenxReplicateMask";
static constexpr const char OCLRuntime[] = "oclrt";
static constexpr const char ReferencedIndirectly[] = "referenced-indirectly";
} // namespace FunctionMD

namespace VCModuleMD {
static constexpr const char VCGlobalVariable[] = "VCGlobalVariable";
static constexpr const char VCVolatile[] = "VCVolatile";
static constexpr const char VCByteOffset[] = "VCByteOffset";
static constexpr const char VCSingleElementVector[] = "VCSingleElementVector";
} // namespace VCModuleMD

namespace VCFunctionMD {
static constexpr const char VCFunction[] = "VCFunction";
static constexpr const char VCStackCall[] = "VCStackCall";
static constexpr const char VCCallable[] = "VCCallable";
static constexpr const char VCFCEntry[] = "VCFCEntry";
static constexpr const char VCArgumentIOKind[] = "VCArgumentIOKind";
static constexpr const char VCFloatControl[] = "VCFloatControl";
static constexpr const char VCSLMSize[] = "VCSLMSize";
static constexpr const char VCArgumentKind[] = "VCArgumentKind";
static constexpr const char VCArgumentDesc[] = "VCArgumentDesc";
static constexpr const char VCSIMTCall[] = "VCSIMTCall";
} // namespace VCFunctionMD

enum KernelMDOp {
  FunctionRef,  // Reference to Function
  Name,         // Kernel name
  ArgKinds,     // Reference to metadata node containing kernel arg kinds
  SLMSize,      // SLM-size in bytes
  ArgOffsets,   // Kernel argument offsets
  ArgIOKinds,   // Reference to metadata node containing kernel argument
                // input/output kinds
  ArgTypeDescs, // Kernel argument type descriptors
  Reserved_0,
  BarrierCnt    // Barrier count
};

inline MDNode *GetOldStyleKernelMD(Function const &F) {
  auto *KernelMD = static_cast<MDNode *>(nullptr);
  auto *KernelMDs = F.getParent()->getNamedMetadata(FunctionMD::GenXKernels);
  if (!KernelMDs)
    return KernelMD;

  for (unsigned I = 0, E = KernelMDs->getNumOperands(); I < E; ++I) {
    auto *Kernel = mdconst::dyn_extract<Function>(
        KernelMDs->getOperand(I)->getOperand(KernelMDOp::FunctionRef));
    if (Kernel == &F) {
      KernelMD = KernelMDs->getOperand(I);
      break;
    }
  }
  return KernelMD;
}

} // namespace genx
} // namespace llvm

#endif
