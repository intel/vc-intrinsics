;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test writer translation of image array arguments.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s
; XFAIL: llvm16

define void @test(i32 %im1darr, i32 %im2darr) {
; CHECK-LABEL: @test(

; CHECK: %opencl.image1d_array_ro_t addrspace(1)*
; CHECK: [[IM1D:%[^,]+]],

; CHECK: %opencl.image2d_array_wo_t addrspace(1)*
; CHECK: [[IM2D:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_array_ro_t(%opencl.image1d_array_ro_t addrspace(1)* [[IM1D]])
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_array_wo_t(%opencl.image2d_array_wo_t addrspace(1)* [[IM2D]])
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
