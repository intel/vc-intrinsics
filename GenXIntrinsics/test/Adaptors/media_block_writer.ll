;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of media block images arguments.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

define void @test(i32 %image) {
; CHECK-LABEL: @test(

; CHECK: %intel.image2d_media_block_ro_t addrspace(1)*
; CHECK: [[IMAGE:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1intel.image2d_media_block_ro_t(%intel.image2d_media_block_ro_t addrspace(1)* [[IMAGE]])
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
