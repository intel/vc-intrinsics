;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of media block images arguments.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: %opencl.image2d_ro_t addrspace(1)*
; CHECK-SAME: "VCMediaBlockIO"
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IMAGE:%[^)]+]])
define void @test(i32 %image) {
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)* [[IMAGE]])
; CHECK-NEXT: ret void
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2}
!2 = !{i32 0}
!3 = !{!"image2d_media_block_t read_only"}
