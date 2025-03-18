;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel surface argument translation from old style with
; metadata to new style with opaque types that SPIRV translator can
; understand. This test checks access qualifiers translation.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

; CHECK: define spir_kernel void @test(
; CHECK-SAME: %intel.buffer_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: %opencl.image1d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: %opencl.image1d_buffer_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: %opencl.image2d_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: %opencl.image3d_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK-SAME: [[IM3D:%[^)]+]])
define void @test(i32 %buf, i32 %im1d, i32 %im1db, i32 %im2d, i32 %im3d) {
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1intel.buffer_ro_t(%intel.buffer_ro_t addrspace(1)* [[BUF]])
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* [[IM1D]])
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_wo_t(%opencl.image1d_buffer_wo_t addrspace(1)* [[IM1DB]])
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* [[IM2D]])
; CHECK-NEXT: call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_ro_t(%opencl.image3d_ro_t addrspace(1)* [[IM3D]])
; CHECK-NEXT: ret void
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, i32, i32, i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t read_only", !"image1d_t read_write", !"image1d_buffer_t write_only", !"image2d_t write_only", !"image3d_t read_only"}
