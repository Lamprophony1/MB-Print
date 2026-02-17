// Copyright MakerBot Inc, 2017

/*
 * jsoncpp_wrappers.h: functionality that jsoncpp should have provided
 * This header file contains wrappers that make it much easier to safely
 * (i.e. without crashing or causing exceptions) extract values from a
 * Json::Value tree, even in the face of missing keys, keys with different
 * and incompatible types, and various trees of same.
 */

#ifndef INCLUDE_MBCOREUTILS_JSONCPP_WRAPPERS_H_
#define INCLUDE_MBCOREUTILS_JSONCPP_WRAPPERS_H_

#include <jsoncpp/json/json.h>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <utility>

namespace MakerBot {

namespace SafeJson {

/** @brief Typesafe wrapper for Json::Value
 *
 * This wrapper provides a compact, straightforward and safe way to populate
 * static typed settings variables from a dynamically typed Json::Value.  The
 * wrapper provides analogous syntax to JsonCpp itself for accessing non-leaf
 * nodes, but requires all leaf nodes to be accessed via an assignTo() method.
 * Any deviations of the json structure from the structure implicitly defined
 * by accessing fields with an assignTo() call will result in a silent failure
 * of the call, but in this case the wrapper will also track a detailed error
 * or warning message that can be retrieved to provide messaging and an
 * overall determination of whether parsing was successful.  Currying behavior
 * modifiers can be used to specify how to deal with missing values, but
 * currently all other failure modes are hard coded as errors.
 */
class Value {
  public:
    Value() {}
    explicit Value(const Json::Value & v)
          : v_(v), err_(new ErrorInfo), cxt_("root") {}

    /** @brief Accessor for fields of an object
     *
     * This can be safely applied to a wrapper of any value, but will return
     * a wrapper with no value unless the wrapped value is an object containing
     * the given key.
     */
    Value operator[](const std::string & key) {
        if (!v_.isObject()) {
            TypeError("object");
            return Value();
        } else if (v_.isMember(key)) {
            return Value(v_[key], err_, missing_action_,
                         cxt_ + "[\"" + key + "\"]");
        } else {
            KeyError(key);
            return Value();
        }
    }

    /** @brief Accessor for entries of an array
     *
     * This can be safely applied to a wrapper of any value, but will return
     * a wrapper with no value unless the wrapped value is an array containing
     * the given index.
     */
    Value operator[](int index) {
        if (!v_.isArray()) {
            TypeError("object");
            return Value();
        } else {
            // This could probably use index range checking but we
            // don't really use this currently except via assignTo().
            return Value(v_[index], err_, missing_action_,
                         cxt_ + "[" + std::to_string(index) + "]");
        }
    }

    /** @brief Return whether the wrapper contains an actual value
     *
     * When accessing fields of an object that we allow to be missing, there
     * isn't really a way to tell the difference between the field not being
     * present at all and the field being present but containing the default
     * value.  In most cases this doesn't really matter, but if we specifically
     * need to tell the difference between these cases, this method will do
     * that.
     */
    bool exists() {
        // We have an ErrorInfo object iff we have a real value
        return static_cast<bool>(err_);
    }

    /** @brief Assign the wrapped value to a given value
     *
     * All assignTo overloads will only assign the wrapped value to the given
     * variable if the variable type is compatible with the wrapped value.
     * Overloads are provided for a usefully large selection of built in types.
     */
    // We need to actually use the fundamental C++ types here, because we need
    // to cover any integer types that we use in any codebase using this, but
    // we also can't duplicate any types here or we would create ambiguous
    // overloads (this has to support all compilers that we use which may use
    // different mappings of type aliases to these fundamental types.
    void assignTo(char * v) { assignToInt(v); }
    void assignTo(signed char * v) { assignToInt(v); }
    void assignTo(unsigned char * v) { assignToInt(v); }
    void assignTo(short * v) { assignToInt(v); }  // NOLINT
    void assignTo(unsigned short * v) { assignToInt(v); }  // NOLINT
    void assignTo(int * v) { assignToInt(v); }
    void assignTo(unsigned int * v) { assignToInt(v); }
    void assignTo(long * v) { assignToInt(v); }  // NOLINT
    void assignTo(unsigned long * v) { assignToInt(v); }  // NOLINT
    void assignTo(long long * v) { assignToInt(v); }  // NOLINT
    void assignTo(unsigned long long * v) { assignToInt(v); }  // NOLINT

