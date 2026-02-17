// Copyright 2013 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_VECTOR_VERSION_H_
#define INCLUDE_MBCOREUTILS_VECTOR_VERSION_H_

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#include "jsoncpp/json/value.h"

#ifdef _MSC_VER
// Fucking Microsoft
#undef min
#undef max
#endif

namespace MakerBot {

/// Generic version of arbitrary length containing unsigned integers
///
/// The version is ordered from most significant to least significant.
class VectorVersion {
 public:
  typedef unsigned Element;

  /// Create an empty version
  VectorVersion() {
  }

  /// Create a one-element version
  explicit VectorVersion(const Element e0) {
    append(e0);
  }

  /// Create a two-element version
  VectorVersion(const Element e0, const Element e1) {
    append(e0);
    append(e1);
  }

  /// Create a three-element version
  VectorVersion(const Element e0, const Element e1, const Element e2) {
    append(e0);
    append(e1);
    append(e2);
  }

  /// Create a four-element version
  VectorVersion(
      const Element e0,
      const Element e1,
      const Element e2,
      const Element e3) {
    append(e0);
    append(e1);
    append(e2);
    append(e3);
  }

  /// Create a version from a Json::Value
  ///
  /// The input must be an array containing zero or more unsigned
  /// integers, otherwise an exception is thrown.
  explicit VectorVersion(const Json::Value &json) {
    if (json.isArray()) {
      for (const auto &elem : json) {
        // Note: using isInt instead of isUInt to work around a bug in
        // older versions of json-cpp (BW-767)
        if (elem.isInt() && (elem.asInt() >= 0)) {
          append(elem.asInt());
        } else if (elem.isString()) {
            // We shouldn't crash on versions that have straings in them, since
            // that IS allowed by semver; but this is a bit of a last second fix
            // and vector_version is used EVERYWHERE, so we're not gonna mess
            // with its semantics by parsing those strings and using them in
            // equality checking or stringification, either.
        } else if (elem.isNull()) {
            // Same here
        } else {
          throw std::runtime_error(
              "VectorVersion: JSON input array contains invalid type: " +
              json.toStyledString());
        }
      }
    } else {
      throw std::runtime_error(
          "VectorVersion: JSON input not an array: " +
          json.toStyledString());
    }
  }

  /// Create a version from a string
  explicit VectorVersion(const std::string &str) {
    std::stringstream sstr(str);
    std::string item;
    try {
      while (std::getline(sstr, item, '.')) {
        append(std::stoi(item));
      }
    } catch (...) {
      throw std::runtime_error("VectorVersion: Bad version string: " + str);
    }
  }

  /// Return true if the version is empty, false otherwise
  bool empty() const {
    return m_vec.empty();
  }

  /// Get the number of elements in the version
  std::vector<Element>::size_type size() const {
    return m_vec.size();
  }

  /// Get the specified element (throws if out of bounds)
  Element operator[](const std::vector<Element>::size_type index) {
    return m_vec.at(index);
  }

  /// Append an element to the end of the version
  void append(const Element e) {
    m_vec.emplace_back(e);
  }

  /// Return a period-separated version string
  std::string str() const {
    std::string result;
    for (const auto &v : m_vec) {
      if (!result.empty()) {
        result += '.';
      }
      result += std::to_string(v);
    }
    return result;
  }

  /// Return a Json cpp array of the version
  Json::Value array() const {
    Json::Value result;
    for (const auto &v : m_vec) {
      result.append(v);
    }
    return result;
  }

  bool operator==(const VectorVersion &other) const {
    return cmp(other) == 0;
  }

  bool operator!=(const VectorVersion &other) const {
    return cmp(other) != 0;
  }

  bool operator<(const VectorVersion &other) const {
    return cmp(other) < 0;
  }

  bool operator<=(const VectorVersion &other) const {
    return cmp(other) <= 0;
  }

  bool operator>(const VectorVersion &other) const {
    return cmp(other) > 0;
  }

  bool operator>=(const VectorVersion &other) const {
    return cmp(other) >= 0;
  }

  /// Return -1 if this is less than other, 0 if equal, otherwise 1
  ///
  /// Note: a.b < a.b.c
  int cmp(const VectorVersion &other) const {
    const auto end(std::min(m_vec.size(), other.m_vec.size()));
    for (std::size_t i(0); i < end; ++i) {
      if (m_vec[i] < other.m_vec[i]) {
        return -1;
      } else if (m_vec[i] > other.m_vec[i]) {
        return 1;
      }
    }

    if (m_vec.size() < other.m_vec.size()) {
      return -1;
    } else if (m_vec.size() > other.m_vec.size()) {
      return 1;
    } else {
      return 0;
    }
  }

 private:
  std::vector<Element> m_vec;
};
}

#endif  // INCLUDE_MBCOREUTILS_VECTOR_VERSION_H_
