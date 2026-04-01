;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define internal void @foo(<1 x ptr> %v) {
  ; CHECK: %ld = load i32, ptr %v, align 4
  %ex = extractelement <1 x ptr> %v, i64 0
  %ld = load i32, ptr %ex, align 4
  ret void
}

define internal <1 x ptr> @bar(<1 x ptr> %in, <1 x ptr> %out) {
  ; CHECK: store ptr %in, ptr %out, align 8
  ; CHECK-NEXT: ret ptr %out
  %ex = extractelement <1 x ptr> %out, i64 0
  store <1 x ptr> %in, ptr %ex, align 8
  ret <1 x ptr> %out
}
