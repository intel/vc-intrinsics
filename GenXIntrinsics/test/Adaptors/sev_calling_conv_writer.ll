;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2022-2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test GenXSingleElementVectorUtil preserves calling convention
; (spir_func here)

; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; ModuleID = 'sev_calling_conv_reader.ll'
source_filename = "start.ll"
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:64-v96:128:128-v128:128:128-v192:256:256-v256:256:256-v512:512:512-v1024:1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: noinline nounwind
; CHECK: define internal spir_func void @bar(i32 "VCSingleElementVector"="0" %a) #0
define internal spir_func void @bar(<1 x i32> %a) #0 {
  ret void
}

; Function Attrs: noinline nounwind
define dllexport spir_kernel void @foo() #1 !intel_reqd_sub_group_size !0 {
; CHECK: call spir_func void @bar(i32 undef)
  call spir_func void @bar(<1 x i32> undef)
  ret void
}

attributes #0 = { noinline nounwind }
attributes #1 = { noinline nounwind "CMGenxMain" "oclrt"="1" }

!0 = !{i32 1}
