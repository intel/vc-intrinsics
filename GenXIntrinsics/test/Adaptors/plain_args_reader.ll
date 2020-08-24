; Test kernel argument translation from new style with opaque types
; that SPIRV translator can understand to old style with
; metadata. Arguments without annotations are used here (CMRT like).

; RUN: opt -S -GenXSPIRVReaderAdaptor < %s | FileCheck %s

%intel.buffer_rw_t = type opaque
%opencl.sampler_t = type opaque

define spir_kernel void @test(%intel.buffer_rw_t addrspace(1)* %surf, %opencl.sampler_t addrspace(2)* %samp, i64 %ptr, i32 %gen) #0 {
; CHECK-LABEL: @test(

; CHECK: i32
; CHECK: [[SURF:%[^,]+]],

; CHECK: i32
; CHECK: [[SAMP:%[^,]+]],

; CHECK: i64
; CHECK: [[PTR:%[^,]+]],

; CHECK: i32
; CHECK: [[GEN:%[^)]+]])

; CHECK-NEXT:  entry:
; CHECK-NEXT:    ret void
;
entry:
  %0 = call i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)* %surf)
  %1 = call i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)* %samp)
  ret void
}

declare i32 @llvm.genx.address.convert.i32.p1intel.buffer_rw_t(%intel.buffer_rw_t addrspace(1)*)
declare i32 @llvm.genx.address.convert.i32.p2opencl.sampler_t(%opencl.sampler_t addrspace(2)*)

attributes #0 = { "VCFunction" }

; CHECK: !genx.kernels = !{[[KERNEL:![0-9]+]]}
; CHECK: [[KERNEL]] = !{void (i32, i32, i64, i32)* @test, !"test", ![[KINDS:[0-9]+]], i32 0, i32 0, !{{[0-9]+}}, ![[DESCS:[0-9]+]], i32 0}
; CHECK-DAG: ![[KINDS]] = !{i32 2, i32 1, i32 0, i32 0}
; CHECK-DAG: ![[DESCS]] = !{!"buffer_t read_write", !"sampler_t", !"", !""}
