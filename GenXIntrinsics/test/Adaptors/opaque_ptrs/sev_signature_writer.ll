;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test simple signatures tranform

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: @global1 = internal global i32 undef, align 4
; CHECK: @global2 = internal global ptr undef, align 4
; CHECK: @global3 = external global ptr
@global1 = internal global <1 x i32> undef, align 4 #0
@global2 = internal global <1 x ptr> undef, align 4 #0
@global3 = external global <1 x ptr> #0

; CHECK: define "VCSingleElementVector"="0" i32 @f1(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b)
define <1 x i32> @f1(<1 x i32> %a, <1 x i32> %b) {
; CHECK: call void @llvm.genx.intr(ptr @global1)
; CHECK: call void @llvm.genx.intr(ptr @global2)
; CHECK: call void @llvm.genx.intr(ptr @global3)
  call void @llvm.genx.intr(ptr @global1)
  call void @llvm.genx.intr(ptr @global2)
  call void @llvm.genx.intr(ptr @global3)
; CHECK: store i32 0, ptr @global1, align 4
; CHECK: store ptr @global1, ptr @global2, align 8
; CHECK: store ptr @global2, ptr @global3, align 8
  store <1 x i32> zeroinitializer, ptr @global1, align 4
  %v1 = insertelement <1 x ptr> undef, ptr @global1, i64 0
  store <1 x ptr> %v1, ptr @global2, align 8
  %v2 = insertelement <1 x ptr> undef, ptr @global2, i64 0
  store <1 x ptr> %v2, ptr @global3, align 8
; CHECK: ret i32 %a
  ret <1 x i32> %a
}

; CHECK: define i32 @f2(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b)
define i32 @f2(<1 x i32> %a, <1 x i32> %b) {
; CHECK ret i32 %a
  %c = extractelement <1 x i32> %a, i64 0
  ret i32 %c
}

; CHECK: define i32 @f3(i32 %a, i32 "VCSingleElementVector"="0" %b)
define i32 @f3(i32 %a, <1 x i32> %b) {
  ret i32 %a
}

declare void @llvm.genx.intr(ptr)

; CHECK: "VCSingleElementVector"="0"
attributes #0 = { "VCGlobalVariable" }
