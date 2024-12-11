;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

define internal void @foo(ptr "VCSingleElementVector"="0" %v) #0 {
  ; CHECK: [[EX:[^ ]+]] = extractelement <1 x ptr> %v, i64 0
  ; CHECK-NEXT: [[LD:[^ ]+]] = load i32, ptr [[EX]], align 4
  %ld = load i32, ptr %v, align 4
  ret void
}

define internal "VCSingleElementVector"="0" ptr @bar(ptr "VCSingleElementVector"="0" %in, ptr "VCSingleElementVector"="0" %out) #0 {
  ; CHECK: [[EX:[^ ]+]] = extractelement <1 x ptr> %out, i64 0
  ; CHECK-NEXT: [[INS:[^ ]+]] = extractelement <1 x ptr> %in, i64 0
  ; CHECK-NEXT: store ptr [[INS]], ptr [[EX]], align 8
  ; CHECK-NEXT: ret <1 x ptr> %out
  store ptr %in, ptr %out, align 8
  ret ptr %out
}

attributes #0 = { "VCFunction" }
