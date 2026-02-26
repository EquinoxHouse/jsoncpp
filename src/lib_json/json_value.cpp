// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

#if !defined(JSONCPP_IS_AMALGAMATION)
#include <json/value.h>
#include <json/writer.h>
#ifndef JSONCPP_USE_SIMPLE_INTERNAL_ALLOCATOR
#include "json_batchallocator.h"
#endif // #ifndef JSONCPP_USE_SIMPLE_INTERNAL_ALLOCATOR
#endif // if !defined(JSONCPP_IS_AMALGAMATION)
#include <cassert>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>

#include <cmath>   // std::nextafter
#include <cstddef> // size_t

namespace Json {

    const Value Value::null;
    const Int Value::minInt = Int(~(UInt(-1) / 2));
    const Int Value::maxInt = Int(UInt(-1) / 2);
    const UInt Value::maxUInt = UInt(-1);
    const Int64 Value::minInt64 = Int64(~(UInt64(-1) / 2));
    const Int64 Value::maxInt64 = Int64(UInt64(-1) / 2);
    const UInt64 Value::maxUInt64 = UInt64(-1);
    const LargestInt Value::minLargestInt = LargestInt(~(LargestUInt(-1) / 2));
    const LargestInt Value::maxLargestInt = LargestInt(LargestUInt(-1) / 2);
    const LargestUInt Value::maxLargestUInt = LargestUInt(-1);

    /// Unknown size marker
    static const unsigned int unknown = (unsigned)-1;

    /** Duplicates the specified string value.
     * @param value Pointer to the string to duplicate. Must be zero-terminated if
     *              length is "unknown".
     * @param length Length of the value. if equals to unknown, then it will be
     *               computed using strlen(value).
     * @return Pointer on the duplicate instance of string.
     */
    static inline const char* duplicateStringValue(const char* value, unsigned int length = unknown) {
        if (length == unknown) length = (unsigned int)strlen(value);
        char* newString = static_cast<char*>(malloc(length + 1));
        JSONCPP_ASSERT_MESSAGE(newString != 0, "Failed to allocate string value buffer");
        memcpy(newString, value, length);
        newString[length] = 0;
        return newString;
    }

    /** Free the string duplicated by duplicateStringValue().
     */
    static inline void releaseStringValue(char* value) {
        if (value) free(value);
    }

} // namespace Json

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// ValueInternals...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !defined(JSONCPP_IS_AMALGAMATION)

#include "json_valueiterator.inl"
#endif // if !defined(JSONCPP_IS_AMALGAMATION)

namespace Json {

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class Value::CZString
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    // Notes: index_ indicates if the string was allocated when
    // a string is stored.

    Value::CZString::CZString(ArrayIndex) : cstr_(0) {}

    Value::CZString::CZString(const CZString& other) : cstr_{ nullptr } {
        const auto len = strlen(other.cstr_);
        const auto chars = static_cast<char*>(malloc(len + 1));
        chars[len] = 0x0;

        memcpy(chars, other.cstr_, len);
        cstr_ = chars;
    }

    Value::CZString::CZString(const char* data, int leng) {
        if (leng == unknown) leng = static_cast<int>(strlen(data));
        const auto chars = static_cast<char*>(malloc(leng + 1));
        chars[leng] = 0x0;

        memcpy(chars, data, leng);
        cstr_ = chars;
    }

    Value::CZString::~CZString() { releaseStringValue(const_cast<char*>(cstr_)); }

    void Value::CZString::swap(CZString& other) { std::swap(cstr_, other.cstr_); }

    Value::CZString& Value::CZString::operator=(const CZString& other) {
        CZString temp(other);
        swap(temp);
        return *this;
    }

    bool Value::CZString::operator<(const CZString& other) const {
        if (cstr_ && other.c_str()) return strcmp(cstr_, other.cstr_) < 0;
        return false;
    }

    bool Value::CZString::operator==(const CZString& other) const {
        if (cstr_ && other.c_str()) return strcmp(cstr_, other.cstr_) == 0;
        return cstr_ == other.cstr_;
    }

