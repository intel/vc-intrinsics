/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file declares interface functions used to aquire version info.

#ifndef GENX_VERSION
#define GENX_VERSION

#include <string>

namespace llvm {

namespace GenXIntrinsic {

std::string getVCIntrinsicsRevision();
std::string getVCIntrinsicsRepository();

} // namespace GenXIntrinsic
} // namespace llvm

#endif
