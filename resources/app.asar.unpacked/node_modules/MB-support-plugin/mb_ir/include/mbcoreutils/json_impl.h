// Copyright 2013 MakerBot Industries

#ifndef INCLUDE_MBCOREUTILS_JSON_IMPL_H_
#define INCLUDE_MBCOREUTILS_JSON_IMPL_H_

#include <boost/algorithm/string/trim.hpp>

#include <cstdint>
#include <limits>
#include <string>

#include "jsoncpp/json/writer.h"

namespace Json {
inline TypeError::TypeError(const std::string &what)
    : std::runtime_error("TypeError: " + what) {
}

inline TypeError::TypeError(const std::string &what, const Json::Value &json)
    : std::runtime_error("TypeError: " + what + ": " + json.toStyledString()) {
}

inline KeyError::KeyError(const std::string &what)
    : std::runtime_error("KeyError: " + what) {
}

inline KeyError::KeyError(const std::string &what, const Json::Value &json)
    : std::runtime_error(
        "KeyError: " + what + ": " + json.toStyledString()) {
}

inline ParseError::ParseError(const std::string &what)
    : std::runtime_error("ParseError: " + what) {
}

inline Json::Value parse(const std::string &text) {
  Json::Reader reader;
  Json::Value root;
  if (reader.parse(text, root)) {
    return root;
  } else {
    throw ParseError(reader.getFormattedErrorMessages());
  }
}

inline std::string toUnstyledString(const Json::Value &json) {
  return boost::trim_copy(Json::FastWriter().write(json));
}

template<typename T>
T &objectMember(T &json, const std::string &key) {
  if (json.isObject()) {
    if (json.isMember(key)) {
      return json[key];
    } else {
      throw KeyError("member " + key + " not found", json);
    }
  } else {
    throw TypeError("expected a JSON object", json);
  }
}

template<>
inline std::string objectMemberAs<std::string>(
    const Json::Value &json,
    const std::string &key) {
  const auto member(objectMember(json, key));
  if (member.isString()) {
    return member.asString();
  } else {
    throw TypeError("member " + key + " not a string", json);
  }
}

template<>
inline bool objectMemberAs<bool>(
    const Json::Value &json,
    const std::string &key) {
  const auto member(objectMember(json, key));
  if (member.isBool()) {
    return member.asBool();
  } else {
    throw TypeError("member " + key + " not a bool", json);
  }
}

template<>
inline double objectMemberAs<double>(
    const Json::Value &json,
    const std::string &key) {
  const auto member(objectMember(json, key));
  if (member.isDouble()) {
    return member.asDouble();
  } else {
    throw TypeError("member " + key + " not a double", json);
  }
}

template<>
inline unsigned int objectMemberAs<unsigned int>(
    const Json::Value &json,
    const std::string &key) {
  const auto member(objectMember(json, key));
  if (member.isNumeric()) {
    return member.asUInt();
  } else {
    throw TypeError("member " + key + " not numeric", json);
  }
}

template<>
inline uint16_t objectMemberAs<uint16_t>(
    const Json::Value &json,
    const std::string &key) {
  const auto member(objectMember(json, key));
  if (member.isNumeric()) {
    const auto v(member.asUInt());
    #undef max
    if (v <= std::numeric_limits<uint16_t>::max()) {
      return static_cast<uint16_t>(v);
    } else {
      throw std::out_of_range("objectMemberAs<uint16_t>: range error");
    }
  } else {
    throw TypeError("member " + key + " not numeric", json);
  }
}

std::string trimStyle(const Json::Value &json) {
  return boost::trim_copy(json.toStyledString());
}
}

#endif  // INCLUDE_MBCOREUTILS_JSON_IMPL_H_
