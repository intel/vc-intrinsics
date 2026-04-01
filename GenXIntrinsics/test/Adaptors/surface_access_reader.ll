;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel surface argument translation from new style with opaque
; types that SPIRV translator can understand to old style with
; metadata. This test checks access qualifiers translation.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%intel.buffer_ro_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_wo_t = type opaque
%opencl.image2d_wo_t = type opaque
%opencl.image3d_ro_t = type opaque

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %intel.buffer_ro_t addrspace(1)*
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: %opencl.image1d_rw_t addrspace(1)*
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: %opencl.image1d_buffer_wo_t addrspace(1)*
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: %opencl.image2d_wo_t addrspace(1)*
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: %opencl.image3d_ro_t addrspace(1)*
; CHECK-SAME: [[IM3D:%[^,]+]])
define spir_kernel void @test(%intel.buffer_ro_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_wo_t addrspace(1)* %im1db, %opencl.image2d_wo_t addrspace(1)* %im2d, %opencl.image3d_ro_t addrspace(1)* %im3d) #0 {
; CHECK-NEXT: ptrtoint %intel.buffer_ro_t addrspace(1)* [[BUF]] to i32
  %buf.conv = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_ro_t(%intel.buffer_ro_t addrspace(1)* %buf)
; CHECK-NEXT: ptrtoint %opencl.image1d_rw_t addrspace(1)* [[IM1D]] to i32
  %im1d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* %im1d)
; CHECK-NEXT: ptrtoint %opencl.image1d_buffer_wo_t addrspace(1)* [[IM1DB]] to i32
  %im1db.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_wo_t(%opencl.image1d_buffer_wo_t addrspace(1)* %im1db)
; CHECK-NEXT: ptrtoint %opencl.image2d_wo_t addrspace(1)* [[IM2D]] to i32
  %im2d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* %im2d)
; CHECK-NEXT: ptrtoint %opencl.image3d_ro_t addrspace(1)* [[IM3D]] to i32
  %im3d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_ro_t(%opencl.image3d_ro_t addrspace(1)* %im3d)
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1intel.buffer_ro_t(%intel.buffer_ro_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_wo_t(%opencl.image1d_buffer_wo_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image3d_ro_t(%opencl.image3d_ro_t addrspace(1)*) #0

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%intel.buffer_ro_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_wo_t addrspace(1)*, %opencl.image2d_wo_t addrspace(1)*, %opencl.image3d_ro_t addrspace(1)*)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_only", !"image1d_t read_write", !"image1d_buffer_t write_only", !"image2d_t write_only", !"image3d_t read_only"}
