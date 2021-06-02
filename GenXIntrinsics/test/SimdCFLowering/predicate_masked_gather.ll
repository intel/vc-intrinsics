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
; RUN: opt -S -cmsimdcflowering < %s | FileCheck %s

; CHECK: @EM = internal global <32 x i1> 

@g2 = internal global <32 x i32> undef

define dso_local dllexport void @test_gather(<32 x i16> %mask, <32 x i32> %addrs) {
entry:
  %g = alloca <32 x i32>, align 512
  %0 = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %0)
  br i1 %call, label %if.then, label %if.end
if.then:
; CHECK-LABEL: if.then:
; CHECK: [[EM_LOAD1:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT: [[CALL1:%.*]] = call <32 x i32> @llvm.genx.gather.masked.scaled2.v32i32.v32i32.v32i1(i32 2, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> [[EM_LOAD1]])
  %call1 = call <32 x i32> @llvm.genx.gather.masked.scaled2.v32i32.v32i32.v32i1(i32 2, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> <i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1>)

; CHECK: [[EM_LOAD2:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT:  [[CALL1_SIMDCFPREDL:%.*]] = select <32 x i1> [[EM_LOAD2:%.*]], <32 x i32> [[CALL1:%.*]]
  store <32 x i32> %call1, <32 x i32>* %g
  br label %if.end

if.end:
  %1 = load <32 x i32>, <32 x i32>* %g
  store <32 x i32> %1, <32 x i32>* @g2
  ret void
}

declare <32 x i32> @llvm.genx.gather.masked.scaled2.v32i32.v32i32.v32i1(i32, i16, i32, i32, <32 x i32>, <32 x i1>)
declare i1 @llvm.genx.simdcf.any.v32i1(<32 x i1>)
