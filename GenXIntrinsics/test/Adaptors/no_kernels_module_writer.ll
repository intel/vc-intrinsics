;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test general translation of attributes within module that has no kernels

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

; CHECK: @some_func
; CHECK: #[[ATTR_GROUP:[0-9]+]]

define <16 x float> @some_func(<16 x float> %x) local_unnamed_addr #0 {
  ret <16 x float> %x
}

; CHECK: attributes #[[ATTR_GROUP]] = {
; CHECK-DAG: "VCFunction"
; CHECK-DAG: "VCStackCall"
; CHECK: }
attributes #0 = { "CMStackCall" }
