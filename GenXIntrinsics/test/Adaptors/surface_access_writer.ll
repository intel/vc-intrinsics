; Test kernel surface argument translation from old style with
; metadata to new style with opaque types that SPIRV translator can
; understand. This test checks access qualifiers translation.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

define void @test(i32 %buf, i32 %im1d, i32 %im1db, i32 %im2d, i32 %im3d) {
; CHECK-LABEL: @test(

; CHECK: %intel.buffer_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[BUF:%[^,]+]],

; CHECK: %opencl.image1d_rw_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM1D:%[^,]+]],

; CHECK: %opencl.image1d_buffer_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM1DB:%[^,]+]],

; CHECK: %opencl.image2d_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM2D:%[^,]+]],

; CHECK: %opencl.image3d_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IM3D:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_ro_t(%intel.buffer_ro_t addrspace(1)* [[BUF]])
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_rw_t(%opencl.image1d_rw_t addrspace(1)* [[IM1D]])
; CHECK-NEXT:    [[TMP2:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image1d_buffer_wo_t(%opencl.image1d_buffer_wo_t addrspace(1)* [[IM1DB]])
; CHECK-NEXT:    [[TMP3:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* [[IM2D]])
; CHECK-NEXT:    [[TMP4:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image3d_ro_t(%opencl.image3d_ro_t addrspace(1)* [[IM3D]])
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, i32, i32, i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 2, i32 2, i32 2}
!2 = !{i32 0, i32 0, i32 0, i32 0, i32 0}
!3 = !{!"buffer_t read_only", !"image1d_t read_write", !"image1d_buffer_t write_only", !"image2d_t write_only", !"image3d_t read_only"}
