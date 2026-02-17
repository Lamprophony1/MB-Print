// Copyright 2013 Makerbot Industries

#ifndef INCLUDE_MBCOREUTILS_VERSION_H_
#define INCLUDE_MBCOREUTILS_VERSION_H_

#include <stdlib.h>

#include <string>

namespace MakerBot {
class Version {
  public:
    // Default constructor produces a version of 0.0.0.0
    Version()
      : m_major(0),
        m_minor(0),
        m_point(0),
        m_build(0) {
    }

    // Copy constructor
    Version(const Version & other)
      : m_major(other.m_major),
        m_minor(other.m_minor),
        m_point(other.m_point),
        m_build(other.m_build) {
    }

    Version(int major, int minor, int point, int build)
      : m_major(major),
        m_minor(minor),
        m_point(point),
        m_build(build)  {
    }

    // Parses a version string in the format #.#.#.#
    explicit Version(const std::string str) {
      size_t found = str.find_last_of("/.");
      m_build = atoi(str.substr(found + 1).c_str());
      std::string new_str = str.substr(0, found);

      found = new_str.find_last_of(".");
      m_point = atoi(new_str.substr(found + 1).c_str());
      new_str = new_str.substr(0, found);

      found = new_str.find_last_of(".");
      m_minor = atoi(new_str.substr(found + 1).c_str());
      new_str = new_str.substr(0, found);

      found = new_str.find_last_of(".");
      m_major = atoi(new_str.substr(found + 1).c_str());
    }

    bool operator<(Version const & other) const {
      return (compare(other) < 0);
    }
    bool operator>(Version const & other) const {
      return (compare(other) > 0);
    }
    bool operator==(Version const & other) const {
      return (compare(other) == 0);
    }
    bool operator!=(Version const & other) const {
      return (compare(other) != 0);
    }

    // compare this to another version.
    // If this is less than other returns a negative number, greater than
    //   returns a positive number, and equal to returns 0
    int compare(const Version & other) const {
      const int major = m_major - other.m_major;
      if (0 != major)
        return major;

      const int minor = m_minor - other.m_minor;
      if (0 != minor)
        return minor;

      const int point = m_point - other.m_point;
      if (0 != point)
        return point;

      const int build = m_build - other.m_build;
      return build;
    }

    int major() const {
      return m_major;
    }

    int minor() const {
      return m_minor;
    }

    int point() const {
      return m_point;
    }

    int build() const {
      return m_build;
    }

    std::string toString() const {
      const std::string sep(".");

      return std::to_string(m_major) + sep +
             std::to_string(m_minor) + sep +
             std::to_string(m_point) + sep +
             std::to_string(m_build);
    }

    std::string toStringNoBuildNumber() const {
      const std::string sep(".");

      return std::to_string(m_major) + sep +
             std::to_string(m_minor) + sep +
             std::to_string(m_point);
    }

  private:
    int m_major;
    int m_minor;
    int m_point;
    int m_build;
};
}  // MakerBot

#endif  // INCLUDE_MBCOREUTILS_VERSION_H_
