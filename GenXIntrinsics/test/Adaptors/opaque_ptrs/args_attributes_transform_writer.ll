;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

%foo = type { i32 }

; CHECK: define spir_kernel void @test(
; CHECK-SAME: ptr addrspace(1) byval(%foo)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[ARG:%[^)]+]])
define spir_kernel void @test(ptr addrspace(1) byval(%foo) %arg) {
; CHECK-NOT: [[ARG]]
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}
!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{i32 0}
!2 = !{i32 0}
!3 = !{!"svmptr_t"}
