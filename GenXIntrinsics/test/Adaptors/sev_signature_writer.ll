;=========================== begin_copyright_notice ============================
;
; Copyright (C) 2020-2021 Intel Corporation
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"),
; to deal in the Software without restriction, including without limitation
; the rights to use, copy, modify, merge, publish, distribute, sublicense,
; and/or sell copies of the Software, and to permit persons to whom the
; Software is furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice (including the next
; paragraph) shall be included in all copies or substantial portions of the
; Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
;
; SPDX-License-Identifier: MIT
;============================ end_copyright_notice =============================

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
