;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test @llvm.genx.address.convert intrinsic generation with proper attributes

; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define void @test(i32 %buf) {
; CHECK-LABEL: @test(
; CHECK: %intel.buffer_rw_t addrspace(1)*
; CHECK: [[BUF:%[^,]+]])
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint %intel.buffer_rw_t addrspace(1)* [[BUF]] to i32
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

!genx.kernels = !{!0}

!0 = !{void (i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2}
!2 = !{i32 0}
!3 = !{!"buffer_t"}
