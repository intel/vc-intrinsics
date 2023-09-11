;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2023 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test kernel arguments translation from old style with metadata to
; new style with opaque types that SPIRV translator can
; understand. Here annotations for OCL runtime are used.
; XFAIL: llvm13, llvm14, llvm15, llvm17, llvm18
; RUN: opt %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s
; RUN: opt %pass%GenXSPIRVWriterAdaptor %pass%GenXSPIRVWriterAdaptor -S < %s | FileCheck %s

define void @test(i32 %buf, i32 %im1d, i32 %im1db, i32 %im2d, i32 %im3d, i32 %samp, i64 %ptr, <4 x i32> %gen) {
; CHECK-LABEL: @test(

; CHECK: %intel.buffer_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[BUF:%[^,]+]],

; CHECK: %opencl.image1d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM1D:%[^,]+]],

; CHECK: %opencl.image1d_buffer_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM1DB:%[^,]+]],

; CHECK: %opencl.image2d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM2D:%[^,]+]],

; CHECK: %opencl.image3d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM3D:%[^,]+]],

; CHECK: %opencl.sampler_t addrspace(2)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i8 addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[PTR:%[^,]+]],

; CHECK: <4 x i32>
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = ptrtoint %intel.buffer_rw_t addrspace(1)* [[BUF]] to i32
; CHECK-NEXT:    [[TMP1:%.*]] = ptrtoint %opencl.image1d_rw_t addrspace(1)* [[IM1D]] to i32
; CHECK-NEXT:    [[TMP2:%.*]] = ptrtoint %opencl.image1d_buffer_rw_t addrspace(1)* [[IM1DB]] to i32
; CHECK-NEXT:    [[TMP3:%.*]] = ptrtoint %opencl.image2d_rw_t addrspace(1)* [[IM2D]] to i32
; CHECK-NEXT:    [[TMP4:%.*]] = ptrtoint %opencl.image3d_rw_t addrspace(1)* [[IM3D]] to i32
; CHECK-NEXT:    [[TMP5:%.*]] = ptrtoint %opencl.sampler_t addrspace(2)* [[SAMP]] to i32
; CHECK-NEXT:    [[TMP6:%.*]] = ptrtoint i8 addrspace(1)* [[PTR]] to i64
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, i32, i32, i32, i32, i64, <4 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2, i32 1, i32 0, i32 0}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t", !"image1d_t", !"image1d_buffer_t", !"image2d_t", !"image3d_t", !"sampler_t", !"svmptr_t", !""}
