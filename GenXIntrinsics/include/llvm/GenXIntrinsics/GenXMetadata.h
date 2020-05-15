/*===================== begin_copyright_notice ==================================

 Copyright (c) 2020, Intel Corporation


 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the "Software"),
 to deal in the Software without restriction, including without limitation
 the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/


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

namespace ModuleMD {
const static char GenXKernels[] = "genx.kernels";
const static char GenXByteOffset[] = "genx_byte_offset";
const static char GenXVolatile[] = "genx_volatile";
const static char CMGenXMain[] = "CMGenxMain";
const static char CMStackCall[] = "CMStackCall";
const static char CMFloatControl[] = "CMFloatControl";
const static char OCLRuntime[] = "oclrt";
const static char SPIRVMemoryModel[] = "spirv.MemoryModel";
const static char ReferencedIndirectly[] = "referenced-indirectly";
} // namespace ModuleMD

namespace SPIRVParams {
const static unsigned SPIRVMemoryModelSimple = 0;
const static unsigned SPIRVAddressingModel32 = 1;
const static unsigned SPIRVAddressingModel64 = 2;
} // namespace SPIRVMD

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
} // namespace genx
} // namespace llvm

#endif
