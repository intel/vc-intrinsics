;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test reader translation of image array arguments.

; UNSUPPORTED: llvm17, llvm18
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%opencl.image1d_array_ro_t = type opaque
%opencl.image2d_array_wo_t = type opaque

define spir_kernel void @test(%opencl.image1d_array_ro_t addrspace(1)* %im1d, %opencl.image2d_array_wo_t addrspace(1)* %im2d) #0 {
; CHECK-LABEL: @test(

; CHECK: i32
; CHECK: [[IM1D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM2D:%[^,]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  %0 = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_array_ro_t(%opencl.image1d_array_ro_t addrspace(1)* %im1d)
  %1 = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_array_wo_t(%opencl.image2d_array_wo_t addrspace(1)* %im2d)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_array_ro_t(%opencl.image1d_array_ro_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_array_wo_t(%opencl.image2d_array_wo_t addrspace(1)*)

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32, i32)* @test, !"test", [[KINDS:![0-9]+]], i32 0, i32 0, !{{[0-9]+}}, [[DESCS:![0-9]+]], i32 0}
; CHECK-DAG: [[KINDS]] = !{i32 2, i32 2}
; CHECK-DAG: [[DESCS]] = !{!"image1d_array_t read_only", !"image2d_array_t write_only"}
