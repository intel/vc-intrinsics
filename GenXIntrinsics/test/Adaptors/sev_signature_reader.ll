;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; XFAIL: llvm16
; Test simple signatures tranform

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

; CHECK: @global_var_0 = internal global <1 x i32> undef, align 4
@global_var_0 = internal global i32 undef, align 4 #2

; CHECK: @global_var_1 = internal global <1 x i32**> undef, align 4
@global_var_1 = internal global i32** undef, align 4 #3

; CHECK: @global_var_2 = external global <1 x i32**>
@global_var_2 = external global i32** #3

; CHECK: @global_var_3 = internal global i32** undef, align 4
@global_var_3 = internal global i32** undef, align 4

; CHECK: <1 x i32> @some.func.1(<1 x i32> %a, <1 x i32> %b)
define internal "VCSingleElementVector" i32 @some.func.1(i32 "VCSingleElementVector" %a, i32 "VCSingleElementVector" %b) local_unnamed_addr #0 {
entry:

; CHECK: call void @llvm.genx.some.intr.0(<1 x i32>* @global_var_0)
  call void @llvm.genx.some.intr.0(i32* @global_var_0)

; CHECK: call void @llvm.genx.some.intr.1(<1 x i32**>* @global_var_1)
  call void @llvm.genx.some.intr.1(i32*** @global_var_1)

; CHECK: call void @llvm.genx.some.intr.1(<1 x i32**>* @global_var_2)
  call void @llvm.genx.some.intr.1(i32*** @global_var_2)

  ret i32 %a
}

; CHECK: i32 @some.func.2(<1 x i32> %a, <1 x i32> %b)
define internal i32 @some.func.2(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b) local_unnamed_addr #0 {
entry:
  ret i32 %a
}

; CHECK: i32 @some.func.3(i32 %a, <1 x i32> %b)
define internal i32 @some.func.3(i32 %a, i32 "VCSingleElementVector"="0" %b) local_unnamed_addr #0 {
entry:
  ret i32 %a
}

; CHECK: i32 @some.func.4(<1 x i32***> %a, <1 x i32>*** %b, <1 x i32*>** %c)
define internal i32 @some.func.4(i32*** "VCSingleElementVector"="3" %a, i32*** "VCSingleElementVector"="0" %b, i32*** "VCSingleElementVector"="1" %c) local_unnamed_addr #0 {
entry:
  ret i32 0
}

define internal dllexport spir_kernel void @test() #1 {
entry:
  ret void
}

declare void @llvm.genx.some.intr.0(i32* "VCSingleElementVector")
declare void @llvm.genx.some.intr.1(i32*** "VCSingleElementVector"="2")

attributes #0 = { "VCFunction" }
attributes #1 = { "VCFunction" "VCSLMSize"="0" }
attributes #2 = { "VCGlobalVariable" "VCSingleElementVector"="0" }
attributes #3 = { "VCGlobalVariable" "VCSingleElementVector"="2" }
