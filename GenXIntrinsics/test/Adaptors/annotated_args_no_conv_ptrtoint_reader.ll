;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that reader correctly restores metadata and does
; not change other things if there is only ptrtoint
; conversion and correct SPIRV types in signature.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_rw_t = type opaque
%opencl.image2d_rw_t = type opaque
%opencl.image3d_rw_t = type opaque
%opencl.sampler_t = type opaque

define spir_kernel void @test(%intel.buffer_rw_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_rw_t addrspace(1)* %im1db, %opencl.image2d_rw_t addrspace(1)* %im2d, %opencl.image3d_rw_t addrspace(1)* %im3d, %opencl.sampler_t addrspace(2)* %samp, i8 addrspace(2)* %ptr, <4 x i32> %gen) #0 {
; CHECK-LABEL: @test(

; CHECK: %intel.buffer_rw_t addrspace(1)*
; CHECK: [[BUF:%[^,]+]],

; CHECK: %opencl.image1d_rw_t addrspace(1)*
; CHECK: [[IM1D:%[^,]+]],

; CHECK: %opencl.image1d_buffer_rw_t addrspace(1)*
; CHECK: [[IM1DB:%[^,]+]],

; CHECK: %opencl.image2d_rw_t addrspace(1)*
; CHECK: [[IM2D:%[^,]+]],

; CHECK: %opencl.image3d_rw_t addrspace(1)*
; CHECK: [[IM3D:%[^,]+]],

; CHECK: %opencl.sampler_t addrspace(2)*
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i8 addrspace(2)*
; CHECK: [[PTR:%[^,]+]],

; CHECK: <4 x i32>
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ptrtoint %intel.buffer_rw_t addrspace(1)* %buf to i32
; CHECK-NEXT:    ptrtoint %opencl.image1d_rw_t addrspace(1)* %im1d to i32
; CHECK-NEXT:    ptrtoint %opencl.image1d_buffer_rw_t addrspace(1)* %im1db to i32
; CHECK-NEXT:    ptrtoint %opencl.image2d_rw_t addrspace(1)* %im2d to i32
; CHECK-NEXT:    ptrtoint %opencl.image3d_rw_t addrspace(1)* %im3d to i32
; CHECK-NEXT:    ptrtoint %opencl.sampler_t addrspace(2)* %samp to i32
; CHECK-NEXT:    ptrtoint i8 addrspace(2)* %ptr to i64
; CHECK-NEXT:    ret void
;
entry:
  %buf.conv = ptrtoint %intel.buffer_rw_t addrspace(1)* %buf to i32
  %im1d.conv = ptrtoint %opencl.image1d_rw_t addrspace(1)* %im1d to i32
  %im1db.conv = ptrtoint %opencl.image1d_buffer_rw_t addrspace(1)* %im1db to i32
  %im2d.conv = ptrtoint %opencl.image2d_rw_t addrspace(1)* %im2d to i32
  %im3d.conv = ptrtoint %opencl.image3d_rw_t addrspace(1)* %im3d to i32
  %samp.conv = ptrtoint %opencl.sampler_t addrspace(2)* %samp to i32
  %ptr.conv = ptrtoint i8 addrspace(2)* %ptr to i64
  ret void
}

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(2)*, <4 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"image1d_t read_write", !"image1d_buffer_t read_write", !"image2d_t read_write", !"image3d_t read_write", !"sampler_t", !"svmptr_t", !""}
