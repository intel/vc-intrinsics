;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2021 Intel Corporation
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"),
; to deal in the Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute, sublicense,
; and/or sell copies of the Software, and to permit persons to whom the
; Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice (including the next
; paragraph) shall be included in all copies or substantial portions of the
; Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
;
; SPDX-License-Identifier: MIT
;============================ end_copyright_notice =============================

; XFAIL: llvm13
; Test that reader correctly restores metadata and does
; not change other things if there is no address conversion
; but correct SPIRV types in signature.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

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
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(2)*, <4 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"image1d_t read_write", !"image1d_buffer_t read_write", !"image2d_t read_write", !"image3d_t read_write", !"sampler_t", !"svmptr_t", !""}
