;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test simple signatures tranform

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: @global1 = internal global <1 x i32> undef, align 4
; CHECK: @global2 = internal global <1 x ptr> undef, align 4
; CHECK: @global3 = external global <1 x ptr>
; CHECK: @global4 = external global ptr
@global1 = internal global i32 undef, align 4 #1
@global2 = internal global ptr undef, align 4 #1
@global3 = external global ptr #1
@global4 = external global ptr #2

; CHECK: define <1 x i32> @f1(<1 x i32> %a, <1 x i32> %b)
define "VCSingleElementVector"="0" i32 @f1(i32 "VCSingleElementVector" %a, i32 "VCSingleElementVector" %b) #0 {
; CHECK: call void @llvm.genx.intr(ptr @global1)
; CHECK: call void @llvm.genx.intr(ptr @global2)
; CHECK: call void @llvm.genx.intr(ptr @global3)
  call void @llvm.genx.intr(ptr @global1)
  call void @llvm.genx.intr(ptr @global2)
  call void @llvm.genx.intr(ptr @global3)
; CHECK: store i32 0, ptr @global1, align 4
; CHECK: store ptr @global1, ptr @global2, align 8
; CHECK: store ptr @global2, ptr @global3, align 8
  store i32 0, ptr @global1, align 4
  store ptr @global1, ptr @global2, align 8
  store ptr @global2, ptr @global3, align 8
; CHECK: ret <1 x i32> %a
  ret i32 %a
}

; CHECK: define i32 @f2(<1 x i32> %a, <1 x i32> %b)
define i32 @f2(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b) #0 {
; CHECK: [[EX:[^ ]+]] = extractelement <1 x i32> %a, i64 0
; CHECK: ret i32 [[EX]]
  ret i32 %a
}

; CHECK: define i32 @f3(i32 %a, <1 x i32> %b)
define i32 @f3(i32 %a, i32 "VCSingleElementVector"="0" %b) #0 {
  ret i32 %a
}

declare void @llvm.genx.intr(ptr)

attributes #0 = { "VCFunction" }
attributes #1 = { "VCGlobalVariable" "VCSingleElementVector"="0" }
attributes #2 = { "VCGlobalVariable" "VCSingleElementVector"="1" }
