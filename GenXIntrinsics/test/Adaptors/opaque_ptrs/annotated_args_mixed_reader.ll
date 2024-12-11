;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that reader can cope with mixed mode when some
; arguments use address convert and some do not.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: i32
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2)
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2)
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM3D:%[^,]+]],
; CHECK-SAME: target("spirv.Sampler") 
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: i64
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: <4 x i32>
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(target("spirv.BufferSurfaceINTEL", 2) %buf, target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) %im1d, target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 2) %im1db, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 2) %im2d, target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 2) %im3d, target("spirv.Sampler") %samp, ptr addrspace(1) %ptr, <4 x i32> %gen) #0 {
; CHECK-NOT: [[BUF]]
; CHECK-NOT: [[IM1D]]
; CHECK-NOT: [[IM1DB]]
; CHECK-NOT: [[IM2D]]
; CHECK-NOT: [[IM3D]]
; CHECK-NOT: [[SAMP]]
; CHECK-NOT: [[PTR]]
; CHECK-NOT: [[GEN]]
  %buf.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_2(target("spirv.BufferSurfaceINTEL", 2) %buf)
  %im1db.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_5_0_0_0_0_0_2(target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 2) %im1db)
  %im3d.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_2_0_0_0_0_0_2(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 2) %im3d)
  %ptr.conv = call i64 @llvm.genx.address.convert.i64.p1(ptr addrspace(1) %ptr)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_2(target("spirv.BufferSurfaceINTEL", 2))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_5_0_0_0_0_0_2(target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 2))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_2_0_0_0_0_0_2(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 2))
declare i64 @llvm.genx.address.convert.i64.p1(ptr addrspace(1))

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{ptr @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"image1d_t read_write", !"image1d_buffer_t read_write", !"image2d_t read_write", !"image3d_t read_write", !"sampler_t", !"svmptr_t", !""}
