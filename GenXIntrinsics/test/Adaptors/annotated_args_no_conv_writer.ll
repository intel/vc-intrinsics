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

; Test that writer does not changes signature if correct
; types are already used. Just drop all annotations.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_rw_t = type opaque
%opencl.image2d_rw_t = type opaque
%opencl.image3d_rw_t = type opaque
%opencl.sampler_t = type opaque

define void @test(%intel.buffer_rw_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_rw_t addrspace(1)* %im1db, %opencl.image2d_rw_t addrspace(1)* %im2d, %opencl.image3d_rw_t addrspace(1)* %im3d, %opencl.sampler_t addrspace(2)* %samp, i8 addrspace(1)* %ptr, <4 x i32> %gen) {
; CHECK-LABEL: @test(

; CHECK: %intel.buffer_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[BUF:%[^,]+]],

; CHECK: %opencl.image1d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM1D:%[^,]+]],

; CHECK: %opencl.image1d_buffer_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM1DB:%[^,]+]],

; CHECK: %opencl.image2d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM2D:%[^,]+]],

; CHECK: %opencl.image3d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM3D:%[^,]+]],

; CHECK: %opencl.sampler_t addrspace(2)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i8 addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[PTR:%[^,]+]],

; CHECK: <4 x i32>
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}
!0 = !{void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(1)*, <4 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t", !"image1d_t", !"image1d_buffer_t", !"image2d_t", !"image3d_t", !"sampler_t", !"svmptr_t", !""}
