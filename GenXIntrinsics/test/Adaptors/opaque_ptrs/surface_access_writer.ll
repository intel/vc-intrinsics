;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel surface argument translation from old style with
; metadata to new style with opaque types that SPIRV translator can
; understand. This test checks access qualifiers translation.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: target("spirv.BufferSurfaceINTEL", 0)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 1)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM3D:%[^)]+]])
define spir_kernel void @test(i32 %buf, i32 %im1d, i32 %im1db, i32 %im2d, i32 %im3d) {
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_0(target("spirv.BufferSurfaceINTEL", 0) [[BUF]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_0_0_0_0_2(target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) [[IM1D]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_5_0_0_0_0_0_1(target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 1) [[IM1DB]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_1(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1) [[IM2D]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_2_0_0_0_0_0_0(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) [[IM3D]])
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t read_only", !"image1d_t read_write", !"image1d_buffer_t write_only", !"image2d_t write_only", !"image3d_t read_only"}
