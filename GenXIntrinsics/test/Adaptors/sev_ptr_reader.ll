;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023-2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

define internal void @foo(i32** "VCSingleElementVector"="1" %v) #0 {
entry:
  ; CHECK: [[SEV:[^ ]+]] = bitcast <1 x i32*>* %v to i32**
  ; CHECK: %ld.v = load i32*, i32** [[SEV]], align 8
  ; CHECK: %ld.ex = load i32, i32* %ld.v, align 4
  %ld.v = load i32*, i32** %v, align 8
  %ld.ex = load i32, i32* %ld.v, align 4
  ret void
}

define internal "VCSingleElementVector"="2" i64*** @bar(i64** "VCSingleElementVector"="2" %in, i64*** "VCSingleElementVector"="2" %out) #0 {
entry:
  ; CHECK: [[SEV:[^ ]+]] = bitcast <1 x i64**>* %out to i64***
  ; CHECK: [[SEVIN:[^ ]+]] = extractelement <1 x i64**> %in, i64 0
  ; CHECK: store i64** [[SEVIN]], i64*** [[SEV]], align 8
  store i64** %in, i64*** %out, align 8
  ; CHECK: ret <1 x i64**>* %out
  ret i64*** %out
}

attributes #0 = { "VCFunction" }
