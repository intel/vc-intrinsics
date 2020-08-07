; Test combined writer translation: kernel has both annotated explicit
; arguments and impicit arguments. Implicit arguments would not show
; in normal flow, though they appear in old cmc.

; RUN: opt -S -GenXSPIRVWriterAdaptor < %s | FileCheck %s

define void @test(i32 %in, i32 %out, <3 x i32> %__arg_llvm.genx.local.id) {
; CHECK-LABEL: @test(

; CHECK: %opencl.image2d_ro_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[IN:%[^,]+]],

; CHECK: %opencl.image2d_wo_t addrspace(1)*
; CHECK-NOT: "VCArgumentDesc"
; CHECK-NOT: "VCArgumentKind"
; CHECK: [[OUT:%[^,]+]],

; CHECK: <3 x i32>
; CHECK: "VCArgumentKind"="24"
; CHECK: [[LOCAL_ID:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    [[TMP0:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_ro_t(%opencl.image2d_ro_t addrspace(1)* [[IN]])
; CHECK-NEXT:    [[TMP1:%.*]] = call i32 @llvm.genx.address.convert.i32.p1opencl.image2d_wo_t(%opencl.image2d_wo_t addrspace(1)* [[OUT]])
; CHECK-NEXT:    [[TMP2:%.*]] = extractelement <3 x i32> [[LOCAL_ID]], i32 0
; CHECK-NEXT:    [[CALL1_I_I_I:%.*]] = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 [[TMP0]], i32 0, i32 32, i32 [[TMP2]], i32 0)
; CHECK-NEXT:    tail call void @llvm.genx.media.st.v8i32(i32 0, i32 [[TMP1]], i32 0, i32 32, i32 [[TMP2]], i32 0, <8 x i32> [[CALL1_I_I_I]])
; CHECK-NEXT:    ret void
;
entry:
  %0 = extractelement <3 x i32> %__arg_llvm.genx.local.id, i32 0
  %call1.i.i.i = tail call <8 x i32> @llvm.genx.media.ld.v8i32(i32 0, i32 %in, i32 0, i32 32, i32 %0, i32 0)
  tail call void @llvm.genx.media.st.v8i32(i32 0, i32 %out, i32 0, i32 32, i32 %0, i32 0, <8 x i32> %call1.i.i.i)
  ret void
}

declare <8 x i32> @llvm.genx.media.ld.v8i32(i32, i32, i32, i32, i32, i32)
declare void @llvm.genx.media.st.v8i32(i32, i32, i32, i32, i32, i32, <8 x i32>)

; CHECK-NOT: !genx.kernels
!genx.kernels = !{!0}

!0 = !{void (i32, i32, <3 x i32>)* @test, !"test", !1, i32 0, i32 0, !2, !3, i32 0, i32 0}
!1 = !{i32 2, i32 2, i32 24}
!2 = !{i32 0, i32 0}
!3 = !{!"image2d_t read_only", !"image2d_t write_only"}
