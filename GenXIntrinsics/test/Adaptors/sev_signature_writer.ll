;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test simple signatures tranform

; UNSUPPORTED: llvm17, llvm18
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: @global_var_0 = internal global i32 undef, align 4
@global_var_0 = internal global <1 x i32> undef, align 4 #0

; CHECK: @global_var_1 = internal global i32** undef, align 4
@global_var_1 = internal global <1 x i32**> undef, align 4 #0

; CHECK: @global_var_2 = external global i32**
@global_var_2 = external global <1 x i32**> #0

; CHECK: "VCSingleElementVector"="0" i32 @some.func.1(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b)
define dso_local <1 x i32> @some.func.1(<1 x i32> %a, <1 x i32> %b) local_unnamed_addr {
  entry:

; CHECK: call void @llvm.genx.some.intr.0(i32* @global_var_0)
  call void @llvm.genx.some.intr.0(<1 x i32>* @global_var_0)

; CHECK: call void @llvm.genx.some.intr.1(i32*** @global_var_1)
  call void @llvm.genx.some.intr.1(<1 x i32**>* @global_var_1)

; CHECK: call void @llvm.genx.some.intr.1(i32*** @global_var_2)
  call void @llvm.genx.some.intr.1(<1 x i32**>* @global_var_2)

  ret <1 x i32> %a
}

; CHECK: i32 @some.func.2(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b)
define dso_local i32 @some.func.2(<1 x i32> %a, <1 x i32> %b) local_unnamed_addr {
  entry:
  ; CHECK-NOT: extractelement
  %c = extractelement <1 x i32> %a, i32 0
  ret i32 %c
}

; CHECK: i32 @some.func.3(i32 %a, i32 "VCSingleElementVector"="0" %b)
define dso_local i32 @some.func.3(i32 %a, <1 x i32> %b) local_unnamed_addr {
  entry:
  ret i32 %a
}

; CHECK: i32 @some.func.4(i32*** "VCSingleElementVector"="3" %a, i32*** "VCSingleElementVector"="0" %b, i32*** "VCSingleElementVector"="1" %c)
define dso_local i32 @some.func.4(<1 x i32***> %a, <1 x i32>*** %b, <1 x i32*>** %c) local_unnamed_addr {
  entry:
  ret i32 0
}


define spir_kernel void @test() {
entry:
  ret void
}

declare void @llvm.genx.some.intr.0(<1 x i32>*)
declare void @llvm.genx.some.intr.1(<1 x i32**>*)

; CHECK: "VCSingleElementVector"="0"
; CHECK: "VCSingleElementVector"="2"
attributes #0 = { "VCGlobalVariable" }

!genx.kernels = !{!0}

!0 = !{void ()* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{}
!2 = !{}
!3 = !{}
