;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of media block images arguments.

; UNSUPPORTED: llvm17, llvm18
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define void @test(i32 %image) {
; CHECK-LABEL: @test(

; CHECK: %intel.image2d_media_block_ro_t addrspace(1)*
; CHECK: [[IMAGE:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint %intel.image2d_media_block_ro_t addrspace(1)* [[IMAGE]] to i32
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

!genx.kernels = !{!0}

!0 = !{void (i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2}
!2 = !{i32 0}
!3 = !{!"image2d_media_block_t read_only"}