    const char* Value::CZString::c_str() const { return cstr_; }

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class Value::Value
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    /*! \internal Default constructor initialization must be equivalent to:
     * memset( this, 0, sizeof(Value) )
     * This optimization is used in ValueInternalMap fast allocator.
     */
    Value::Value(ValueType type) : type_(type) {
        switch (type) {
        case nullValue:
            break;
        case intValue:
        case uintValue:
            value_.int_ = 0;
            break;
        case realValue:
            value_.real_ = 0.0;
            break;
        case stringValue:
            value_.string_ = nullptr;
            break;
        case arrayValue:
            value_.array_ = new std::vector<Value*>();
            break;
        case objectValue:
            value_.map_ = new ObjectValues();
            break;

        case booleanValue:
            value_.bool_ = false;
            break;
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
    }

#if defined(JSONCPP_HAS_INT64)
    Value::Value(UInt value) : type_(uintValue) { value_.uint_ = value; }

    Value::Value(Int value) : type_(intValue) { value_.int_ = value; }

#endif // if defined(JSONCPP_HAS_INT64)

    Value::Value(Int64 value) : type_(intValue) { value_.int_ = value; }

    Value::Value(UInt64 value) : type_(uintValue) { value_.uint_ = value; }

    Value::Value(double value) : type_(realValue) { value_.real_ = value; }

    Value::Value(const char* value) : type_(stringValue) { value_.string_ = new CZString(value); }

    Value::Value(const char* beginValue, const char* endValue) : type_(stringValue) {
        value_.string_ = new CZString(beginValue, (unsigned int)(endValue - beginValue));
    }

    Value::Value(const std::string& value) : type_(stringValue) {
        value_.string_ = new CZString(value.c_str(), static_cast<unsigned int>(value.length()));
    }

    Value::Value(const StaticString& value) : type_(stringValue) { value_.string_ = new CZString(value.c_str()); }

    Value::Value(bool value) : type_(booleanValue) { value_.bool_ = value; }

    Value::Value(const Value& other) : type_(other.type_) {
        switch (type_) {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
            value_ = other.value_;
            break;
        case stringValue:
            if (other.value_.string_) {
                value_.string_ = new CZString(*other.value_.string_);
            } else
                value_.string_ = nullptr;
            break;
        case arrayValue:
            value_.array_ = new std::vector<Value*>(*other.value_.array_);
            break;
        case objectValue:
            value_.map_ = new ObjectValues(*other.value_.map_);
            break;
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
    }

    Value::~Value() {
        switch (type_) {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
            break;
        case stringValue:
            delete value_.string_;
            break;
        case arrayValue:
            delete value_.array_;
            break;
        case objectValue:
            delete value_.map_;
            break;
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
    }

    Value& Value::operator=(const Value& other) {
        Value temp(other);
        swap(temp);
        return *this;
    }

    void Value::swap(Value& other) {
        ValueType temp = type_;
        type_ = other.type_;
        other.type_ = temp;
        std::swap(value_, other.value_);
    }

    ValueType Value::type() const { return type_; }

    int Value::compare(const Value& other) const {
        if (*this < other) return -1;
        if (*this > other) return 1;
        return 0;
    }

    bool Value::operator<(const Value& other) const {
        int typeDelta = type_ - other.type_;
        if (typeDelta) return typeDelta < 0 ? true : false;
        switch (type_) {
        case nullValue:
            return false;
        case intValue:
            return value_.int_ < other.value_.int_;
        case uintValue:
            return value_.uint_ < other.value_.uint_;
        case realValue:
            return value_.real_ < other.value_.real_;
        case booleanValue:
            return value_.bool_ < other.value_.bool_;
        case stringValue:
            return (value_.string_ == 0 && other.value_.string_) ||
                   (other.value_.string_ && value_.string_ &&
                    strcmp(value_.string_->c_str(), other.value_.string_->c_str()) < 0);
        case arrayValue: {
            int delta = static_cast<int>(value_.array_->size()) - static_cast<int>(other.value_.array_->size());
            if (delta != 0) return delta < 0;
            return *value_.array_ < *other.value_.array_;
        }
        case objectValue: {
            int delta = int(value_.map_->size() - other.value_.map_->size());
            if (delta) return delta < 0;
            return (*value_.map_) < (*other.value_.map_);
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return false; // unreachable
    }

    bool Value::operator<=(const Value& other) const { return !(other < *this); }

    bool Value::operator>=(const Value& other) const { return !(*this < other); }

    bool Value::operator>(const Value& other) const { return other < *this; }

    bool Value::operator==(const Value& other) const {
        // if ( type_ != other.type_ )
        //  GCC 2.95.3 says:
        //  attempt to take address of bit-field structure member `Json::Value::type_'
        //  Beats me, but a temp solves the problem.
        int temp = other.type_;
        if (type_ != temp) return false;
        switch (type_) {
        case nullValue:
            return true;
        case intValue:
            return value_.int_ == other.value_.int_;
        case uintValue:
            return value_.uint_ == other.value_.uint_;
        case realValue:
            return value_.real_ == other.value_.real_;
        case booleanValue:
            return value_.bool_ == other.value_.bool_;
        case stringValue:
            return (value_.string_ == other.value_.string_) ||
                   (other.value_.string_ && value_.string_ &&
                    strcmp(value_.string_->c_str(), other.value_.string_->c_str()) == 0);
        case arrayValue:
            return value_.array_->size() == other.value_.array_->size() && *value_.array_ == *other.value_.array_;
        case objectValue:
            return value_.map_->size() == other.value_.map_->size() && (*value_.map_) == (*other.value_.map_);
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return false; // unreachable
    }

    bool Value::operator!=(const Value& other) const { return !(*this == other); }

    const char* Value::asCString() const {
        JSONCPP_ASSERT(type_ == stringValue);
        return value_.string_->c_str();
    }

    std::string Value::asString(const std::string& defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case stringValue: {
            const auto& data = value_.string_ ? *value_.string_ : CZString("");
            return std::string{ data.c_str() };
        }
        case booleanValue:
            return value_.bool_ ? "true" : "false";
        case intValue:
        case uintValue:
        case realValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to string");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable
    }

    Value::Int Value::asInt(Value::Int defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
            JSONCPP_ASSERT_MESSAGE(
                value_.int_ >= minInt && value_.int_ <= maxInt, "unsigned integer out of signed int range"
            );
            return Int(value_.int_);
        case uintValue:
            JSONCPP_ASSERT_MESSAGE(value_.uint_ <= UInt(maxInt), "unsigned integer out of signed int range");
            return Int(value_.uint_);
        case realValue:
            JSONCPP_ASSERT_MESSAGE(
                value_.real_ >= minInt && value_.real_ <= maxInt, "Real out of signed integer range"
            );
            return Int(value_.real_);
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        case stringValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to int");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }

    Value::UInt Value::asUInt(Value::UInt defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
            JSONCPP_ASSERT_MESSAGE(value_.int_ >= 0, "Negative integer can not be converted to unsigned integer");
            JSONCPP_ASSERT_MESSAGE(value_.int_ <= maxUInt, "signed integer out of UInt range");
            return UInt(value_.int_);
        case uintValue:
            JSONCPP_ASSERT_MESSAGE(value_.uint_ <= maxUInt, "unsigned integer out of UInt range");
            return UInt(value_.uint_);
        case realValue:
            JSONCPP_ASSERT_MESSAGE(value_.real_ >= 0 && value_.real_ <= maxUInt, "Real out of unsigned integer range");
            return UInt(value_.real_);
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        case stringValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to uint");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }

#if defined(JSONCPP_HAS_INT64)

    Value::Int64 Value::asInt64(Value::Int64 defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
            return value_.int_;
        case uintValue:
            JSONCPP_ASSERT_MESSAGE(value_.uint_ <= UInt64(maxInt64), "unsigned integer out of Int64 range");
            return value_.uint_;
        case realValue:
            JSONCPP_ASSERT_MESSAGE(
                value_.real_ >= std::nextafter(static_cast<double>(minInt64), 0.0) &&
                    value_.real_ <= std::nextafter(static_cast<double>(maxInt64), 0.0),
                "Real out of Int64 range"
            );
            return Int(value_.real_);
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        case stringValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to Int64");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }

    Value::UInt64 Value::asUInt64(Value::UInt64 defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
            JSONCPP_ASSERT_MESSAGE(value_.int_ >= 0, "Negative integer can not be converted to UInt64");
            return value_.int_;
        case uintValue:
            return value_.uint_;
        case realValue:
            JSONCPP_ASSERT_MESSAGE(
                value_.real_ >= 0 && value_.real_ <= std::nextafter(static_cast<double>(maxUInt64), 0.0),
                "Real out of UInt64 range"
            );
            return UInt(value_.real_);
        case booleanValue:
            return value_.bool_ ? 1 : 0;
        case stringValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to UInt64");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }
#endif // if defined(JSONCPP_HAS_INT64)

    LargestInt Value::asLargestInt() const {
#if defined(JSONCPP_NO_INT64)
        return asInt(0);
#else
        return asInt64(0);
#endif
    }

    LargestUInt Value::asLargestUInt() const {
#if defined(JSONCPP_NO_INT64)
        return asUInt(0);
#else
        return asUInt64(0);
#endif
    }

    double Value::asDouble(double defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
            return static_cast<double>(value_.int_);
        case uintValue:
#if !defined(JSONCPP_USE_INT64_DOUBLE_CONVERSION)
            return static_cast<double>(value_.uint_);
#else  // if !defined(JSONCPP_USE_INT64_DOUBLE_CONVERSION)
            return static_cast<double>(Int(value_.uint_ / 2)) * 2 + Int(value_.uint_ & 1);
#endif // if !defined(JSONCPP_USE_INT64_DOUBLE_CONVERSION)
        case realValue:
            return value_.real_;
        case booleanValue:
            return value_.bool_ ? 1.0 : 0.0;
        case stringValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to double");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }

    float Value::asFloat(float defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
            return static_cast<float>(value_.int_);
        case uintValue:
#if !defined(JSONCPP_USE_INT64_DOUBLE_CONVERSION)
            return static_cast<float>(value_.uint_);
#else  // if !defined(JSONCPP_USE_INT64_DOUBLE_CONVERSION)
            return static_cast<float>(Int(value_.uint_ / 2)) * 2 + Int(value_.uint_ & 1);
#endif // if !defined(JSONCPP_USE_INT64_DOUBLE_CONVERSION)
        case realValue:
            return static_cast<float>(value_.real_);
        case booleanValue:
            return value_.bool_ ? 1.0f : 0.0f;
        case stringValue:
        case arrayValue:
        case objectValue: {
            JSONCPP_FAIL_MESSAGE("Type is not convertible to float");
            break;
        }
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }

    bool Value::asBool(bool defaultValue) const {
        switch (type_) {
        case nullValue:
            return defaultValue;
        case intValue:
        case uintValue:
            return value_.int_ != 0;
        case realValue:
            return value_.real_ != 0.0;
        case booleanValue:
            return value_.bool_;
        case stringValue:
            return value_.string_ && value_.string_->c_str()[0] != 0;
        case arrayValue:
            return !value_.array_->empty();
        case objectValue:
            return !value_.map_->empty();
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return defaultValue; // unreachable;
    }

    bool Value::isConvertibleTo(ValueType other) const {
        switch (type_) {
        case nullValue:
            return true;
        case intValue:
            return (other == nullValue && value_.int_ == 0) || other == intValue ||
                   (other == uintValue && value_.int_ >= 0) || other == realValue || other == stringValue ||
                   other == booleanValue;
        case uintValue:
            return (other == nullValue && value_.uint_ == 0) ||
                   (other == intValue && value_.uint_ <= (unsigned)maxInt) || other == uintValue ||
                   other == realValue || other == stringValue || other == booleanValue;
        case realValue:
            return (other == nullValue && value_.real_ == 0.0) ||
                   (other == intValue && value_.real_ >= minInt && value_.real_ <= maxInt) ||
                   (other == uintValue && value_.real_ >= 0 && value_.real_ <= maxUInt) || other == realValue ||
                   other == stringValue || other == booleanValue;
        case booleanValue:
            return (other == nullValue && value_.bool_ == false) || other == intValue || other == uintValue ||
                   other == realValue || other == stringValue || other == booleanValue;
        case stringValue:
            return other == stringValue || (other == nullValue && (!value_.string_ || value_.string_->c_str()[0] == 0));
        case arrayValue:
            return other == arrayValue || (other == nullValue && value_.array_->empty());
        case objectValue:
            return other == objectValue || (other == nullValue && value_.map_->empty());
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return false; // unreachable;
    }

    /// Number of values in array or object
    ArrayIndex Value::size() const {
        switch (type_) {
        case nullValue:
        case intValue:
        case uintValue:
        case realValue:
        case booleanValue:
        case stringValue:
            return 0;
        case arrayValue: // size of the array is highest index + 1
            return static_cast<ArrayIndex>(value_.array_->size());
        case objectValue:
            return static_cast<ArrayIndex>(value_.map_->size());
        default:
            JSONCPP_ASSERT_UNREACHABLE;
        }
        return 0; // unreachable;
    }

    bool Value::empty() const {
        if (isNull() || isArray() || isObject())
            return size() == 0u;
        else
            return false;
    }

    bool Value::operator!() const { return isNull(); }

    void Value::clear() {
        JSONCPP_ASSERT(type_ == nullValue || type_ == arrayValue || type_ == objectValue);

        switch (type_) {
        case arrayValue:
            value_.array_->clear();
            break;
        case objectValue:
            value_.map_->clear();
            break;
        default:
            break;
        }
    }

    void Value::resize(ArrayIndex newSize) {
        JSONCPP_ASSERT(type_ == nullValue || type_ == arrayValue);
        if (type_ == nullValue) *this = Value(arrayValue);

        value_.array_->resize(newSize);
    }

    Value& Value::operator[](ArrayIndex index) {
        JSONCPP_ASSERT(type_ == nullValue || type_ == arrayValue);
        if (type_ == nullValue) *this = Value(arrayValue);

        auto* array = value_.array_;

        if (index >= array->size()) {
            array->resize(index + 1);
            array->operator[](index) = new Value(nullValue);
        }

        return *array->at(index);
    }

    Value& Value::operator[](int index) {
        JSONCPP_ASSERT(index >= 0);
        return (*this)[ArrayIndex(index)];
    }

    const Value& Value::operator[](ArrayIndex index) const {
        JSONCPP_ASSERT(type_ == nullValue || type_ == arrayValue);
        if (type_ == nullValue) return null;

        const auto* array = value_.array_;

        if (index >= array->size()) {
            return null;
        }

        return *array->at(index);
    }

    const Value& Value::operator[](int index) const {
        JSONCPP_ASSERT(index >= 0);
        return (*this)[ArrayIndex(index)];
    }

    Value& Value::operator[](const char* key) { return resolveReference(key, false); }

    Value& Value::resolveReference(const char* key, bool isStatic) {
        JSONCPP_ASSERT(type_ == nullValue || type_ == objectValue);
        if (type_ == nullValue) *this = Value(objectValue);
        CZString actualKey(key);
        auto it = value_.map_->lower_bound(actualKey);
        if (it != value_.map_->end() && (*it).first == actualKey) return (*it).second;

        ObjectValues::value_type defaultValue(actualKey, null);
        it = value_.map_->insert(it, defaultValue);
        Value& value = it->second;
        return value;
    }

    Value Value::get(ArrayIndex index, const Value& defaultValue) const {
        const Value* value = &((*this)[index]);
        return value == &null ? defaultValue : *value;
    }

    bool Value::isValidIndex(ArrayIndex index) const { return index < size(); }

    const Value& Value::operator[](const char* key) const {
        JSONCPP_ASSERT(type_ == nullValue || type_ == objectValue);
        if (type_ == nullValue) return null;
        CZString actualKey(key);
        ObjectValues::const_iterator it = value_.map_->find(actualKey);
        if (it == value_.map_->end()) return null;
        return (*it).second;
    }

    Value& Value::operator[](const std::string& key) { return (*this)[key.c_str()]; }

    const Value& Value::operator[](const std::string& key) const { return (*this)[key.c_str()]; }

    Value& Value::operator[](const StaticString& key) { return resolveReference(key, true); }

    Value& Value::append(const Value& value) { return (*this)[size()] = value; }

    Value Value::get(const char* key, const Value& defaultValue) const {
        const Value* value = &((*this)[key]);
        return value == &null ? defaultValue : *value;
    }

    Value Value::get(const std::string& key, const Value& defaultValue) const { return get(key.c_str(), defaultValue); }

    Value Value::removeMember(const char* key) {
        JSONCPP_ASSERT(type_ == nullValue || type_ == objectValue);
        if (type_ == nullValue) return null;
        CZString actualKey(key);
        ObjectValues::iterator it = value_.map_->find(actualKey);
        if (it == value_.map_->end()) return null;
        Value old(it->second);
        value_.map_->erase(it);
        return old;
    }

    Value Value::removeMember(const std::string& key) { return removeMember(key.c_str()); }

    bool Value::isMember(const char* key) const {
        const Value* value = &((*this)[key]);
        return value != &null;
    }

    bool Value::isMember(const std::string& key) const { return isMember(key.c_str()); }

    Value::Members Value::getMemberNames() const {
        JSONCPP_ASSERT(type_ == nullValue || type_ == objectValue);
        if (type_ == nullValue) return Value::Members();
        Members members;
        members.reserve(value_.map_->size());
        ObjectValues::const_iterator it = value_.map_->begin();
        ObjectValues::const_iterator itEnd = value_.map_->end();
        for (; it != itEnd; ++it)
            members.emplace_back(it->first.c_str());
        return members;
    }

    bool Value::isNull() const { return type_ == nullValue; }

    bool Value::isBool() const { return type_ == booleanValue; }

    bool Value::isInt() const { return type_ == intValue; }

    bool Value::isUInt() const { return type_ == uintValue; }

    bool Value::isIntegral() const { return type_ == intValue || type_ == uintValue || type_ == booleanValue; }

    bool Value::isDouble() const { return type_ == realValue; }

    bool Value::isNumeric() const { return isIntegral() || isDouble(); }

    bool Value::isString() const { return type_ == stringValue; }

    bool Value::isArray() const { return type_ == nullValue || type_ == arrayValue; }

    bool Value::isObject() const { return type_ == nullValue || type_ == objectValue; }

    void Value::setComment([[maybe_unused]] const char* comment, [[maybe_unused]] CommentPlacement placement) {}

    void Value::setComment(const std::string& comment, CommentPlacement placement) {
        setComment(comment.c_str(), placement);
    }

    bool Value::hasComment([[maybe_unused]] CommentPlacement placement) const { return false; }

    std::string Value::getComment([[maybe_unused]] CommentPlacement placement) const { return ""; }

    std::string Value::toStyledString() const {
        StyledWriter writer;
        return writer.write(*this);
    }

    Value::const_iterator Value::begin() const {
        switch (type_) {

        case arrayValue:
            if (value_.array_) {
                return const_iterator(value_.array_->begin());
            }
            break;
        case objectValue:
            if (value_.map_) return const_iterator(value_.map_->begin());
            break;
        default:
            break;
        }
        return const_iterator();
    }

    Value::const_iterator Value::end() const {
        switch (type_) {
        case arrayValue:
            if (value_.array_) return const_iterator(value_.array_->end());
            break;
        case objectValue:
            if (value_.map_) return const_iterator(value_.map_->end());
            break;
        default:
            break;
        }
        return const_iterator();
    }

    Value::iterator Value::begin() {
        switch (type_) {

        case arrayValue:
            if (value_.array_) return iterator(value_.array_->begin());
            break;
        case objectValue:
            if (value_.map_) return iterator(value_.map_->begin());
            break;
        default:
            break;
        }
        return iterator();
    }

    Value::iterator Value::end() {
        switch (type_) {
        case arrayValue:
            if (value_.array_) return iterator(value_.array_->end());
            break;
        case objectValue:
            if (value_.map_) return iterator(value_.map_->end());
            break;
        default:
            break;
        }
        return iterator();
    }

    // class PathArgument
    // //////////////////////////////////////////////////////////////////

    PathArgument::PathArgument() : kind_(kindNone) {}

    PathArgument::PathArgument(ArrayIndex index) : index_(index), kind_(kindIndex) {}

    PathArgument::PathArgument(const char* key) : key_(key), kind_(kindKey) {}

    PathArgument::PathArgument(const std::string& key) : key_(key.c_str()), kind_(kindKey) {}

    // class Path
    // //////////////////////////////////////////////////////////////////

    Path::Path(
        const std::string& path, const PathArgument& a1, const PathArgument& a2, const PathArgument& a3,
        const PathArgument& a4, const PathArgument& a5
    ) {
        InArgs in;
        in.push_back(&a1);
        in.push_back(&a2);
        in.push_back(&a3);
        in.push_back(&a4);
        in.push_back(&a5);
        makePath(path, in);
    }

    void Path::makePath(const std::string& path, const InArgs& in) {
        const char* current = path.c_str();
        const char* end = current + path.length();
        InArgs::const_iterator itInArg = in.begin();
        while (current != end) {
            if (*current == '[') {
                ++current;
                if (*current == '%')
                    addPathInArg(path, in, itInArg, PathArgument::kindIndex);
                else {
                    ArrayIndex index = 0;
                    for (; current != end && *current >= '0' && *current <= '9'; ++current)
                        index = index * 10 + ArrayIndex(*current - '0');
                    args_.push_back(index);
                }
                if (current == end || *current++ != ']') invalidPath(path, int(current - path.c_str()));
            } else if (*current == '%') {
                addPathInArg(path, in, itInArg, PathArgument::kindKey);
                ++current;
            } else if (*current == '.') {
                ++current;
            } else {
                const char* beginName = current;
                while (current != end && !strchr("[.", *current))
                    ++current;
                args_.push_back(std::string(beginName, current));
            }
        }
    }

    void Path::addPathInArg(
        [[maybe_unused]] const std::string& path, const InArgs& in, InArgs::const_iterator& itInArg,
        PathArgument::Kind kind
    ) {
        if (itInArg == in.end()) {
            // Error: missing argument %d
        } else if ((*itInArg)->kind_ != kind) {
            // Error: bad argument type
        } else {
            args_.push_back(**itInArg);
        }
    }

    void Path::invalidPath([[maybe_unused]] const std::string& path, [[maybe_unused]] int location) {
        // Error: invalid path.
    }

    const Value& Path::resolve(const Value& root) const {
        const Value* node = &root;
        for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
            const PathArgument& arg = *it;
            if (arg.kind_ == PathArgument::kindIndex) {
                if (!node->isArray() || node->isValidIndex(arg.index_)) {
                    // Error: unable to resolve path (array value expected at position...
                }
                node = &((*node)[arg.index_]);
            } else if (arg.kind_ == PathArgument::kindKey) {
                if (!node->isObject()) {
                    // Error: unable to resolve path (object value expected at position...)
                }
                node = &((*node)[arg.key_]);
                if (node == &Value::null) {
                    // Error: unable to resolve path (object has no member named '' at position...)
                }
            }
        }
        return *node;
    }

    Value Path::resolve(const Value& root, const Value& defaultValue) const {
        const Value* node = &root;
        for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
            const PathArgument& arg = *it;
            if (arg.kind_ == PathArgument::kindIndex) {
                if (!node->isArray() || node->isValidIndex(arg.index_)) return defaultValue;
                node = &((*node)[arg.index_]);
            } else if (arg.kind_ == PathArgument::kindKey) {
                if (!node->isObject()) return defaultValue;
                node = &((*node)[arg.key_]);
                if (node == &Value::null) return defaultValue;
            }
        }
        return *node;
    }

    Value& Path::make(Value& root) const {
        Value* node = &root;
        for (Args::const_iterator it = args_.begin(); it != args_.end(); ++it) {
            const PathArgument& arg = *it;
            if (arg.kind_ == PathArgument::kindIndex) {
                if (!node->isArray()) {
                    // Error: node is not an array at position ...
                }
                node = &((*node)[arg.index_]);
            } else if (arg.kind_ == PathArgument::kindKey) {
                if (!node->isObject()) {
                    // Error: node is not an object at position...
                }
                node = &((*node)[arg.key_]);
            }
        }
        return *node;
    }

} // namespace Json
