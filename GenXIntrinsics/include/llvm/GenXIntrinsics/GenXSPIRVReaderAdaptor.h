/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

///
/// GenXSPIRVReaderAdaptor
/// ---------------------------
/// This pass converts metadata from SPIRV format to whichever used in backend

namespace llvm {
class ModulePass;
class PassRegistry;

void initializeGenXSPIRVReaderAdaptorPass(PassRegistry &);
ModulePass *createGenXSPIRVReaderAdaptorPass();
} // namespace llvm
