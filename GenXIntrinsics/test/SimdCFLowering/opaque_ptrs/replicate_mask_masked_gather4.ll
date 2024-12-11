;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; REQUIRES: opaque-pointers
; RUN: opt -passes=cmsimdcflowering -S < %s | FileCheck %s

@Rcp_T2 = internal global <64 x i32> undef

; CHECK: @EM = internal global <32 x i1>

define dso_local dllexport void @test(<32 x i16> %mask, <32 x i32> %addrs) {
  %Rcp_T = alloca <64 x i32>, align 512
  %cmp = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %cmp)
  br i1 %call, label %if.then, label %if.end

; CHECK-LABEL: if.then:
if.then:
; CHECK: [[EM_LOAD1:%.*]] = load <32 x i1>, ptr @EM
; CHECK-NEXT: [[CALL1:%.*]] = call <64 x i32> @llvm.genx.gather4.masked.scaled2.v64i32.v32i32.v32i1(i32 12, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> [[EM_LOAD1]])
  %call1 = call <64 x i32> @llvm.genx.gather4.masked.scaled2.v64i32.v32i32.v32i1(i32 12, i16 0, i32 254, i32 0, <32 x i32> %addrs, <32 x i1> <i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1, i1 1>)
; CHECK: [[EM_LOAD2:%.*]] = load <32 x i1>, ptr @EM
; CHECK-NEXT: [[CHENNELEM:%.*]] = shufflevector <32 x i1> [[EM_LOAD2]], <32 x i1> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:  [[CALL1_SIMDCFPREDL:%.*]] = select <64 x i1> [[CHENNELEM]], <64 x i32> [[CALL1]]
  store <64 x i32> %call1, ptr %Rcp_T
  br label %if.end

if.end:
  %ld = load <64 x i32>, ptr %Rcp_T
  store <64 x i32> %ld, ptr @Rcp_T2
  ret void
}

declare <64 x i32> @llvm.genx.gather4.masked.scaled2.v64i32.v32i32.v32i1(i32, i16, i32, i32, <32 x i32>, <32 x i1>)
declare i1 @llvm.genx.simdcf.any.v32i1(<32 x i1>)
