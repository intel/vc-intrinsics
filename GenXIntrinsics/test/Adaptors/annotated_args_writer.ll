; Test kernel arguments translation from old style with metadata to
; new style with opaque types that SPIRV translator can
; understand. Here annotations for OCL runtime are used.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

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
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* [[BUF]])
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* [[IM1D]])
; CHECK-NEXT:    [[TMP2:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_rw_t(%opencl.image1d_buffer_rw_t addrspace(1)* [[IM1DB]])
; CHECK-NEXT:    [[TMP3:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_rw_t(%opencl.image2d_rw_t addrspace(1)* [[IM2D]])
; CHECK-NEXT:    [[TMP4:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_rw_t(%opencl.image3d_rw_t addrspace(1)* [[IM3D]])
; CHECK-NEXT:    [[TMP5:%.*]] = call i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)* [[SAMP]])
; CHECK-NEXT:    [[TMP6:%.*]] = call i64 @llvm.genx.address.convert.i64.p1i8(i8 addrspace(1)* [[PTR]])
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
