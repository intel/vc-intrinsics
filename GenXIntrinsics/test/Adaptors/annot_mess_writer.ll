;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test messy annnotations translation in writer. First valid
; annotation should be matched.

; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define void @test(i32 %im2d, i32 %samp, i64 %ptr, i32 %gen) {
; CHECK-LABEL: @test(

; CHECK: %opencl.image2d_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM2D:%[^,]+]],

; CHECK: %opencl.sampler_t addrspace(2)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i8 addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[PTR:%[^,]+]],

; CHECK: i32
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint %opencl.image2d_ro_t addrspace(1)* [[IM2D]] to i32
; CHECK-NEXT:    [[TMP1:%.*]] = ptrtoint %opencl.sampler_t addrspace(2)* [[SAMP]] to i32
; CHECK-NEXT:    [[TMP2:%.*]] = ptrtoint i8 addrspace(1)* [[PTR:%.*]] to i64
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, i64, i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0}
!3 = !{!"image2d_t buffer_t read_only read_write", !"sampler_t read_only", !"svmptr_t write_only", !"write_only"}
