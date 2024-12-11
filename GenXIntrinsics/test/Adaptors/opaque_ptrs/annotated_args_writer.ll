;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel arguments translation from old style with metadata to
; new style with opaque types that SPIRV translator can
; understand.
; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s
; RUN: opt -passes=GenXSPIRVWriterAdaptor,GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

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
define spir_kernel void @test(i32 %buf, i32 %im1d, i32 %im1db, i32 %im2d, i32 %im3d, i32 %samp, i64 %ptr, <4 x i32> %gen) {
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_2(target("spirv.BufferSurfaceINTEL", 2) [[BUF]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_0_0_0_0_2(target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) [[IM1D]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_5_0_0_0_0_0_2(target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 2) [[IM1DB]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_2(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) [[IM2D]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_2_0_0_0_0_0_2(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 2) [[IM3D]])
; CHECK: call i32 @llvm.genx.address.convert.i32.t_spirv.Sampler(target("spirv.Sampler") [[SAMP]])
; CHECK: ptrtoint ptr addrspace(1) [[PTR]] to i64
; CHECK-NOT: [[GEN]]
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t", !"image1d_t", !"image1d_buffer_t", !"image2d_t", !"image3d_t", !"sampler_t", !"svmptr_t", !""}
