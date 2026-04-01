;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test combined reader translation: kernel has both native SPIRV types
; and impicit arguments. Implicit arguments would not show in normal
; flow, though they appear in old cmc.

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: ptr addrspace(1)
; CHECK-SAME: [[IN:%[^,]+]],
; CHECK-SAME: ptr addrspace(1)
; CHECK-SAME: [[OUT:%[^,]+]],
; CHECK-SAME: <3 x i32>
; CHECK-SAME: [[LOCAL_ID:%[^)]+]])
define spir_kernel void @test(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %in, target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1) %out, <3 x i32> "VCArgumentKind"="24" %__arg_llvm.genx.local.id) #0 {
; CHECK-NEXT: [[IN_CONV:%.*]] = ptrtoint ptr addrspace(1) [[IN]] to i32
  %in.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_0(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0) %in)
; CHECK-NEXT: [[OUT_CONV:%.*]] = ptrtoint ptr addrspace(1) [[OUT]] to i32
  %out.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_1(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1) %out)
; CHECK-NEXT: [[LOCAL_ID_0:%.*]] = extractelement <3 x i32> [[LOCAL_ID]], i32 0
; CHECK-NEXT: [[LD:%.*]] = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 [[IN_CONV]], i32 0, i32 32, i32 [[LOCAL_ID_0]], i32 0)
; CHECK-NEXT: tail call void @llvm.genx.media.st.v8i32(i32 0, i32 [[OUT_CONV]], i32 0, i32 32, i32 [[LOCAL_ID_0]], i32 0, <8 x i32> [[LD]])
  %local.id.0 = extractelement <3 x i32> %__arg_llvm.genx.local.id, i32 0
  %ld = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 %in.conv, i32 0, i32 32, i32 %local.id.0, i32 0)
  tail call void @llvm.genx.media.st.v8i32(i32 0, i32 %out.conv, i32 0, i32 32, i32 %local.id.0, i32 0, <8 x i32> %ld)
; CHECK-NEXT: ret void
  ret void
}

declare <8 x i32> @llvm.genx.media.ld.v8i32(i32, i32, i32, i32, i32, i32)
declare void @llvm.genx.media.st.v8i32(i32, i32, i32, i32, i32, i32, <8 x i32>)
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_0(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 0))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Image_isVoid_1_0_0_0_0_0_1(target("spirv.Image", void, 1, 0, 0, 0, 0, 0, 1))

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{ptr @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 24}
; CHECK-DAG: ![[DESCS]] = !{!"image2d_t read_only", !"image2d_t write_only", !""}
