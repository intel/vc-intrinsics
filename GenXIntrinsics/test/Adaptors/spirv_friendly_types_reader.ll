;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test reader translation of SPIRV friendly IR types

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%spirv.Sampler = type opaque
%spirv.Image._void_0_0_1_0_0_0_0 = type opaque
%spirv.Image._void_1_0_1_0_0_0_1 = type opaque
%spirv.Image._void_2_0_0_0_0_0_2 = type opaque

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)*
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: %spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)*
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: %spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)*
; CHECK-SAME: [[IM3D:%[^,]+]],
; CHECK-SAME: %spirv.Sampler addrspace(2)*
; CHECK-SAME: [[SAMP:%[^,]+]])
define spir_kernel void @test(%spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)* %im1d, %spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)* %im2d, %spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)* %im3d, %spirv.Sampler addrspace(2)* %samp) #0 {
; CHECK-NEXT: ptrtoint %spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)* [[IM1D]] to i32
  %im1d.conv = call i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_0_0_1_0_0_0_0(%spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)* %im1d)
; CHECK-NEXT: ptrtoint %spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)* [[IM2D]] to i32
  %im2d.conv = call i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_1_0_1_0_0_0_1(%spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)* %im2d)
; CHECK-NEXT: ptrtoint %spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)* [[IM3D]] to i32
  %im3d.conv = call i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_2_0_0_0_0_0_2(%spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)* %im3d)
; CHECK-NEXT: ptrtoint %spirv.Sampler addrspace(2)* [[SAMP]] to i32
  %samp.conv = call i32 @llvm.genx.address.convert.i32.p2spirv.Sampler(%spirv.Sampler addrspace(2)* %samp)
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_0_0_1_0_0_0_0(%spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_1_0_1_0_0_0_1(%spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1spirv.Image._void_2_0_0_0_0_0_2(%spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p2spirv.Sampler(%spirv.Sampler addrspace(2)*) #0

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%spirv.Image._void_0_0_1_0_0_0_0 addrspace(1)*, %spirv.Image._void_1_0_1_0_0_0_1 addrspace(1)*, %spirv.Image._void_2_0_0_0_0_0_2 addrspace(1)*, %spirv.Sampler addrspace(2)*)* @test, !"test", [[KINDS:![0-9]+]], i32 0, i32 0, !{{[0-9]+}}, [[DESCS:![0-9]+]], i32 0}
; CHECK-DAG: [[KINDS]] = !{i32 2, i32 2, i32 2, i32 1}
; CHECK-DAG: [[DESCS]] = !{!"image1d_array_t read_only", !"image2d_array_t write_only", !"image3d_t read_write", !"sampler_t"}
