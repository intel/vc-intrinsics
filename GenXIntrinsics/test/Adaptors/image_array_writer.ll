;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of image array arguments.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: %opencl.image1d_array_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1DARR:%[^,]+]],
; CHECK-SAME: %opencl.image2d_array_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM2DARR:%[^)]+]])
define void @test(i32 %im1darr, i32 %im2darr) {
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_array_ro_t(%opencl.image1d_array_ro_t addrspace(1)* [[IM1DARR]])
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_array_wo_t(%opencl.image2d_array_wo_t addrspace(1)* [[IM2DARR]])
; CHECK-NEXT: ret void
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2}
!2 = !{i32 0, i32 0}
!3 = !{!"image1d_array_t read_only", !"image2d_array_t write_only"}
