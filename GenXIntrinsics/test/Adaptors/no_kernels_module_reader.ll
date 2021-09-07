;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; XFAIL: llvm13,llvm14
; Test general translation of attributes within module that has no kernels

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

; CHECK: @some_func
; CHECK-SAME: #[[ATTR_GROUP:[0-9]+]]
define <16 x float> @some_func(<16 x float> %x) local_unnamed_addr #0 {
  ret <16 x float> %x
}

; CHECK: attributes #[[ATTR_GROUP]] = {
; CHECK: "CMStackCall"
; CHECK: }
attributes #0 = { "VCStackCall" "VCFunction"}
