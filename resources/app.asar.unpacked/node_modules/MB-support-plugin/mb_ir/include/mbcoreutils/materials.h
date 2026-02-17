// Copyright 2013 Makerbot Industries

#ifndef INCLUDE_MBCOREUTILS_MATERIALS_H_
#define INCLUDE_MBCOREUTILS_MATERIALS_H_

#include <string>

namespace MakerBot {
enum Material {
  /// Acrylonitrile butadiene styrene AKA MakerBot ABS
  kMaterialABS = 0,

  /// Polylactic acid AKA MakerBot PLA
  kMaterialPLA,

  /// High Impact Polystyrene AKA MakerBot Support Material
  kMaterialHIPS,

  // Polycaprolactone AKA MakerBot Flexible Filament
  kMaterialPCL,

  /// Invalid material type
  kMaterialUnknown
};


/// Return the internal material name as a std string
/// This is intened for communication with conveyor and writing to files.  It's
/// inverse is materialFromString().
///
/// If the material type is invalid, the stirng "materialUnknown" is reutnred
/// rather than throwing an exception.

inline std::string stringFromMaterial(const Material material) {
  switch (material) {
    case kMaterialABS:
      return "ABS";
    case kMaterialPLA:
      return "PLA";
    case kMaterialHIPS:
      return "HIPS";
    case kMaterialPCL:
      return "PCL";
    case kMaterialUnknown:
      break;
  }

  return "materialUnknown";
}

/// Get a material type from a string
///
/// This is intended for communication with conveyor and writing to files. It's
/// inverse is stringFromMaterial().
///
/// If the material string is invalid, kMaterialUnknown is returned rather than
/// throwing an exeption.
inline Material stringToMaterial(const std::string &identifier) {
  if (identifier == stringFromMaterial(kMaterialABS)) {
    return kMaterialABS;
  } else if (identifier == stringFromMaterial(kMaterialPLA)) {
    return kMaterialPLA;
  } else if (identifier == stringFromMaterial(kMaterialHIPS)) {
    return kMaterialHIPS;
  } else if (identifier == stringFromMaterial(kMaterialPCL)) {
    return kMaterialPCL;
  } else {
    return kMaterialUnknown;
  }
}
}  // namespace MakerBot

#endif  // INCLUDE_MBCOREUTILS_MATERIALS_H_
