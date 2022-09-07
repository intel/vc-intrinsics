;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; XFAIL: llvm13, llvm14, llvm15, llvm16
; Test that reader ignores signature rewriting for kernels
; that are not VCFunction.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

define spir_kernel void @test(i8 addrspace(1) *%ptr) {
; CHECK-LABEL: @test(

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
