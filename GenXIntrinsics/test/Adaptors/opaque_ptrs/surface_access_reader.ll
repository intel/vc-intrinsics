;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel surface argument translation from new style with opaque
; types that SPIRV translator can understand to old style with
; metadata. This test checks access qualifiers translation.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: i32
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM3D:%[^)]+]])
define dllexport spir_kernel void @test(target("spirv.BufferSurfaceINTEL", 0) %buf, target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) %im1d, target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 1) %im1db, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1) %im2d, target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %im3d) #0 {
  %buf.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_0(target("spirv.BufferSurfaceINTEL", 0) %buf)
  %im1d.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_0_0_0_0_2(target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2) %im1d)
  %im1db.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_5_0_0_0_0_0_1(target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 1) %im1db)
  %im2d.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_1(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1) %im2d)
  %im3d.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_2_0_0_0_0_0_0(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0) %im3d) 
  ret void
}

declare i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_0(target("spirv.BufferSurfaceINTEL", 0))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_0_0_0_0_2(target("spirv.Image", void, 0, 0, 0, 0, 0, 0, 2))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_5_0_0_0_0_0_1(target("spirv.Image", void, 5, 0, 0, 0, 0, 0, 1))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_1(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_2_0_0_0_0_0_0(target("spirv.Image", void, 2, 0, 0, 0, 0, 0, 0)) 

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{ptr @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_only", !"image1d_t read_write", !"image1d_buffer_t write_only", !"image2d_t write_only", !"image3d_t read_only"}
