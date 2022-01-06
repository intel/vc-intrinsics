;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; UNSUPPORTED: llvm8
; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s
; CHECK: @test
; CHECK-SAME: i8
; CHECK-SAME: byval(i8)
; CHECK-SAME: arg

%foo = type { i32 }

define spir_kernel void @test(%foo addrspace(1)* byval(%foo) %arg) {
  ret void
}

!genx.kernels = !{!0}
!0 = !{void (%foo addrspace(1)*)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{i32 0}
!2 = !{i32 0}
!3 = !{!"svmptr_t"}
