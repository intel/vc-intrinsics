;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that reader correctly restores metadata and does
; not change other things if there is no address conversion
; but correct SPIRV types in signature.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_rw_t = type opaque
%opencl.image2d_rw_t = type opaque
%opencl.image3d_rw_t = type opaque
%opencl.sampler_t = type opaque

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %intel.buffer_rw_t addrspace(1)*
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: %opencl.image1d_rw_t addrspace(1)*
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: %opencl.image1d_buffer_rw_t addrspace(1)*
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: %opencl.image2d_rw_t addrspace(1)*
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: %opencl.image3d_rw_t addrspace(1)*
; CHECK-SAME: [[IM3D:%[^,]+]],
; CHECK-SAME: %opencl.sampler_t addrspace(2)*
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: i8 addrspace(1)*
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: <4 x i32>
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(%intel.buffer_rw_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_rw_t addrspace(1)* %im1db, %opencl.image2d_rw_t addrspace(1)* %im2d, %opencl.image3d_rw_t addrspace(1)* %im3d, %opencl.sampler_t addrspace(2)* %samp, i8 addrspace(1)* %ptr, <4 x i32> %gen) #0 {
; CHECK-NEXT:    ret void
  ret void
}

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(1)*, <4 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"image1d_t read_write", !"image1d_buffer_t read_write", !"image2d_t read_write", !"image3d_t read_write", !"sampler_t", !"svmptr_t", !""}
