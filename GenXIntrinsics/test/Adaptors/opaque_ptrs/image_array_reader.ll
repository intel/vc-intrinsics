;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test reader translation of image array arguments.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: i32
; CHECK-SAME: [[IM1DARR:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[IM2DARR:%[^,]+]])
define spir_kernel void @test(target("spirv.Image", void, 0, 0, 1, 0, 0, 0, 0) %im1darr, target("spirv.Image", void, 1, 0, 1, 0, 0, 0, 1) %im2darr) #0 {
; CHECK-NOT: [[IM1DARR]]
; CHECK-NOT: [[IM2DARR]]
  %im1darr.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_1_0_0_0_0(target("spirv.Image", void, 0, 0, 1, 0, 0, 0, 0) %im1darr)
  %im2darr.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_1_0_0_0_1(target("spirv.Image", void, 1, 0, 1, 0, 0, 0, 1) %im2darr)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_0_0_1_0_0_0_0(target("spirv.Image", void, 0, 0, 1, 0, 0, 0, 0))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_1_0_0_0_1(target("spirv.Image", void, 1, 0, 1, 0, 0, 0, 1))

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{ptr @test, !"test", [[KINDS:![0-9]+]], i32 0, i32 0, !{{[0-9]+}}, [[DESCS:![0-9]+]], i32 0}
; CHECK-DAG: [[KINDS]] = !{i32 2, i32 2}
; CHECK-DAG: [[DESCS]] = !{!"image1d_array_t read_only", !"image2d_array_t write_only"}
