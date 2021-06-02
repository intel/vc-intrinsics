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

@g1 = internal global <64 x i32> undef

; CHECK: @EM = internal global <32 x i1> 

; CHECK-LABEL: @test
define dso_local dllexport void @test(<32 x i16> %mask) {
entry:
  %g = alloca <64 x i32>, align 512
  %0 = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %0)
  br i1 %call, label %if.then, label %if.end

if.then:
; CHECK:        [[CALL1:%.*]] = call <64 x i32> @_Z24func_replicate_mask_attrv()
  %call1 = call <64 x i32> @_Z24func_replicate_mask_attrv()

; CHECK:         [[EM_LOAD:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT:    [[CHENNELEM:%.*]] = shufflevector <32 x i1> [[EM_LOAD]], <32 x i1> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:    [[CALL1_SIMDCFPREDL:%.*]] = select <64 x i1> [[CHENNELEM]], <64 x i32> [[CALL1]]
  store <64 x i32> %call1, <64 x i32>* %g
  br label %if.end

if.end:
  %1 = load <64 x i32>, <64 x i32>* %g
  store <64 x i32> %1, <64 x i32>* @g1
  ret void
}

; CHECK-LABEL: @test_gather4
define dso_local dllexport void @test_gather4(<32 x i16> %mask, <32 x i32> %addrs) {
entry:
  %g = alloca <64 x i32>, align 512
  %0 = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %0)
  br i1 %call, label %if.then, label %if.end
if.then:
; CHECK-LABEL: if.then:
; CHECK: [[EM_LOAD1:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT: [[CALL1:%.*]] = call <64 x i32> @llvm.genx.gather4.masked.scaled2.v64i32.v32i32.v32i1(i32 12, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> [[EM_LOAD1]])
  %call1 = call <64 x i32> @llvm.genx.gather4.masked.scaled2.v64i32.v32i32.v32i1(i32 12, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> <i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1>)

; CHECK: [[EM_LOAD2:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT: [[CHENNELEM:%.*]] = shufflevector <32 x i1> [[EM_LOAD2]], <32 x i1> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:  [[CALL1_SIMDCFPREDL:%.*]] = select <64 x i1> [[CHENNELEM]], <64 x i32> [[CALL1]]
  store <64 x i32> %call1, <64 x i32>* %g
  br label %if.end

if.end:
  %1 = load <64 x i32>, <64 x i32>* %g
  store <64 x i32> %1, <64 x i32>* @g1
  ret void
}

define internal <64 x i32> @_Z24func_replicate_mask_attrv() #0 {
entry:
  ret <64 x i32> zeroinitializer
}

declare <64 x i32> @llvm.genx.gather4.masked.scaled2.v64i32.v32i32.v32i1(i32, i16, i32, i32, <32 x i32>, <32 x i1>)
declare i1 @llvm.genx.simdcf.any.v32i1(<32 x i1>)

attributes #0 = { noinline nounwind "CMGenxReplicateMask"="2"}
