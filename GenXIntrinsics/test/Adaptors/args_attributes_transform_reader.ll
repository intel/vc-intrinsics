;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; UNSUPPORTED: llvm8, opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%foo = type { i32 }

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %foo addrspace(1)* byval(%foo)
; CHECK-SAME: [[ARG:%[^)]+]])
define spir_kernel void @test(%foo addrspace(1)* byval(%foo) %arg) #0 {
  ret void
}

attributes #0 = { "VCFunction" }
