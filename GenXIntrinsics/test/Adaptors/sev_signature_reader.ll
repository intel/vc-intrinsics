; XFAIL: llvm13
; Test simple signatures tranform

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

; CHECK: <1 x i32> @some.func.1(<1 x i32> %a, <1 x i32> %b)
define internal "VCSingleElementVector" i32 @some.func.1(i32 "VCSingleElementVector" %a, i32 "VCSingleElementVector" %b) local_unnamed_addr #0 {
entry:
  ret i32 %a
}

; CHECK: i32 @some.func.2(<1 x i32> %a, <1 x i32> %b)
define internal i32 @some.func.2(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b) local_unnamed_addr #0 {
entry:
  ret i32 %a
}

; CHECK: i32 @some.func.3(i32 %a, <1 x i32> %b)
define internal i32 @some.func.3(i32 %a, i32 "VCSingleElementVector"="0" %b) local_unnamed_addr #0 {
entry:
  ret i32 %a
}

; CHECK: i32 @some.func.4(<1 x i32***> %a, <1 x i32>*** %b, <1 x i32*>** %c)
define internal i32 @some.func.4(i32*** "VCSingleElementVector"="3" %a, i32*** "VCSingleElementVector"="0" %b, i32*** "VCSingleElementVector"="1" %c) local_unnamed_addr #0 {
entry:
  ret i32 0
}

define internal dllexport spir_kernel void @test() #1 {
entry:
  ret void
}

attributes #0 = { "VCFunction" }
attributes #1 = { "VCFunction" "VCNamedBarrierCount"="0" "VCSLMSize"="0" }
