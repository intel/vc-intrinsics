;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly translates function attributes to VC-specific
; metadata (the processed attributes are expected to be discarded)

; UNSUPPORTED: llvm8
; XFAIL: llvm16
; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s
; CHECK: @test_VCFunction()
; CHECK: @test_VCStackCall()
; CHECK-SAME: #[[FATR_STACK_CALL_ATTR_IDX:[0-9]+]]
; CHECK: @test_VCCallable()
; CHECK-SAME: #[[FATR_CALLABLE_ATTR_IDX:[0-9]+]]
; CHECK: @test_VCFCEntry()
; CHECK-SAME: #[[FATR_FC_ENTRY_IDX:[0-9]+]]
; CHECK: @test_VCSIMTCall()
; CHECK-SAME: #[[FATR_SIMT_CALL_IDX:[0-9]+]]
; CHECK: @test_VCFloatControl()
; CHECK-SAME: #[[FATR_FLOAT_CONTROL_IDX:[0-9]+]]
; CHECK: @test_VCSLMSize()
; CHECK-SAME: #[[FATR_SLM_SIZE_IDX:[0-9]+]]

define void @test_VCFunction() #0 {
  ret void
}
define void @test_VCStackCall() #1 {
  ret void
}
define void @test_VCCallable() #2 {
  ret void
}
define void @test_VCFCEntry() #3 {
  ret void
}
define void @test_VCSIMTCall() #4 {
  ret void
}
define void @test_VCFloatControl() #5 {
  ret void
}
define spir_kernel void @test_VCSLMSize() #6 {
  ret void
}

; CHECK-DAG: attributes #[[FATR_STACK_CALL_ATTR_IDX]] = { "CMStackCall" }
; CHECK-DAG: attributes #[[FATR_CALLABLE_ATTR_IDX]] = { "CMCallable" }
; CHECK-DAG: attributes #[[FATR_FC_ENTRY_IDX]] = { "CMEntry" }
; CHECK-DAG: attributes #[[FATR_SIMT_CALL_IDX]] = { "CMGenxSIMT" }
; CHECK-DAG: attributes #[[FATR_FLOAT_CONTROL_IDX]] = { "CMFloatControl"="0" }
; CHECK-DAG: attributes #[[FATR_SLM_SIZE_IDX]] = { "CMGenxMain" }

; CHECK-DAG: !{void ()* @test_VCSLMSize, !"test_VCSLMSize", !{{[0-9]+}}, i32 100500, i32 0, !{{[0-9]+}}, !{{[0-9]+}}, i32 0}

attributes #0 = { "VCFunction" }
attributes #1 = { "VCFunction" "VCStackCall" }
attributes #2 = { "VCFunction" "VCCallable" }
attributes #3 = { "VCFunction" "VCFCEntry" }
attributes #4 = { "VCFunction" "VCSIMTCall" }
attributes #5 = { "VCFunction" "VCFloatControl"="0" }
attributes #6 = { "VCFunction" "VCSLMSize"="100500" }

