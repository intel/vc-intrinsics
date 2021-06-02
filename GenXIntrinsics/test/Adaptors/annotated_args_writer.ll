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

; Test kernel arguments translation from old style with metadata to
; new style with opaque types that SPIRV translator can
; understand. Here annotations for OCL runtime are used.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s
; RUN: opt -S -GenXSPIRVWriterAdaptor -GenXSPIRVWriterAdaptor < %s | FileCheck %s

define void @test(i32 %buf, i32 %im1d, i32 %im1db, i32 %im2d, i32 %im3d, i32 %samp, i64 %ptr, <4 x i32> %gen) {
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
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* [[BUF]])
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* [[IM1D]])
; CHECK-NEXT:    [[TMP2:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_rw_t(%opencl.image1d_buffer_rw_t addrspace(1)* [[IM1DB]])
; CHECK-NEXT:    [[TMP3:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_rw_t(%opencl.image2d_rw_t addrspace(1)* [[IM2D]])
; CHECK-NEXT:    [[TMP4:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_rw_t(%opencl.image3d_rw_t addrspace(1)* [[IM3D]])
; CHECK-NEXT:    [[TMP5:%.*]] = call i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)* [[SAMP]])
; CHECK-NEXT:    [[TMP6:%.*]] = call i64 @llvm.genx.address.convert.i64.p1i8(i8 addrspace(1)* [[PTR]])
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, i32, i32, i32, i32, i64, <4 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t", !"image1d_t", !"image1d_buffer_t", !"image2d_t", !"image3d_t", !"sampler_t", !"svmptr_t", !""}
