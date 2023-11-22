;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of image array arguments.

; UNSUPPORTED: llvm17, llvm18
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define void @test(i32 %im1darr, i32 %im2darr) {
; CHECK-LABEL: @test(

; CHECK: %opencl.image1d_array_ro_t addrspace(1)*
; CHECK: [[IM1D:%[^,]+]],

; CHECK: %opencl.image2d_array_wo_t addrspace(1)*
; CHECK: [[IM2D:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint %opencl.image1d_array_ro_t addrspace(1)* [[IM1D]] to i32
; CHECK-NEXT:    [[TMP1:%.*]] = ptrtoint %opencl.image2d_array_wo_t addrspace(1)* [[IM2D]] to i32
; CHECK-NEXT:     ret void
;
entry:
  ret void
}

!genx.kernels = !{!0}

!0 = !{void (i32, i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2}
!2 = !{i32 0, i32 0}
!3 = !{!"image1d_array_t read_only", !"image2d_array_t write_only"}
