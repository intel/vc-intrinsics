;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test @llvm.genx.address.convert intrinsic generation with proper attributes

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: target("spirv.BufferSurfaceINTEL", 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[BUF:%[^)]+]])
define spir_kernel void @test(i32 %buf) {
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_2(target("spirv.BufferSurfaceINTEL", 2) [[BUF]])
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2}
!2 = !{i32 0}
!3 = !{!"buffer_t"}
