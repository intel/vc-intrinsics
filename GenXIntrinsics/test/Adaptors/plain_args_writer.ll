;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel arguments translation from old style with metadata to
; new style with opaque types that SPIRV translator can
; understand. Arguments without annotations are used here (CMRT like).
; UNSUPPORTED: llvm17, llvm18
; XFAIL: llvm13, llvm14, llvm15
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s
; RUN: opt %pass%GenXSPIRVWriterAdaptor %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define spir_kernel void @test(i32 %surf, i32 %samp, i64 %ptr, i32 %gen) {
; CHECK-LABEL: @test(

; CHECK: %intel.buffer_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[SURF:%[^,]+]],

; CHECK: %opencl.sampler_t addrspace(2)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i64
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[PTR:%[^,]+]],

; CHECK: i32
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint %intel.buffer_rw_t addrspace(1)* [[SURF]] to i32
; CHECK-NEXT:    [[TMP1:%.*]] = ptrtoint %opencl.sampler_t addrspace(2)* [[SAMP]] to i32
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, i64, i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0}
!3 = !{!"", !"", !"", !""}
