;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of implicit argument. Implicit arguments
; should not appear in current form after transition from cmc.

; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: <3 x i32>
; CHECK-SAME: "VCArgumentKind"="24"
; CHECK-SAME: [[LOCAL_ID:%[^)]+]])
define void @test(<3 x i32> %__arg_llvm.genx.local.id) {
; CHECK-NEXT: ret void
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (<3 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !2, i32 0, i32 0}
!1 = !{i32 24}
!2 = !{}
