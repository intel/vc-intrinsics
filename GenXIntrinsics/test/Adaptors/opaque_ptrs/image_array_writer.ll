;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of image array arguments.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: target("spirv.Image", void, 0, 0, 1, 0, 0, 0, 0)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1DARR:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 1, 0, 1, 0, 0, 0, 1)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM2DARR:%[^)]+]])
define spir_kernel void @test(i32 %im1darr, i32 %im2darr) {
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_1_0_0_0_0(target("spirv.Image", void, 0, 0, 1, 0, 0, 0, 0) [[IM1DARR]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_1_0_0_0_1(target("spirv.Image", void, 1, 0, 1, 0, 0, 0, 1) [[IM2DARR]])
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2}
!2 = !{i32 0, i32 0}
!3 = !{!"image1d_array_t read_only", !"image2d_array_t write_only"}
