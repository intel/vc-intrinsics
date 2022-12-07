;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2021-2022 Intel Corporation
;
; SPDX-License-Identifier: MIT
;
;============================ end_copyright_notice =============================

; Test that adaptor correctly handles parameter attributes with types.

; UNSUPPORTED: llvm8
; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s
; CHECK: @test
; CHECK-SAME: (%foo addrspace(1)* byval(%foo) %arg)

%foo = type { i32 }

declare void @bar(%foo addrspace(1)*)

define spir_kernel void @test(i8 addrspace(1)* byval(i8) "VCArgumentIOKind"="0" %arg) #0 {
  %1 = call %foo addrspace(1)* @llvm.genx.address.convert.p1foo.p1i8(i8 addrspace(1)* %arg)
  call void @bar(%foo addrspace(1)* %1)
  ret void
}

; CHECK: @testx
; CHECK-SAME: (%foo addrspace(1)* byval(%foo) %arg)
define spir_kernel void @testx(%foo addrspace(1)* byval(%foo) "VCArgumentIOKind"="0" %arg) #0 {
  %1 = call i8 addrspace(1)* @llvm.genx.address.convert.p1i8.p1foo(%foo addrspace(1)* %arg)
  %2 = bitcast i8 addrspace(1)* %1 to %foo addrspace(1)*
  call void @bar(%foo addrspace(1)* %2)
  ret void
}

declare %foo addrspace(1)* @llvm.genx.address.convert.p1foo.p1i8(i8 addrspace(1)*)
declare i8 addrspace(1)* @llvm.genx.address.convert.p1i8.p1foo(%foo addrspace(1)*)

attributes #0 = { "VCFunction" }