    void assignTo(float * v) {
        if (!v_.isNumeric()) {
            TypeError("numeric");
            return;
        }
        *v = v_.asFloat();
    }

    void assignTo(double * v) {
        if (!v_.isNumeric()) {
            TypeError("numeric");
            return;
        }
        *v = v_.asDouble();
    }

    void assignTo(bool * v) {
        if (!v_.isBool()) {
            TypeError("boolean");
            return;
        }
        *v = v_.asBool();
    }

    void assignTo(std::string * v) {
        if (!v_.isString()) {
            TypeError("string");
            return;
        }
        *v = v_.asString();
    }

    template<typename T>
    void assignTo(std::vector<T> * v) {
        if (!v_.isArray()) {
            TypeError("array");
            return;
        }
        size_t size = v_.size();
        v->resize(size);
        for (size_t i = 0; i < size; ++i) {
            (*this)[i].assignTo(&(*v)[i]);
        }
    }

    template<typename T1, typename T2>
    void assignTo(std::pair<T1, T2> * v) {
        if (!v_.isArray()) {
            TypeError("array");
            return;
        }
        if (v_.size() != 2) {
            Error(" must contain 2 elements");
            return;
        }
        (*this)[0].assignTo(&v->first);
        (*this)[1].assignTo(&v->second);
    }

    /** @brief Currying operator to ignore missing values
     *
     * Return a wrapper identical except for the fact that it will not
     * generate any errors or warnings for missing object fields.
     */
    Value missingOk() {
        return Value(v_, err_, MissingOk, cxt_);
    }

    /** @brief Currying operator to warn on missing values
     *
     * Return a wrapper identical except for the fact that it will only
     * warn about missing object fields.  No warnings will be generated
     * if the wrapper was already completely ignoring missing fields.
     */
    Value missingWarn() {
        if (missing_action_ != MissingError) return *this;
        return Value(v_, err_, MissingWarn, cxt_);
    }

    /** @brief Retrieve and clear all error messages
     *
     * This returns a unique error message for each issue found when parsing
     * this wrapper, a parent of this wrapper, or a child of a parent of this
     * wrapper.  The count of errors is not necessarily related to the number
     * of assignments that did not occur, but a count of zero errors can be
     * used as an indicator that everything was parsed sucessfully.
     */
    std::vector<std::string> getErrors() {
        return std::move(err_->errors);
    }

    /** @brief Retrieve and clear all warning messages
     *
     * Same as getErrors but for warnings
     */
    std::vector<std::string> getWarnings() {
        return std::move(err_->warnings);
    }

  private:
    struct ErrorInfo {
        std::vector<std::string> errors, warnings;
    };
    enum MissingAction {
        MissingError,
        MissingWarn,
        MissingOk
    };

    Value(const Json::Value & v, std::shared_ptr<ErrorInfo> err,
          MissingAction missing_action, const std::string & cxt)
          : v_(v), err_(err), missing_action_(missing_action), cxt_(cxt) {
    }

    template <typename IntT>
    void assignToInt(IntT * v) {
        if (!v_.isIntegral()) {
            TypeError("integral");
            return;
        }
        auto min = std::numeric_limits<IntT>::min();
        auto max = std::numeric_limits<IntT>::max();
        if (min < 0) {
            auto val = v_.asLargestInt();
            if (val < min || val > max) {
                return;
            }
            *v = val;
        } else {
            auto val = v_.asLargestUInt();
            if (val > max) {
                return;
            }
            *v = val;
        }
    }


    void TypeError(const char * reqd_type) {
        if (!err_) return;
        err_->errors.push_back(cxt_ + " must be " + reqd_type + " type");
    }

