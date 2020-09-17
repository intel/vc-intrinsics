; XFAIL: llvm7
; RUN: opt -S -cmsimdcflowering < %s | FileCheck %s

@Rcp_T2 = internal global <64 x double> undef

; CHECK: @EM = internal global <32 x i1>

define dso_local dllexport void @test1(<32 x i16> %mask, <64 x i32> %oldval) {
entry:
  %Rcp_T = alloca <64 x double>, align 512
  %0 = icmp ne <32 x i16> %mask, zeroinitializer
  %call = call i1 @llvm.genx.simdcf.any.v32i1(<32 x i1> %0)
  br i1 %call, label %if.then, label %if.end

if.then:
; CHECK:        if.then:
; CHECK-NEXT:     [[EM_LOAD:%.*]] = load <32 x i1>, <32 x i1>* @EM
; CHECK-NEXT:     [[PRED_WRR:%.*]] = call <64 x i32> @llvm.genx.wrregioni.v64i32.v32i32.i16.v32i1(<64 x i32> %oldval, <32 x i32> zeroinitializer, i32 0, i32 32, i32 2, i16 0, i32 undef, <32 x i1> [[EM_LOAD]])
; CHECK-NEXT:     [[PRED_WRR_CAST:%.*]] = bitcast <64 x i32> [[PRED_WRR]] to <32 x double>
  %wrregion26 = call <64 x i32> @llvm.genx.wrregioni.v64i32.v32i32.i16.i1(<64 x i32> %oldval, <32 x i32> zeroinitializer, i32 0, i32 32, i32 2, i16 0, i32 undef, i1 true)
  %cast27 = bitcast <64 x i32> %wrregion26 to <32 x double>

  %Rcp_T2_load = load <64 x double>, <64 x double>* %Rcp_T
  %wrregion28 = call <64 x double> @llvm.genx.wrregionf.v64f64.v32f64.i16.i1(<64 x double> %Rcp_T2_load, <32 x double> %cast27, i32 0, i32 32, i32 1, i16 0, i32 32, i1 true)
  store <64 x double> %wrregion28, <64 x double>* %Rcp_T
  br label %if.end

if.end:
  %1 = load <64 x double>, <64 x double>* %Rcp_T
  store <64 x double> %1, <64 x double>* @Rcp_T2
  ret void
}

declare i1 @llvm.genx.simdcf.any.v32i1(<32 x i1>)
declare <64 x i32> @llvm.genx.wrregioni.v64i32.v32i32.i16.i1(<64 x i32>, <32 x i32>, i32, i32, i32, i16, i32, i1)
declare <64 x double> @llvm.genx.wrregionf.v64f64.v32f64.i16.i1(<64 x double>, <32 x double>, i32, i32, i32, i16, i32, i1)
