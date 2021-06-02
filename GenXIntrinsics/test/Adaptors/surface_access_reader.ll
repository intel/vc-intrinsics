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
; Test kernel surface argument translation from new style with opaque
; types that SPIRV translator can understand to old style with
; metadata. This test checks access qualifiers translation.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

%intel.buffer_ro_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_wo_t = type opaque
%opencl.image2d_wo_t = type opaque
%opencl.image3d_ro_t = type opaque

define spir_kernel void @test(%intel.buffer_ro_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_wo_t addrspace(1)* %im1db, %opencl.image2d_wo_t addrspace(1)* %im2d, %opencl.image3d_ro_t addrspace(1)* %im3d) #0 {
; CHECK-LABEL: @test(

; CHECK: i32
; CHECK: [[BUF:%[^,]+]],

; CHECK: i32
; CHECK: [[IM1D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM1DB:%[^,]+]],

; CHECK: i32
; CHECK: [[IM2D:%[^,]+]],

; CHECK: i32
; CHECK: [[IM3D:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  %0 = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_ro_t(%intel.buffer_ro_t addrspace(1)* %buf)
  %1 = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* %im1d)
  %2 = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_wo_t(%opencl.image1d_buffer_wo_t addrspace(1)* %im1db)
  %3 = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* %im2d)
  %4 = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_ro_t(%opencl.image3d_ro_t addrspace(1)* %im3d)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1intel.buffer_ro_t(%intel.buffer_ro_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_wo_t(%opencl.image1d_buffer_wo_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image3d_ro_t(%opencl.image3d_ro_t addrspace(1)*)

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32, i32, i32, i32, i32)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_only", !"image1d_t read_write", !"image1d_buffer_t write_only", !"image2d_t write_only", !"image3d_t read_only"}
