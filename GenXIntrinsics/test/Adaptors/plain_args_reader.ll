;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel argument translation from new style with opaque types
; that SPIRV translator can understand to old style with
; metadata. Arguments without annotations are used here (CMRT like).

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.sampler_t = type opaque

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %intel.buffer_rw_t addrspace(1)*
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: %opencl.sampler_t addrspace(2)*
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: i64
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: i32
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(%intel.buffer_rw_t addrspace(1)* %buf, %opencl.sampler_t addrspace(2)* %samp, i64 %ptr, i32 %gen) #0 {
; CHECK-NEXT: ptrtoint %intel.buffer_rw_t addrspace(1)* [[BUF]] to i32
  %buf.conv = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* %buf)
; CHECK-NEXT: ptrtoint %opencl.sampler_t addrspace(2)* [[SAMP]] to i32
  %samp.conv = call i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)* %samp)
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)*) #0

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%intel.buffer_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i64, i32)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"sampler_t", !"", !""}
