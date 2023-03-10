;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

target datalayout = "e-p:64:64-i64:64-n8:16:32"
target triple = "spir64"

; CHECK: [[STRUCT1:[^ ]+]] = type { i32, <2 x i32>, [[STRUCT2:[^ ]+]] }  
%struct.sev_test = type { <1 x i32>, <2 x i32>, %struct.sev_test_nested }
; CHECK: [[STRUCT2]] = type { i32*, <2 x i32>*, [[STRUCT1]]* }
%struct.sev_test_nested = type { <1 x i32>*, <2 x i32>*, %struct.sev_test* }

; CHECK: define void @test(i32 "VCSingleElementVector"="0" %sev, i32* "VCSingleElementVector"="0" %sev_ptr)
define void @test(<1 x i32> %sev, <1 x i32>* %sev_ptr) {
; CHECK: %1 = alloca [[STRUCT1]], align 8
  %1 = alloca %struct.sev_test, align 8
; CHECK: %2 = getelementptr inbounds [[STRUCT1]], [[STRUCT1]]* %1, i32 0, i32 0
  %2 = getelementptr inbounds %struct.sev_test, %struct.sev_test* %1, i32 0, i32 0
; CHECK: store i32 %sev, i32* %2, align 4
  store <1 x i32> %sev, <1 x i32>* %2, align 4
; CHECK: %3 = getelementptr inbounds [[STRUCT1]], [[STRUCT1]]* %1, i32 0, i32 2
  %3 = getelementptr inbounds %struct.sev_test, %struct.sev_test* %1, i32 0, i32 2
; CHECK: %4 = getelementptr inbounds [[STRUCT2]], [[STRUCT2]]* %3, i32 0, i32 0
  %4 = getelementptr inbounds %struct.sev_test_nested, %struct.sev_test_nested* %3, i32 0, i32 0
; CHECK: store i32* %sev_ptr, i32** %4, align 8
  store <1 x i32>* %sev_ptr, <1 x i32>** %4, align 8
; CHECK: %5 = getelementptr inbounds [[STRUCT2]], [[STRUCT2]]* %3, i32 0, i32 2
  %5 = getelementptr inbounds %struct.sev_test_nested, %struct.sev_test_nested* %3, i32 0, i32 2
; CHECK: store [[STRUCT1]]* %1, [[STRUCT1]]** %5, align 8
  store %struct.sev_test* %1, %struct.sev_test** %5, align 8
  ret void
}
