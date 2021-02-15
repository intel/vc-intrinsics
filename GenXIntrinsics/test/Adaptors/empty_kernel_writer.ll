; Test empty kernel metadata translation: old -> new.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

; CHECK: @test() #[[ATTR_GROUP:[0-9]+]]
define void @test() #0 {
  ret void
}

; CHECK: attributes #[[ATTR_GROUP]] = {
; CHECK-DAG: "VCFunction"
; CHECK-DAG: "VCSLMSize"="0"
; CHECK: }
attributes #0 = { "CMGenxMain" }

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void ()* @test, !"test", !1, i32 0, i32 0, !1, !1, i32 0, i32 0}
!1 = !{}
