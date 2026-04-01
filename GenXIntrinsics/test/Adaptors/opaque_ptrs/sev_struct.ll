;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

target datalayout = "e-p:64:64-i64:64-n8:16:32"
target triple = "spir64"

; CHECK: [[STRUCT1:[^ ]+]] = type { i32, <2 x i32>, [[STRUCT2:[^ ]+]] }
%struct.sev_test = type { <1 x i32>, <2 x i32>, %struct.sev_test_nested }
; CHECK: [[STRUCT2]] = type { ptr, ptr, ptr }
%struct.sev_test_nested = type { ptr, ptr, ptr }

; CHECK: define void @test(i32 "VCSingleElementVector"="0" %sev, ptr %ptr)
define void @test(<1 x i32> %sev, ptr %ptr) {
; CHECK: %1 = alloca [[STRUCT1]], align 8
  %1 = alloca %struct.sev_test, align 8
; CHECK: %2 = getelementptr inbounds [[STRUCT1]], ptr %1, i32 0, i32 0
  %2 = getelementptr inbounds %struct.sev_test, ptr %1, i32 0, i32 0
; CHECK: store i32 %sev, ptr %2, align 4
  store <1 x i32> %sev, ptr %2, align 4
; CHECK: %3 = getelementptr inbounds [[STRUCT1]], ptr %1, i32 0, i32 2
  %3 = getelementptr inbounds %struct.sev_test, ptr %1, i32 0, i32 2
; CHECK: %4 = getelementptr inbounds [[STRUCT2]], ptr %3, i32 0, i32 0
  %4 = getelementptr inbounds %struct.sev_test_nested, ptr %3, i32 0, i32 0
; CHECK: store ptr %ptr, ptr %4, align 8
  store ptr %ptr, ptr %4, align 8
; CHECK: %5 = getelementptr inbounds [[STRUCT2]], ptr %3, i32 0, i32 2
  %5 = getelementptr inbounds %struct.sev_test_nested, ptr %3, i32 0, i32 2
; CHECK: store ptr %1, ptr %5, align 8
  store ptr %1, ptr %5, align 8
  ret void
}
