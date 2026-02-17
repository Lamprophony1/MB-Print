// Copyright 2014 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_KILOGRAMS_H_
#define INCLUDE_MBCOREUTILS_KILOGRAMS_H_

#include <stdexcept>

namespace MakerBot {

/// Stores a measurement of weight vaguely in the style of std::chrono
///
/// Kilograms is used as the base unit because that's what SI does.
///
/// The internal storage is a double.
///
/// The purpose of this class is to avoid dealing in raw numbers
/// without corresponding units.
class Kilograms {
 public:
  typedef double ValueType;

  /// Initialize with zero weight
  Kilograms()
      : m_kilograms(0.0) {
  }

  static Kilograms grams(const ValueType grams) {
    return Kilograms(grams * 0.001);
  }

  static Kilograms kilograms(const ValueType kilograms) {
    return Kilograms(kilograms);
  }

  static Kilograms pounds(const ValueType pounds) {
    return Kilograms(pounds * 0.453592);
  }

  /// Get the weight in grams
  ValueType inGrams() const {
    return m_kilograms * 1000;
  }

  /// Get the weight in kilograms
  ValueType inKilograms() const {
    return m_kilograms;
  }

  /// Convert the weight to pounds
  ValueType inPounds() const {
    return m_kilograms * 2.20462;
  }

  Kilograms &operator=(const Kilograms &other) {
    m_kilograms = other.m_kilograms;
    return *this;
  }

  Kilograms operator+(const Kilograms &other) const {
    return Kilograms(m_kilograms + other.m_kilograms);
  }

 private:
  /// Initialize with the given kilograms value
  explicit Kilograms(const ValueType kilograms)
      : m_kilograms(kilograms) {
    if (m_kilograms < 0.0) {
      throw std::out_of_range("Kilograms value cannot be negative");
    }
  }

  /// Current value
  double m_kilograms;
};
}

#endif  // INCLUDE_MBCOREUTILS_KILOGRAMS_H_
