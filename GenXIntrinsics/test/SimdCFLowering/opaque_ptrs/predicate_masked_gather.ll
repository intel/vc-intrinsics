;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; REQUIRES: opaque-pointers
; RUN: opt -passes=cmsimdcflowering -S < %s | FileCheck %s

; CHECK: @EM = internal global <32 x i1>

@g2 = internal global <32 x i32> undef

define dso_local dllexport void @test_gather(<32 x i16> %mask, <32 x i32> %addrs) {
  %g = alloca <32 x i32>, align 512
  %cmp = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %cmp)
  br i1 %call, label %if.then, label %if.end

; CHECK-LABEL: if.then:
if.then:
; CHECK: [[EM_LOAD1:%.*]] = load <32 x i1>, ptr @EM
; CHECK-NEXT: [[CALL1:%.*]] = call <32 x i32> @llvm.genx.gather.masked.scaled2.v32i32.v32i32.v32i1(i32 2, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> [[EM_LOAD1]])
  %call1 = call <32 x i32> @llvm.genx.gather.masked.scaled2.v32i32.v32i32.v32i1(i32 2, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> <i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1>)
; CHECK: [[EM_LOAD2:%.*]] = load <32 x i1>, ptr @EM
; CHECK-NEXT: [[CALL1_SIMDCFPREDL:%.*]] = select <32 x i1> [[EM_LOAD2:%.*]], <32 x i32> [[CALL1:%.*]]
  store <32 x i32> %call1, ptr %g
  br label %if.end

if.end:
  %ld = load <32 x i32>, ptr %g
  store <32 x i32> %ld, ptr @g2
  ret void
}

declare <32 x i32> @llvm.genx.gather.masked.scaled2.v32i32.v32i32.v32i1(i32, i16, i32, i32, <32 x i32>, <32 x i1>)
declare i1 @llvm.genx.simdcf.any.v32i1(<32 x i1>)