    void KeyError(const std::string & key) {
        if (!err_) return;
        if (missing_action_ == MissingOk) return;
        auto err = cxt_ + " missing key \"" + key + "\"";
        if (missing_action_ == MissingError) {
            err_->errors.push_back(err);
        } else {
            err_->warnings.push_back(err);
        }
    }

    void RangeError() {
        Error(" out of range");
    }

    void Error(const char * msg) {
        if (!err_) return;
        err_->errors.push_back(cxt_ + msg);
    }

    Json::Value v_;
    std::shared_ptr<ErrorInfo> err_;
    MissingAction missing_action_ = MissingError;
    std::string cxt_;
};

/**
 * type_correct - check if a value is present in the right type
 * Returns true if the key contains a value, and that value
 * is the same type as type.
 * This is probably not a useful function for library consumers to call.
 */
// This is the base templated type, we should fall back to it for string
// types

template<typename KeyType>
inline bool type_correct(const Json::Value& val,
                         const KeyType& key,
                         Json::ValueType type) {
    return val.isObject()
        && val.isMember(key)
        && !val[key].isNull()
        && val[key].isConvertibleTo(type);
}

// Overloads for Json::ArrayIndex and int that check if val is an array
template<>
inline bool type_correct<Json::ArrayIndex>(const Json::Value& val,
                                           const Json::ArrayIndex& idx,
                                           Json::ValueType type) {
    return val.isArray()
        && val.isValidIndex(idx)
        && !val[idx].isNull()
        && val[idx].isConvertibleTo(type);
}

template<>
inline bool type_correct<int>(const Json::Value& val,
                              const int& idx,
                              Json::ValueType type) {
    return type_correct(val, Json::ArrayIndex(idx), type);
}


/**
 * get_leaf (and overloads) - get C++ typed versions of leaf values
 * The first two arguments are always the same, the value and the key.
 * The key is templated so you can use a string, char array, or array index.
 * The third argument is both a default value and an overload resolution.
 */
template<typename KeyType>
float get_leaf(const Json::Value& val,
               const KeyType& key,
               float default_val) {
    if (type_correct(val, key, Json::realValue)) {
        return val[key].asFloat();
    } else {
        return default_val;
    }
}

template<typename KeyType>
unsigned int get_leaf(const Json::Value& val,
                      const KeyType& key,
                      unsigned int default_val) {
    if (type_correct(val, key, Json::uintValue)) {
        return val[key].asUInt();
    } else {
        return default_val;
    }
}

template<typename KeyType>
int get_leaf(const Json::Value& val,
             const KeyType& key,
             int default_val) {
    if (type_correct(val, key, Json::intValue)) {
        return val[key].asInt();
    } else {
        return default_val;
    }
}

template<typename KeyType>
std::string get_leaf(const Json::Value& val,
                     const KeyType& key,
                     const std::string& default_val) {
    if (type_correct(val, key, Json::stringValue)) {
        return val[key].asString();
    } else {
        return default_val;
    }
}

template<typename KeyType>
std::string get_leaf(const Json::Value& val,
                     const KeyType& key,
                     const char* default_val) {
    return get_leaf(val, key, std::string(default_val));
}

template<typename KeyType>
bool get_leaf(const Json::Value& val,
              const KeyType& key,
              const bool& default_val) {
    if (type_correct(val, key, Json::booleanValue)) {
        return val[key].asBool();
    } else {
        return default_val;
    }
}

template<typename KeyType>
Json::Value get_obj(const Json::Value& val,
                    const KeyType& key) {
    if (type_correct(val, key, Json::objectValue)) {
        return val[key];
    } else {
        return Json::Value(Json::objectValue);
    }
}

template<typename KeyType>
Json::Value get_arr(const Json::Value& val,
                    const KeyType& key) {
    if (type_correct(val, key, Json::arrayValue)) {
        return val[key];
    } else {
        return Json::Value(Json::arrayValue);
    }
}

};  // namespace SafeJson

};  // namespace MakerBot

#endif  // INCLUDE_MBCOREUTILS_JSONCPP_WRAPPERS_H_
