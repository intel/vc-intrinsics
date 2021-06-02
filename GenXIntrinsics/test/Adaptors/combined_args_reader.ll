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
; Test combined reader translation: kernel has both native SPIRV types
; and impicit arguments. Implicit arguments would not show in normal
; flow, though they appear in old cmc.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

%opencl.image2d_ro_t = type opaque
%opencl.image2d_wo_t = type opaque

define spir_kernel void @test(%opencl.image2d_ro_t addrspace(1)* %in, %opencl.image2d_wo_t addrspace(1)* %out, <3 x i32> "VCArgumentKind"="24" %__arg_llvm.genx.local.id) #0 {
; CHECK-LABEL: @test(

; CHECK: i32
; CHECK: [[IN:%[^,]+]],

; CHECK: i32
; CHECK: [[OUT:%[^,]+]],

; CHECK: <3 x i32>
; CHECK: [[LOCAL_ID:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = extractelement <3 x i32> [[LOCAL_ID]], i32 0
; CHECK-NEXT:    [[CALL1_I_I_I:%.*]] = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 [[IN]], i32 0, i32 32, i32 [[TMP0]], i32 0)
; CHECK-NEXT:    tail call void @llvm.genx.media.st.v8i32(i32 0, i32 [[OUT]], i32 0, i32 32, i32 [[TMP0]], i32 0, <8 x i32> [[CALL1_I_I_I]])
; CHECK-NEXT:    ret void
;
entry:
  %0 = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)* %in)
  %1 = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* %out)
  %2 = extractelement <3 x i32> %__arg_llvm.genx.local.id, i32 0
  %call1.i.i.i = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 %0, i32 0, i32 32, i32 %2, i32 0)
  tail call void @llvm.genx.media.st.v8i32(i32 0, i32 %1, i32 0, i32 32, i32 %2, i32 0, <8 x i32> %call1.i.i.i)
  ret void
}

declare <8 x i32> @llvm.genx.media.ld.v8i32(i32, i32, i32, i32, i32, i32)
declare void @llvm.genx.media.st.v8i32(i32, i32, i32, i32, i32, i32, <8 x i32>)
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)*) #0

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32, i32, <3 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 24}
; CHECK-DAG: ![[DESCS]] = !{!"image2d_t read_only", !"image2d_t write_only", !""}
