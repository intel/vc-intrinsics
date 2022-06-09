;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; XFAIL: llvm13, llvm14, llvm15
; Test reader translation of SPIRV friendly IR types

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

%spirv.Sampler = type opaque
%spirv.Image._void_0_0_1_0_0_0_0 = type opaque
%spirv.Image._void_1_0_1_0_0_0_1 = type opaque
%spirv.Image._void_2_0_0_0_0_0_2 = type opaque

define spir_kernel void @test(%spirv.Sampler addrspace(2)* %smp, %spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)* %im1d, %spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)* %im2d, %spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)* %im3d) #0 {
; CHECK-LABEL: @test(

; CHECK: i32
; CHECK: [[SMP:%[^,]+]],

; CHECK: i32
; CHECK: [[IM1D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM2D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM3D:%[^,]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  %0 = call i32 @llvm.genx.address.convert.i32.p2spirv.Sampler(%spirv.Sampler addrspace(2)* %smp)
  %1 = call i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_0_0_1_0_0_0_0(%spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)* %im1d)
  %2 = call i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_1_0_1_0_0_0_1(%spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)* %im2d)
  %3 = call i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_2_0_0_0_0_0_2(%spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)* %im3d)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p2spirv.Sampler(%spirv.Sampler addrspace(2)*)
declare i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_0_0_1_0_0_0_0(%spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_1_0_1_0_0_0_1(%spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_2_0_0_0_0_0_2(%spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)*)

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32, i32, i32, i32)* @test, !"test", [[KINDS:![0-9]+]], i32 0, i32 0, !{{[0-9]+}}, [[DESCS:![0-9]+]], i32 0}
; CHECK-DAG: [[KINDS]] = !{i32 1, i32 2, i32 2, i32 2}
; CHECK-DAG: [[DESCS]] = !{!"sampler_t", !"image1d_array_t read_only", !"image2d_array_t write_only", !"image3d_t read_write"}
