;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test reader translation of media block image arguments.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%opencl.image2d_ro_t = type opaque

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %opencl.image2d_ro_t addrspace(1)*
; CHECK-SAME: [[IM2D:%[^,]+]])
define spir_kernel void @test(%opencl.image2d_ro_t addrspace(1)* "VCMediaBlockIO" %im2d) #0 {
; CHECK-NEXT: ptrtoint %opencl.image2d_ro_t addrspace(1)* [[IM2D]] to i32
  %im2d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)* %im2d)
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)*) #0

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%opencl.image2d_ro_t addrspace(1)*)* @test, !"test", [[KINDS:![0-9]+]], i32 0, i32 0, !{{[0-9]+}}, [[DESCS:![0-9]+]], i32 0}
; CHECK-DAG: [[KINDS]] = !{i32 2}
; CHECK-DAG: [[DESCS]] = !{!"image2d_media_block_t read_only"}
