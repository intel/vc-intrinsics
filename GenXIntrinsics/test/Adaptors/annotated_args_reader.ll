;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2025 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel argument translation from new style with opaque types
; that SPIRV translator can understand to old style with
; metadata. Here annotations for OCL runtime are used.

; UNSUPPORTED: opaque-pointers
; RUN: opt %pass%GenXSPIRVReaderAdaptor -S < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.image1d_rw_t = type opaque
%opencl.image1d_buffer_rw_t = type opaque
%opencl.image2d_rw_t = type opaque
%opencl.image3d_rw_t = type opaque
%opencl.sampler_t = type opaque

@0 = private unnamed_addr constant [15 x i8] c"some attribute\00", section "llvm.metadata"
; CHECK: @llvm.global.annotations
; CHECK-SAME: void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(1)*, <4 x i32>)* @test
@llvm.global.annotations = appending global [1 x { i8*, i8*, i8*, i32 }] [{ i8*, i8*, i8*, i32 } { i8* bitcast (void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(1)*, <4 x i32>)* @test to i8*), i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i32 0, i32 0), i8* undef, i32 undef }], section "llvm.metadata"

; CHECK: define dllexport spir_kernel void @test(
; CHECK-SAME: %intel.buffer_rw_t addrspace(1)*
; CHECK-SAME: [[BUF:%[^,]+]],
; CHECK-SAME: %opencl.image1d_rw_t addrspace(1)*
; CHECK-SAME: [[IM1D:%[^,]+]],
; CHECK-SAME: %opencl.image1d_buffer_rw_t addrspace(1)*
; CHECK-SAME: [[IM1DB:%[^,]+]],
; CHECK-SAME: %opencl.image2d_rw_t addrspace(1)*
; CHECK-SAME: [[IM2D:%[^,]+]],
; CHECK-SAME: %opencl.image3d_rw_t addrspace(1)*
; CHECK-SAME: [[IM3D:%[^,]+]],
; CHECK-SAME: %opencl.sampler_t addrspace(2)*
; CHECK-SAME: [[SAMP:%[^,]+]],
; CHECK-SAME: i8 addrspace(1)*
; CHECK-SAME: [[PTR:%[^,]+]],
; CHECK-SAME: <4 x i32>
; CHECK-SAME: [[GEN:%[^)]+]])
define spir_kernel void @test(%intel.buffer_rw_t addrspace(1)* %buf, %opencl.image1d_rw_t addrspace(1)* %im1d, %opencl.image1d_buffer_rw_t addrspace(1)* %im1db, %opencl.image2d_rw_t addrspace(1)* %im2d, %opencl.image3d_rw_t addrspace(1)* %im3d, %opencl.sampler_t addrspace(2)* %samp, i8 addrspace(1)* %ptr, <4 x i32> %gen) #0 {
; CHECK-NEXT: ptrtoint %intel.buffer_rw_t addrspace(1)* [[BUF]] to i32
  %buf.conv = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* %buf)
; CHECK-NEXT: ptrtoint %opencl.image1d_rw_t addrspace(1)* [[IM1D]] to i32
  %im1d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* %im1d)
; CHECK-NEXT: ptrtoint %opencl.image1d_buffer_rw_t addrspace(1)* [[IM1DB]] to i32
  %im1db.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_rw_t(%opencl.image1d_buffer_rw_t addrspace(1)* %im1db)
; CHECK-NEXT: ptrtoint %opencl.image2d_rw_t addrspace(1)* [[IM2D]] to i32
  %im2d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_rw_t(%opencl.image2d_rw_t addrspace(1)* %im2d)
; CHECK-NEXT: ptrtoint %opencl.image3d_rw_t addrspace(1)* [[IM3D]] to i32
  %im3d.conv = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_rw_t(%opencl.image3d_rw_t addrspace(1)* %im3d)
; CHECK-NEXT: ptrtoint %opencl.sampler_t addrspace(2)* [[SAMP]] to i32
  %samp.conv = call i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)* %samp)
; CHECK-NEXT: ptrtoint i8 addrspace(1)* [[PTR]] to i64
  %ptr.conv = ptrtoint i8 addrspace(1)* %ptr to i64
; CHECK-NEXT: ret void
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_rw_t(%opencl.image1d_buffer_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image2d_rw_t(%opencl.image2d_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p1opencl.image3d_rw_t(%opencl.image3d_rw_t addrspace(1)*) #0
declare i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)*) #0

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (%intel.buffer_rw_t addrspace(1)*, %opencl.image1d_rw_t addrspace(1)*, %opencl.image1d_buffer_rw_t addrspace(1)*, %opencl.image2d_rw_t addrspace(1)*, %opencl.image3d_rw_t addrspace(1)*, %opencl.sampler_t addrspace(2)*, i8 addrspace(1)*, <4 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"image1d_t read_write", !"image1d_buffer_t read_write", !"image2d_t read_write", !"image3d_t read_write", !"sampler_t", !"svmptr_t", !""}
