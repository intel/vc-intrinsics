;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test messy annnotations translation in writer. First valid
; annotation should be matched.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: target("spirv.Sampler")
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: ptr addrspace(1)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: i32
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(i32 %im2d, i32 %samp, i64 %ptr, i32 %gen) {
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_0(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) [[IM2D]])
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.t_spirv.Sampler(target("spirv.Sampler") [[SAMP]])
; CHECK-NEXT: ptrtoint ptr addrspace(1) [[PTR]] to i64
; CHECK-NEXT: ret void
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0}
!3 = !{!"image2d_t buffer_t read_only read_write", !"sampler_t read_only", !"svmptr_t write_only", !"write_only"}
