;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%foo = type { i32 }

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: ptr addrspace(1) byval(%foo)
; CHECK-SAME: [[ARG:%[^)]+]])
define spir_kernel void @test(ptr addrspace(1) byval(%foo) %arg) #0 {
; CHECK-NOT: [[ARG]]
  ret void
}

attributes #0 = { "VCFunction" }
