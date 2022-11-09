;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test @llvm.genx.address.convert intrinsic generation with proper attributes

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

define void @test(i32 %buf) {
; CHECK-LABEL: @test(
; CHECK: %intel.buffer_rw_t addrspace(1)*
; CHECK: [[BUF:%[^,]+]])
; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* [[BUF]])
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

; CHECK: declare !genx_intrinsic_id !{{[0-9]+}} i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)*) #[[ATTRS:[0-9]+]]
!genx.kernels = !{!0}

; CHECK: attributes #[[ATTRS]]
; CHECK-SAME: "VCFunction"

!0 = !{void (i32)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2}
!2 = !{i32 0}
!3 = !{!"buffer_t"}
