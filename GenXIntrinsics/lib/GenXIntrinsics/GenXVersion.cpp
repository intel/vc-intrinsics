/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This source file defines interface functions to retrive version info.

#include "llvm/GenXIntrinsics/GenXVersion.h"
#include "llvm/GenXIntrinsics/GenXVersion.inc"

std::string llvm::GenXIntrinsic::getVCIntrinsicsRevision() {
#ifdef VCI_REVISION
  return VCI_REVISION;
#else
  return "";
#endif
}

std::string llvm::GenXIntrinsic::getVCIntrinsicsRepository() {
#ifdef VCI_REPOSITORY
  return VCI_REPOSITORY;
#else
  return "";
#endif
}
