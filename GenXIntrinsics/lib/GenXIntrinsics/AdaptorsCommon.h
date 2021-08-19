/*========================== begin_copyright_notice ============================

Copyright (C) 2020-2021 Intel Corporation

SPDX-License-Identifier: MIT

============================= end_copyright_notice ===========================*/

// This file defines common constants for writer/reader spirv adaptors.

namespace llvm {

class Function;

namespace genx {

enum class ArgKind {
  General = 0,
  Sampler = 1,
  Surface = 2,
};

namespace ArgDesc {
static constexpr const char ReadOnly[] = "read_only";
static constexpr const char WriteOnly[] = "write_only";
static constexpr const char ReadWrite[] = "read_write";

static constexpr const char Buffer[] = "buffer_t";
static constexpr const char SVM[] = "svmptr_t";
static constexpr const char Sampler[] = "sampler_t";
static constexpr const char Image1d[] = "image1d_t";
static constexpr const char Image1dBuffer[] = "image1d_buffer_t";
static constexpr const char Image2d[] = "image2d_t";
static constexpr const char Image3d[] = "image3d_t";
} // namespace ArgDesc

// Separate kinds of SPIRV types.
// Each of these kinds has different representation
// in terms of arg kind and arg desc.
enum class SPIRVType {
  // Surfaces + corresponding desc.
  Buffer,
  Image1d,
  Image1dBuffer,
  Image2d,
  Image3d,
  // Sampler + sampler_t.
  Sampler,
  // General + smvptr_t.
  Pointer,
  // Other general types (no arg desc).
  Other,
  // Old-style decorated types or already SPIRV type.
  None,
};

// Access type used by surfaces.
enum class AccessType {
  ReadOnly,
  WriteOnly,
  ReadWrite,
};

struct SPIRVArgDesc {
  SPIRVType Ty;
  AccessType Acc = AccessType::ReadWrite;

  SPIRVArgDesc(SPIRVType T) : Ty(T) {}
  SPIRVArgDesc(SPIRVType T, AccessType A) : Ty(T), Acc(A) {}
};

namespace OCLTypes {
// Common type prefix for ocl types in llvm IR.
static constexpr const char TypePrefix[] = "opencl.";

// Main types.
// Currently used image types.
static constexpr const char Image[] = "image";
static constexpr const char Dim1d[] = "1d";
static constexpr const char Dim1dBuffer[] = "1d_buffer";
static constexpr const char Dim2d[] = "2d";
static constexpr const char Dim3d[] = "3d";
// Sampler type.
static constexpr const char Sampler[] = "sampler";
} // namespace OCLTypes

// These are not really standardized names.
// Just something for POC implementation.
namespace IntelTypes {
// Type prefix for custom types.
static constexpr const char TypePrefix[] = "intel.";

// Stateful buffer type.
static constexpr const char Buffer[] = "buffer";
} // namespace IntelTypes

namespace CommonTypes {
// Access qualifiers. Should come after image type.
static constexpr const char ReadOnly[] = "_ro";
static constexpr const char WriteOnly[] = "_wo";
static constexpr const char ReadWrite[] = "_rw";

// Common type suffix for ocl types in llvm IR.
static constexpr const char TypeSuffix[] = "_t";
} // namespace CommonTypes

namespace SPIRVParams {
static constexpr const char SPIRVMemoryModel[] = "spirv.MemoryModel";
static constexpr const char SPIRVSIMDSubgroupSize[] =
    "intel_reqd_sub_group_size";
static constexpr unsigned SPIRVMemoryModelSimple = 0;
static constexpr unsigned SPIRVMemoryModelOCL = 2;
static constexpr unsigned SPIRVAddressingModel32 = 1;
static constexpr unsigned SPIRVAddressingModel64 = 2;

// Has to correspond to spir address space encoding.
static constexpr unsigned SPIRVGlobalAS = 1;
static constexpr unsigned SPIRVConstantAS = 2;
} // namespace SPIRVParams

inline unsigned getOpaqueTypeAddressSpace(SPIRVType Ty) {
  switch (Ty) {
  case SPIRVType::Sampler:
    return SPIRVParams::SPIRVConstantAS;
  case SPIRVType::Buffer:
  case SPIRVType::Image1d:
  case SPIRVType::Image1dBuffer:
  case SPIRVType::Image2d:
  case SPIRVType::Image3d:
    return SPIRVParams::SPIRVGlobalAS;
  default:
    // Default to zero for other types.
    return 0;
  }
}

// Overrides specific attributes of function parameters.
//
// Function arguments of PointerType can have specific
// attributes like ByVal, ByRef, Preallocated, InAlloca
// that contain Pointee Type of that pointer as parameter.
// SPIRV Adaptor passes may change Pointee type, so we must
// explicitly change this type in corresponding attributes
// in order to construct valid llvm-IR.
//
// (see more here: https://llvm.org/docs/LangRef.html#parameter-attributes)
void legalizeParamAttributes(Function* F);

} // namespace genx
} // namespace llvm
