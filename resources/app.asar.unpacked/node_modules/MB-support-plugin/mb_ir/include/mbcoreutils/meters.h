// Copyright 2014 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_METERS_H_
#define INCLUDE_MBCOREUTILS_METERS_H_

#include <stdexcept>

namespace MakerBot {

/// Stores a measurement of length vaguely in the style of std::chrono
///
/// The internal storage is a double.
///
/// The purpose of this class is to avoid dealing in raw numbers
/// without corresponding units.
class Meters {
 public:
  typedef double ValueType;

  /// Initialize with zero length
  Meters()
      : m_meters(0.0) {
  }

  /// Create a new Meters object by converting from millimeters
  static Meters meters(const ValueType meters) {
    return Meters(meters);
  }

  /// Create a new Meters object by converting from millimeters
  static Meters millimeters(const ValueType millimeters) {
    return Meters(millimeters * 0.001);
  }

  /// Get the length in meters
  ValueType inMeters() const {
    return m_meters;
  }

  /// Convert the length to millimeters
  ValueType inMillimeters() const {
    return m_meters * 1000.0;
  }

  Meters &operator=(const Meters &other) {
    m_meters = other.m_meters;
    return *this;
  }

  Meters operator+(const Meters &other) const {
    return Meters(m_meters + other.m_meters);
  }

 private:
  /// Initialize with the given meters value
  explicit Meters(const ValueType meters)
      : m_meters(meters) {
    if (m_meters < 0.0) {
      throw std::out_of_range("Meters value cannot be negative");
    }
  }

  /// Current value
  double m_meters;
};
}

#endif  // INCLUDE_MBCOREUTILS_METERS_H_
