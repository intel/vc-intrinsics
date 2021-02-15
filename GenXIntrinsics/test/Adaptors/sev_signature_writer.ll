; Test simple signatures tranform

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

; CHECK: "VCSingleElementVector"="0" i32 @some.func.1(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b)
define dso_local <1 x i32> @some.func.1(<1 x i32> %a, <1 x i32> %b) local_unnamed_addr {
  entry:
  ret <1 x i32> %a
}

; CHECK: i32 @some.func.2(i32 "VCSingleElementVector"="0" %a, i32 "VCSingleElementVector"="0" %b)
define dso_local i32 @some.func.2(<1 x i32> %a, <1 x i32> %b) local_unnamed_addr {
  entry:
  ; CHECK-NOT: extractelement
  %c = extractelement <1 x i32> %a, i32 0
  ret i32 %c
}

; CHECK: i32 @some.func.3(i32 %a, i32 "VCSingleElementVector"="0" %b)
define dso_local i32 @some.func.3(i32 %a, <1 x i32> %b) local_unnamed_addr {
  entry:
  ret i32 %a
}

; CHECK: i32 @some.func.4(i32*** "VCSingleElementVector"="3" %a, i32*** "VCSingleElementVector"="0" %b, i32*** "VCSingleElementVector"="1" %c)
define dso_local i32 @some.func.4(<1 x i32***> %a, <1 x i32>*** %b, <1 x i32*>** %c) local_unnamed_addr {
  entry:
  ret i32 0
}


define spir_kernel void @test() {
entry:
  ret void
}

!genx.kernels = !{!0}

!0 = !{void ()* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0}
!1 = !{}
!2 = !{}
!3 = !{}
