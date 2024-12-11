;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test combined writer translation: kernel has both annotated explicit
; arguments and impicit arguments. Implicit arguments would not show
; in normal flow, though they appear in old cmc.
; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVWriterAdaptor -S < %s | FileCheck %s
; RUN: opt -passes=GenXSPIRVWriterAdaptor,GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IN:%[^,]+]],
; CHECK-SAME: target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1)
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[OUT:%[^,]+]],
; CHECK-SAME: <3 x i32>
; CHECK-SAME: "VCArgumentKind"="24"
; CHECK-SAME: [[LOCAL_ID:%[^)]+]])
define spir_kernel void @test(i32 %in, i32 %out, <3 x i32> %__arg_llvm.genx.local.id) {
; CHECK: [[IN_CONV:%.*]] = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_0(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) [[IN]])
; CHECK: [[OUT_CONV:%.*]] = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_1(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1) [[OUT]])
; CHECK: [[LOCAL_ID_0:%.*]] = extractelement <3 x i32> [[LOCAL_ID]], i32 0
; CHECK-NEXT: [[LD:%.*]] = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 [[IN_CONV]], i32 0, i32 32, i32 [[LOCAL_ID_0]], i32 0)
; CHECK-NEXT: tail call void @llvm.genx.media.st.v8i32(i32 0, i32 [[OUT_CONV]], i32 0, i32 32, i32 [[LOCAL_ID_0]], i32 0, <8 x i32> [[LD]])
  %local.id.0 = extractelement <3 x i32> %__arg_llvm.genx.local.id, i32 0
  %ld = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 %in, i32 0, i32 32, i32 %local.id.0, i32 0)
  tail call void @llvm.genx.media.st.v8i32(i32 0, i32 %out, i32 0, i32 32, i32 %local.id.0, i32 0, <8 x i32> %ld)
  ret void
}

declare <8 x i32> @llvm.genx.media.ld.v8i32(i32, i32, i32, i32, i32, i32)
declare void @llvm.genx.media.st.v8i32(i32, i32, i32, i32, i32, i32, <8 x i32>)

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{ptr @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 24}
!2 = !{i32 0, i32 0}
!3 = !{!"image2d_t read_only", !"image2d_t write_only"}
