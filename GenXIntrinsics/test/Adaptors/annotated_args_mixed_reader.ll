;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that reader can cope with mixed mode when some
; arguments use address convert and some do not.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_rw_t = type opaque
%opencl.image2d_rw_t = type opaque
%opencl.image3d_rw_t = type opaque
%opencl.sampler_t = type opaque

define spir_kernel void @test(%intel.buffer_rw_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_rw_t addrspace(1)* %im1db, %opencl.image2d_rw_t addrspace(1)* %im2d, %opencl.image3d_rw_t addrspace(1)* %im3d, %opencl.sampler_t addrspace(2)* %samp, i8 addrspace(1)* %ptr, <4 x i32> %gen) #0 {
; CHECK-LABEL: @test(

; CHECK: i32
; CHECK: [[BUF:%[^,]+]],

; CHECK: %opencl.image1d_rw_t addrspace(1)*
; CHECK: [[IM1D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM1DB:%[^,]+]],

; CHECK: %opencl.image2d_rw_t addrspace(1)*
; CHECK: [[IM2D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM3D:%[^,]+]],

; CHECK: %opencl.sampler_t addrspace(2)*
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i64
; CHECK: [[PTR:%[^,]+]],

; CHECK: <4 x i32>
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  %0 = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* %buf)
  %1 = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_rw_t(%opencl.image1d_buffer_rw_t addrspace(1)* %im1db)
  %2 = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_rw_t(%opencl.image3d_rw_t addrspace(1)* %im3d)
  %3 = call i64 @llvm.genx.address.convert.i64.p1i8(i8 addrspace(1)* %ptr)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_rw_t(%opencl.image1d_buffer_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_rw_t(%opencl.image2d_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image3d_rw_t(%opencl.image3d_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)*)
declare i64 @llvm.genx.address.convert.i64.p1i8(i8 addrspace(1)*)

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32, %opencl.image1d_rw_t addrspace(1)*, i32, %opencl.image2d_rw_t addrspace(1)*, i32, %opencl.sampler_t addrspace(2)*, i64, <4 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"image1d_t read_write", !"image1d_buffer_t read_write", !"image2d_t read_write", !"image3d_t read_write", !"sampler_t", !"svmptr_t", !""}
