;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2024-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel argument translation from new style with opaque types
; that SPIRV translator can understand to old style with
; metadata. Arguments without annotations are used here (CMRT like).

; REQUIRES: opaque-pointers
; RUN: opt -passes=GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: ptr addrspace(1)
; CHECK-SAME: [[SURF:%[^,]+]],
; CHECK-SAME: ptr addrspace(2)
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: i64
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(target("spirv.BufferSurfaceINTEL", 2) %surf, target("spirv.Sampler") %samp, i64 %ptr, i32 %gen) #0 {
; CHECK-NEXT: ptrtoint ptr addrspace(1) [[SURF]] to i32
  %surf.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_2(target("spirv.BufferSurfaceINTEL", 2) %surf)
; CHECK-NEXT: ptrtoint ptr addrspace(2) [[SAMP]] to i32
  %samp.conv = call i32 @llvm.genx.address.convert.i32.t_spirv.Sampler(target("spirv.Sampler") %samp)
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.t_spirv.BufferSurfaceINTEL_2(target("spirv.BufferSurfaceINTEL", 2))
declare i32 @llvm.genx.address.convert.i32.t_spirv.Sampler(target("spirv.Sampler"))

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{ptr @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"sampler_t", !"", !""}
