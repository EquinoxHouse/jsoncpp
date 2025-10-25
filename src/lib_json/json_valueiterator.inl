// Copyright 2007-2010 Baptiste Lepilleur
// Distributed under MIT license, or public domain if desired and
// recognized in your jurisdiction.
// See file LICENSE for detail or copy at http://jsoncpp.sourceforge.net/LICENSE

// included by json_value.cpp

namespace Json {

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class ValueIteratorBase
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    ValueIteratorBase::ValueIteratorBase() : iterator_{ .array_ = {} }, isArray_(true) {}

    ValueIteratorBase::ValueIteratorBase(const Value::ObjectValues::iterator& current)
        : iterator_{ .map_ = current }, isArray_(false) {}

    ValueIteratorBase::ValueIteratorBase(const Value::ArrayValues::iterator& current)
        : iterator_{ .array_ = current }, isArray_(true) {}

    Value& ValueIteratorBase::deref() const {
        if (isArray_) {
            return **iterator_.array_;
        } else {
            return iterator_.map_->second;
        }
    }

    void ValueIteratorBase::increment() {
        if (isArray_) {
            ++iterator_.array_;
        } else {
            ++iterator_.map_;
        }
    }

    void ValueIteratorBase::decrement() {
        if (isArray_) {
            --iterator_.array_;
        } else {
            --iterator_.map_;
        }
    }

    ValueIteratorBase::difference_type ValueIteratorBase::computeDistance(const SelfType& other) const {
        if (isArray_) {
            return std::distance(iterator_.array_, other.iterator_.array_);
        } else {
            return std::distance(iterator_.map_, other.iterator_.map_);
        }
    }

    bool ValueIteratorBase::isEqual(const SelfType& other) const {
        if (other.isArray_ && isArray_) {
            return other.iterator_.array_ == iterator_.array_;
        }

        if (other.isArray_) {
            return false;
        }

        if (isArray_) {
            return false;
        }

        return iterator_.map_ == other.iterator_.map_;
    }

    void ValueIteratorBase::copy(const SelfType& other) {
        if (isArray_) {
            iterator_.array_ = other.iterator_.array_;
        } else {
            iterator_.map_ = other.iterator_.map_;
        }
    }

    Value ValueIteratorBase::key() const {
        if (isArray_) {
            return Value(-1);
        }
        return Value(StaticString(iterator_.map_->first.c_str()));
    }

    UInt ValueIteratorBase::index() const { return -1; }

    const char* ValueIteratorBase::memberName() const {
        if (isArray_) {
            return "";
        }

        const char* name = iterator_.map_->first.c_str();
        return name ? name : "";
    }

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class ValueConstIterator
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    ValueConstIterator::ValueConstIterator() {}

    ValueConstIterator::ValueConstIterator(const Value::ObjectValues::iterator& current) : ValueIteratorBase(current) {}

    ValueConstIterator::ValueConstIterator(const Value::ArrayValues::iterator& current) : ValueIteratorBase(current) {}

    ValueConstIterator& ValueConstIterator::operator=(const ValueIteratorBase& other) {
        copy(other);
        return *this;
    }

    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // class ValueIterator
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////
    // //////////////////////////////////////////////////////////////////

    ValueIterator::ValueIterator() {}

    ValueIterator::ValueIterator(const Value::ObjectValues::iterator& current) : ValueIteratorBase(current) {}
    ValueIterator::ValueIterator(const Value::ArrayValues::iterator& current) : ValueIteratorBase(current) {}

    ValueIterator::ValueIterator(const ValueConstIterator& other) : ValueIteratorBase(other) {}

    ValueIterator::ValueIterator(const ValueIterator& other) : ValueIteratorBase(other) {}

    ValueIterator& ValueIterator::operator=(const SelfType& other) {
        copy(other);
        return *this;
    }

} // namespace Json
