;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test reader translation of media block image arguments.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: ptr addrspace(1)
; CHECK-SAME: [[IMAGE:%[^)]+]])
define spir_kernel void @test(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) "VCMediaBlockIO" %image) #0 {
; CHECK-NEXT: ptrtoint ptr addrspace(1) [[IMAGE]] to i32
  %image.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_0(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %image)
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_0(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0))

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{ptr @test, !"test", [[KINDS:![0-9]+]], i32 0, i32 0, !{{[0-9]+}}, [[DESCS:![0-9]+]], i32 0}
; CHECK-DAG: [[KINDS]] = !{i32 2}
; CHECK-DAG: [[DESCS]] = !{!"image2d_media_block_t read_only"}
