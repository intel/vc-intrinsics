; Test that reader treats only global pointer as svmptr type
; and ignores other address spaces.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

define spir_kernel void @test(i32* %ptr) #0 {
; CHECK-LABEL: @test(

; CHECK: i32*
; CHECK: "VCArgumentKind"="0"
; CHECK: [[PTR:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32*)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, !{{[0-9]+}}, i32 0}
; CHECK: ![[KINDS]] = !{i32 0}
