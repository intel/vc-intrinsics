;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test combined writer translation: kernel has both annotated explicit
; arguments and impicit arguments. Implicit arguments would not show
; in normal flow, though they appear in old cmc.

; XFAIL: llvm13, llvm14
; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s
; RUN: opt %pass%GenXSPIRVWriterAdaptor %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: %opencl.image2d_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IN:%[^,]+]],
; CHECK-SAME: %opencl.image2d_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[OUT:%[^,]+]],
; CHECK-SAME: <3 x i32>
; CHECK-SAME: "VCArgumentKind"="24"
; CHECK-SAME: [[LOCAL_ID:%[^)]+]])
define void @test(i32 %in, i32 %out, <3 x i32> %__arg_llvm.genx.local.id) {
; CHECK-NEXT: [[IN_CONV:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)* [[IN]])
; CHECK-NEXT: [[OUT_CONV:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* [[OUT]])
; CHECK-NEXT: [[LOCAL_ID_X:%.*]] = extractelement <3 x i32> [[LOCAL_ID]], i32 0
; CHECK-NEXT: [[VAL:%.*]] = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 [[IN_CONV]], i32 0, i32 32, i32 [[LOCAL_ID_X]], i32 0)
; CHECK-NEXT: tail call void @llvm.genx.media.st.v8i32(i32 0, i32 [[OUT_CONV]], i32 0, i32 32, i32 [[LOCAL_ID_X]], i32 0, <8 x i32> [[VAL]])
; CHECK-NEXT: ret void
  %local.id.x = extractelement <3 x i32> %__arg_llvm.genx.local.id, i32 0
  %val = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 %in, i32 0, i32 32, i32 %local.id.x, i32 0)
  tail call void @llvm.genx.media.st.v8i32(i32 0, i32 %out, i32 0, i32 32, i32 %local.id.x, i32 0, <8 x i32> %val)
  ret void
}

declare <8 x i32> @llvm.genx.media.ld.v8i32(i32, i32, i32, i32, i32, i32)
declare void @llvm.genx.media.st.v8i32(i32, i32, i32, i32, i32, i32, <8 x i32>)

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, <3 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 24}
!2 = !{i32 0, i32 0}
!3 = !{!"image2d_t read_only", !"image2d_t write_only"}
