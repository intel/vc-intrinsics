;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that writer does not changes signature if correct
; types are already used. Just drop all annotations.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: target("spirv.BufferSurfaceINTEL", 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 2)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM3D:%[^,]+]],
; CHECK-SAME: target("spirv.Sampler")
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: ptr addrspace(1)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: <4 x i32>
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(target("spirv.BufferSurfaceINTEL", 2) %buf, target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) %im1d, target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 2) %im1db, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) %im2d, target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 2) %im3d, target("spirv.Sampler") %samp, ptr addrspace(1) %ptr, <4 x i32> %gen) {
; CHECK-NOT: [[BUF]]
; CHECK-NOT: [[IM1D]]
; CHECK-NOT: [[IM1DB]]
; CHECK-NOT: [[IM2D]]
; CHECK-NOT: [[IM3D]]
; CHECK-NOT: [[SAMP]]
; CHECK-NOT: [[PTR]]
; CHECK-NOT: [[GEN]]
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}
!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t", !"image1d_t", !"image1d_buffer_t", !"image2d_t", !"image3d_t", !"sampler_t", !"svmptr_t", !""}
