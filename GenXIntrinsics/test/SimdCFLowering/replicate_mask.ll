; RUN: opt -S -cmsimdcflowering < %s | FileCheck %s

@Rcp_T2 = internal global <64 x double> undef

; CHECK: @EM = internal global <32 x i1> 

define internal <64 x double> @_Z24func_replicate_mask_attrv() #0 {
entry:
  ret <64 x double> zeroinitializer
}

define dso_local dllexport void @test(<32 x i16> %mask) {
entry:
  %Rcp_T = alloca <64 x double>, align 512
  %0 = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %0)
  br i1 %call, label %if.then, label %if.end

if.then:
; CHECK:        [[CALL1:%.*]] = call <64 x double> @_Z24func_replicate_mask_attrv()
  %call1 = call <64 x double> @_Z24func_replicate_mask_attrv()

; CHECK:         [[EM_LOAD:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT:    [[CHENNELEM:%.*]] = shufflevector <32 x i1> [[EM_LOAD]], <32 x i1> undef, <64 x i32> <i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31, i32 0, i32 1, i32 2, i32 3, i32 4, i32 5, i32 6, i32 7, i32 8, i32 9, i32 10, i32 11, i32 12, i32 13, i32 14, i32 15, i32 16, i32 17, i32 18, i32 19, i32 20, i32 21, i32 22, i32 23, i32 24, i32 25, i32 26, i32 27, i32 28, i32 29, i32 30, i32 31>
; CHECK-NEXT:    [[CALL1_SIMDCFPREDL:%.*]] = select <64 x i1> [[CHENNELEM]], <64 x double> [[CALL1]]
  store <64 x double> %call1, <64 x double>* %Rcp_T
  br label %if.end

if.end:
  %1 = load <64 x double>, <64 x double>* %Rcp_T
  store <64 x double> %1, <64 x double>* @Rcp_T2
  ret void
}

declare i1 @llvm.genx.simdcf.any.v32i1(<32 x i1>)

attributes #0 = { noinline nounwind "CMGenxReplicateMask"="2"}
