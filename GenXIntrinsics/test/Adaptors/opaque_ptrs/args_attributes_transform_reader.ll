;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024-2025 Intel Corporation
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
; CHECK-NEXT: ret void
  ret void
}

; CHECK: define dllexport spir_kernel void @test_restore(
; CHECK-SAME: ptr byval(%foo)
; CHECK-SAME: [[ARG:%[^)]+]])
define spir_kernel void @test_restore(ptr addrspace(1) byval(i8) %arg) #0 {
  %conv = call ptr @llvm.genx.address.convert.p0foo.p1(ptr addrspace(1) %arg)
  %gep = getelementptr %foo, ptr %conv, i64 0, i32 0
  ret void
}

declare ptr @llvm.genx.address.convert.p0foo.p1(ptr addrspace(1)) #0

attributes #0 = { "VCFunction" }
