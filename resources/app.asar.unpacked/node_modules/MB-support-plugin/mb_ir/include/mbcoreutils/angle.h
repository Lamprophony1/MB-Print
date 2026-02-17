// Copyright 2014 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_ANGLE_H_
#define INCLUDE_MBCOREUTILS_ANGLE_H_

namespace MakerBot {

/// Stores an angle as a double
class Angle {
 public:
  typedef double ValueType;

  /// Initialize zero angle
  Angle()
      : m_radians(0.0) {
  }

  /// Pi constant
  static inline ValueType pi() {
    return 3.14159265358979323846;
  }

  /// Initialize from radians
  static inline Angle fromRadians(const ValueType radians) {
    return Angle(radians);
  }

  /// Initialize from degrees
  static inline Angle fromDegrees(const ValueType degrees) {
    return Angle(degrees * (pi() / 180.0));
  }

  /// Return new angle wrapped into the circle starting at minimumAngle
  inline Angle wrapped(const Angle minimumAngle) const {
    Angle angle(*this);

    const auto pi2(fromRadians(2.0 * pi()));

    while (angle < minimumAngle) {
      angle += pi2;
    }

    const auto maximumAngle(minimumAngle + pi2);

    while (maximumAngle < angle) {
      angle -= pi2;
    }

    return angle;
  }

  /// Return new angle clamped into the range [minimumAngle, maximumAngle]
  ///
  /// Precondition: minimumAngle must be less than or equal to
  /// maximumAngle.
  inline Angle clamped(
      const Angle minimumAngle,
      const Angle maximumAngle) const {
    if ((*this) < minimumAngle) {
      return minimumAngle;
    } else if ((*this) > maximumAngle) {
      return maximumAngle;
    } else {
      return *this;
    }
  }

  void setRadians(const ValueType radians) {
    m_radians = radians;
  }

  void setDegrees(const ValueType degrees) {
    m_radians = degrees * (pi() / 180.0);
  }

  /// Get angle value in radians
  inline ValueType inRadians() const {
    return m_radians;
  }

  /// Get angle value in degrees
  inline ValueType inDegrees() const {
    return m_radians * (180.0 / pi());
  }

  inline bool operator<(const Angle &other) const {
    return m_radians < other.m_radians;
  }

  inline bool operator>(const Angle &other) const {
    return m_radians > other.m_radians;
  }

  inline Angle operator+(const Angle &other) const {
    return Angle(m_radians + other.m_radians);
  }

  inline Angle operator-(const Angle &other) const {
    return Angle(m_radians - other.m_radians);
  }

  inline void operator+=(const Angle &other) {
    m_radians += other.m_radians;
  }

  inline void operator-=(const Angle &other) {
    m_radians -= other.m_radians;
  }

 private:
  /// Initialize with the given radians value
  explicit inline Angle(const ValueType radians)
      : m_radians(radians) {
  }

  /// Current value
  ValueType m_radians;
};
}

#endif  // INCLUDE_MBCOREUTILS_ANGLE_H_
