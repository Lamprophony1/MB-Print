// Copyright 2013 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_TAGGED_ID_H_
#define INCLUDE_MBCOREUTILS_TAGGED_ID_H_

#include <stdexcept>

namespace MakerBot {

/// Error thrown when attempting to use an invalid ID
class IDError : public std::runtime_error {
 public:
  IDError()
      : std::runtime_error("Invalid ID") {
  }
};

/// Do-nothing argument for third parameter of CREATE_TAGGED_ID_TYPE
///
/// Use this for IDs that don't need to be marked for export.
#define TAGGED_ID_NO_EXPORT

/// Create a new ID type
///
/// This is a generic ID for ID types that can represent both valid
/// and invalid values (in other words, at least one value of
/// ValueType must be able to represent an invalid ID).
///
/// The new type will be named name_ and have a completely separate
/// type from any other tagged ID. This is implemented as a macro
/// rather than a templated class so that forward declarations are
/// easy.
///
/// The id_policy_ type must provide three things:
///
/// - A ValueType typedef that will be used as the underlying storage
///   for the ID.
///
/// - A static method invalidValue() that returns a ValueType
///   representing an invalid ID.
///
/// - A static method valid() that a ValueType parameter and returns
///   true if the parameter is valid, false otherwise.
///
/// The default constructor creates an invalid ID, the one-argument
/// constructor produces a valid ID (an IDError is thrown if the
/// argument is not a valid ID.)
///
/// The valid() method returns true if the current ID is valid, false
/// otherwise. The static overload of valid() passes through to the
/// static valid() method of the policy type.
///
/// The get() method returns the current ID if valid, otherwise an
/// IDError is thrown.
///
/// Equality and less-than operators are provided. Invalid IDs are
/// considered equal to eachother. An invalid ID is always less-than a
/// valid ID.
///
/// Example:
///
///   If a valid ID is a number between 0 and 127 (inclusive), a
///   tagged ID could be created using a ValueType of signed
///   char. ValidatorType::invalidValue() could return -1 (or any
///   negative number down to -128.)
///
///   Code:
///
///   struct MyIDPolicy {
///     typedef signed char ValueType;
///     static ValueType invalidValue() { return -1; }
///     static bool valid(const ValueType value) { return value >= 0; }
///   };
///
///   CREATE_TAGGED_ID_TYPE(MyID, MyIDPolicy);
#define CREATE_TAGGED_ID_TYPE(name_, policy_, api_) \
  class api_ name_ { \
   public: \
    typedef policy_ PolicyType; \
    typedef PolicyType::ValueType ValueType; \
    \
    name_() \
        : m_id(PolicyType::invalidValue()) { \
    } \
    \
    explicit name_(const ValueType &id) \
        : m_id(id) { \
      if (!PolicyType::valid(id)) { \
        throw MakerBot::IDError(); \
      } \
    } \
    \
    bool valid() const { \
      return PolicyType::valid(m_id); \
    } \
    \
    static bool valid(const ValueType &id) { \
      return PolicyType::valid(id); \
    } \
    \
    ValueType get() const { \
      if (valid()) { \
        return m_id; \
      } else { \
        throw MakerBot::IDError(); \
      } \
    } \
    \
    bool operator==(const name_ &other) const { \
      const bool meValid(valid()); \
      const bool otherValid(other.valid()); \
      \
      if (meValid && otherValid) { \
        return get() == other.get(); \
      } else if (!meValid && !otherValid) { \
        return true; \
      } else { \
        return false; \
      } \
    } \
    \
    bool operator!=(const name_ &other) const { \
      return !((*this) == other); \
    } \
    \
    bool operator<(const name_ &other) const { \
      const bool meValid(valid()); \
      const bool otherValid(other.valid()); \
      \
      if (meValid && otherValid) { \
        return get() < other.get(); \
      } else if (!meValid && otherValid) { \
        return true; \
      } else if (meValid && !otherValid) { \
        return false; \
      } else { \
        return false; \
      } \
    } \
    \
   private: \
    ValueType m_id; \
  }
}

#endif  // INCLUDE_MBCOREUTILS_TAGGED_ID_H_
