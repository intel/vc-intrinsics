;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; UNSUPPORTED: llvm8
; XFAIL: llvm13, llvm14
; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s
; CHECK: @test
; CHECK-SAME: (%foo addrspace(1)* byval(%foo) %arg)

%foo = type { i32 }

define spir_kernel void @test(i8 addrspace(1)* byval(i8) "VCArgumentIOKind"="0" %arg) #0 {
  %1 = call %foo addrspace(1)* @llvm.genx.address.convert.p1foo.p1i8(i8 addrspace(1)* %arg)
  ret void
}

declare %foo addrspace(1)* @llvm.genx.address.convert.p1foo.p1i8(i8 addrspace(1)*)

attributes #0 = { "VCFunction" }
