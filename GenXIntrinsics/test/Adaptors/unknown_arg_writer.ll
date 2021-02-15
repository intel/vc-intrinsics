; Test writer translation of implicit argument. Implicit arguments
; should not appear in current form after transition from cmc.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

define void @test(<3 x i32> %__arg_llvm.genx.local.id) {
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

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (<3 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !2, i32 0, i32 0}
!1 = !{i32 24}
!2 = !{}
