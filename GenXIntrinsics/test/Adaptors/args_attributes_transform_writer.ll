;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; UNSUPPORTED: llvm8, llvm17, llvm18
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s
; CHECK: @test
; CHECK-SAME: %foo addrspace(1)* byval(%foo)
; CHECK-SAME: %arg

%foo = type { i32 }

define spir_kernel void @test(%foo addrspace(1)* byval(%foo) %arg) {
  ret void
}

!genx.kernels = !{!0}
!0 = !{void (%foo addrspace(1)*)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{i32 0}
!2 = !{i32 0}
!3 = !{!"svmptr_t"}
