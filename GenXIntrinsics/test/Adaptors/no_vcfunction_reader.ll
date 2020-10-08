; Test that reader ignores signature rewriting for kernels
; that are not VCFunction.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

define spir_kernel void @test(i8 addrspace(1) *%ptr) {
; CHECK-LABEL: @test(

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
entry:
  ret void
}

; CHECK-NOT: !genx.kernels
