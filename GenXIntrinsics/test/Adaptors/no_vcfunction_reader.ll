;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that reader ignores signature rewriting for kernels
; that are not VCFunction.

; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

define spir_kernel void @test(i8 addrspace(1) *%ptr) {
; CHECK-LABEL: @test(

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
