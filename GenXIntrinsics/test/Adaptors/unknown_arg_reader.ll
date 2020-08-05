; Test reader translation of implicit argument with argument kind
; decoration.

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

define spir_kernel void @test(<3 x i32> "VCArgumentKind"="24" %__arg_llvm.genx.local.id) #0 {
; CHECK-LABEL: @test(

; CHECK: <3 x i32>
; CHECK: "VCArgumentKind"="24"
; CHECK: [[LOCAL_ID:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  ret void
}

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (<3 x i32>)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, !{{[0-9]+}}, i32 0}
; CHECK: ![[KINDS]] = !{i32 24}
