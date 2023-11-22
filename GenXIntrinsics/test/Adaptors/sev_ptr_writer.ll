;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; UNSUPPORTED: llvm17, llvm18
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define internal void @foo(<1 x i32*>* %v) #0 {
entry:
  ; CHECK: %ld.v = load i32*, i32** %v, align 8
  ; CHECK: %ld.ex = load i32, i32* %ld.v, align 4
  %ld.v = load <1 x i32*>, <1 x i32*>* %v, align 8
  %ex = extractelement <1 x i32*> %ld.v, i32 0
  %ld.ex = load i32, i32* %ex, align 4
  ret void
}

define internal <1 x i64**>* @bar(<1 x i64**> %in, <1 x i64**>* %out) #0 {
entry:
  ; CHECK: store i64** %in, i64*** %out, align 8
  store <1 x i64**> %in, <1 x i64**>* %out, align 8
  ret <1 x i64**>* %out
}

attributes #0 = { "VCFunction" }
