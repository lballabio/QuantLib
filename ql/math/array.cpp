/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2006, 2009 StatPro Italia srl
 Copyright (C) 2004 Ferdinando Ametrano

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/array.hpp>
#include <ql/errors.hpp>
#include <iomanip>

namespace QuantLib {

    Array::Array(Size size)
    : data_(size != 0U ? new Real[size] : (Real*)nullptr), n_(size) {}

    Array::Array(Size size, Real value)
    : data_(size != 0U ? new Real[size] : (Real*)nullptr), n_(size) {
        std::fill(begin(),end(),value);
    }

    Array::Array(Size size, Real value, Real increment)
    : data_(size != 0U ? new Real[size] : (Real*)nullptr), n_(size) {
        for (iterator i=begin(); i!=end(); ++i, value+=increment)
            *i = value;
    }

    Array::Array(const Array& from)
    : data_(from.n_ != 0U ? new Real[from.n_] : (Real*)nullptr), n_(from.n_) {
        if (data_)
            std::copy(from.begin(),from.end(),begin());
    }

    Array::Array(Array&& from) noexcept
    : data_((Real*)nullptr), n_(0) {
        swap(from);
    }

    Array::Array(std::initializer_list<Real> init) {
        detail::_fill_array_(*this, data_, n_, init.begin(), init.end(),
                             std::false_type());
    }

    Array& Array::operator=(const Array& from) {
        // strong guarantee
        Array temp(from);
        swap(temp);
        return *this;
    }

    Array& Array::operator=(Array&& from) noexcept {
        swap(from);
        return *this;
    }

    bool Array::operator==(const Array& to) const {
        return (n_ == to.n_) && std::equal(begin(), end(), to.begin());
    }

    bool Array::operator!=(const Array& to) const {
        return !(this->operator==(to));
    }

    const Array& Array::operator+=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be added");
        std::transform(begin(),end(),v.begin(),begin(), std::plus<>());
        return *this;
    }


    const Array& Array::operator+=(Real x) {
        std::transform(begin(), end(), begin(), [=](Real y) -> Real { return y + x; });
        return *this;
    }

    const Array& Array::operator-=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be subtracted");
        std::transform(begin(), end(), v.begin(), begin(), std::minus<>());
        return *this;
    }

    const Array& Array::operator-=(Real x) {
        std::transform(begin(),end(),begin(), [=](Real y) -> Real { return y - x; });
        return *this;
    }

    const Array& Array::operator*=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be multiplied");
        std::transform(begin(), end(), v.begin(), begin(), std::multiplies<>());
        return *this;
    }

    const Array& Array::operator*=(Real x) {
        std::transform(begin(), end(), begin(), [=](Real y) -> Real { return y * x; });
        return *this;
    }

    const Array& Array::operator/=(const Array& v) {
        QL_REQUIRE(n_ == v.n_,
                   "arrays with different sizes (" << n_ << ", "
                   << v.n_ << ") cannot be divided");
        std::transform(begin(), end(), v.begin(), begin(), std::divides<>());
        return *this;
    }

    const Array& Array::operator/=(Real x) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(x != 0.0, "division by zero");
        #endif
        std::transform(begin(), end(), begin(), [=](Real y) -> Real { return y / x; });
        return *this;
    }

    Real Array::operator[](Size i) const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        #endif
        return data_.get()[i];
    }

    Real Array::at(Size i) const {
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        return data_.get()[i];
    }

    Real Array::front() const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null Array: array access out of range");
        #endif
        return data_.get()[0];
    }

    Real Array::back() const {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null Array: array access out of range");
        #endif
        return data_.get()[n_-1];
    }

    Real& Array::operator[](Size i) {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        #endif
        return data_.get()[i];
    }

    Real& Array::at(Size i) {
        QL_REQUIRE(i<n_,
                   "index (" << i << ") must be less than " << n_ <<
                   ": array access out of range");
        return data_.get()[i];
    }

    Real& Array::front() {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null Array: array access out of range");
        #endif
        return data_.get()[0];
    }

    Real& Array::back() {
        #if defined(QL_EXTRA_SAFETY_CHECKS)
        QL_REQUIRE(n_>0, "null Array: array access out of range");
        #endif
        return data_.get()[n_-1];
    }

    Size Array::size() const {
        return n_;
    }

    bool Array::empty() const {
        return n_ == 0;
    }

    Array::const_iterator Array::begin() const {
        return data_.get();
    }

    Array::iterator Array::begin() {
        return data_.get();
    }

    Array::const_iterator Array::end() const {
        return data_.get()+n_;
    }

    Array::iterator Array::end() {
        return data_.get()+n_;
    }

    Array::const_reverse_iterator Array::rbegin() const {
        return const_reverse_iterator(end());
    }

    Array::reverse_iterator Array::rbegin() {
        return reverse_iterator(end());
    }

    Array::const_reverse_iterator Array::rend() const {
        return const_reverse_iterator(begin());
    }

    Array::reverse_iterator Array::rend() {
        return reverse_iterator(begin());
    }

    void Array::resize(Size n) {
        if (n > n_) {
            Array swp(n);
            std::copy(begin(), end(), swp.begin());
            swap(swp);
        }
        else if (n < n_) {
            n_ = n;
        }
    }

    void Array::swap(Array& from) noexcept {
        data_.swap(from.data_);
        std::swap(n_, from.n_);
    }

    // dot product and norm

    Real DotProduct(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        return std::inner_product(v1.begin(),v1.end(),v2.begin(),Real(0.0));
    }

    Real Norm2(const Array& v) {
        return std::sqrt(DotProduct(v, v));
    }

    // overloaded operators

    // unary

    Array operator+(const Array& v) {
        Array result = v;
        return result;
    }

    Array operator+(Array&& v) {
        return std::move(v);
    }

    Array operator-(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(), v.end(), result.begin(), std::negate<>());
        return result;
    }

    Array operator-(Array&& v) {
        Array result = std::move(v);
        std::transform(result.begin(), result.end(), result.begin(), std::negate<>());
        return result;
    }

    // binary operators

    Array operator+(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be added");
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),v2.begin(),result.begin(), std::plus<>());
        return result;
    }

    Array operator+(const Array& v1, Array&& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be added");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::plus<>());
        return result;
    }

    Array operator+(Array&& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be added");
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), v2.begin(), result.begin(), std::plus<>());
        return result;
    }

    Array operator+(Array&& v1, Array&& v2) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be added");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::plus<>());
        return result;
    }

    Array operator+(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(), v1.end(), result.begin(), [=](Real y) -> Real { return y + a; });
        return result;
    }

    Array operator+(Array&& v1, Real a) {
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return y + a; });
        return result;
    }

    Array operator+(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(), [=](Real y) -> Real { return a + y; });
        return result;
    }

    Array operator+(Real a, Array&& v2) {
        Array result = std::move(v2);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return a + y; });
        return result;
    }

    Array operator-(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be subtracted");
        Array result(v1.size());
        std::transform(v1.begin(), v1.end(), v2.begin(), result.begin(), std::minus<>());
        return result;
    }

    Array operator-(const Array& v1, Array&& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be subtracted");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::minus<>());
        return result;
    }

    Array operator-(Array&& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be subtracted");
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), v2.begin(), result.begin(), std::minus<>());
        return result;
    }

    Array operator-(Array&& v1, Array&& v2) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be subtracted");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::minus<>());
        return result;
    }

    Array operator-(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(), [=](Real y) -> Real { return y - a; });
        return result;
    }

    Array operator-(Array&& v1, Real a) {
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return y - a; });
        return result;
    }

    Array operator-(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(), [=](Real y) -> Real { return a - y; });
        return result;
    }

    Array operator-(Real a, Array&& v2) {
        Array result = std::move(v2);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return a - y; });
        return result;
    }

    Array operator*(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        Array result(v1.size());
        std::transform(v1.begin(), v1.end(), v2.begin(), result.begin(), std::multiplies<>());
        return result;
    }

    Array operator*(const Array& v1, Array&& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::multiplies<>());
        return result;
    }

    Array operator*(Array&& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), v2.begin(), result.begin(), std::multiplies<>());
        return result;
    }

    Array operator*(Array&& v1, Array&& v2) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be multiplied");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::multiplies<>());
        return result;
    }

    Array operator*(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(), [=](Real y) -> Real { return y * a; });
        return result;
    }

    Array operator*(Array&& v1, Real a) {
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return y * a; });
        return result;
    }

    Array operator*(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(), [=](Real y) -> Real { return a * y; });
        return result;
    }

    Array operator*(Real a, Array&& v2) {
        Array result = std::move(v2);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return a * y; });
        return result;
    }

    Array operator/(const Array& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be divided");
        Array result(v1.size());
        std::transform(v1.begin(), v1.end(), v2.begin(), result.begin(), std::divides<>());
        return result;
    }

    Array operator/(const Array& v1, Array&& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be divided");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::divides<>());
        return result;
    }

    Array operator/(Array&& v1, const Array& v2) {
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be divided");
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), v2.begin(), result.begin(), std::divides<>());
        return result;
    }

    Array operator/(Array&& v1, Array&& v2) { // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved)
        QL_REQUIRE(v1.size() == v2.size(),
                   "arrays with different sizes (" << v1.size() << ", "
                   << v2.size() << ") cannot be divided");
        Array result = std::move(v2);
        std::transform(v1.begin(), v1.end(), result.begin(), result.begin(), std::divides<>());
        return result;
    }

    Array operator/(const Array& v1, Real a) {
        Array result(v1.size());
        std::transform(v1.begin(),v1.end(),result.begin(), [=](Real y) -> Real { return y / a; });
        return result;
    }

    Array operator/(Array&& v1, Real a) {
        Array result = std::move(v1);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return y / a; });
        return result;
    }

    Array operator/(Real a, const Array& v2) {
        Array result(v2.size());
        std::transform(v2.begin(),v2.end(),result.begin(), [=](Real y) -> Real { return a / y; });
        return result;
    }

    Array operator/(Real a, Array&& v2) {
        Array result = std::move(v2);
        std::transform(result.begin(), result.end(), result.begin(), [=](Real y) -> Real { return a / y; });
        return result;
    }

    // functions

    Array Abs(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(), v.end(), result.begin(),
                       [](Real x) -> Real { return std::fabs(x); });
        return result;
    }

    Array Abs(Array&& v) {
        Array result = std::move(v);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](Real x) -> Real { return std::fabs(x); });
        return result;
    }

    Array Sqrt(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       [](Real x) -> Real { return std::sqrt(x); });
        return result;
    }

    Array Sqrt(Array&& v) {
        Array result = std::move(v);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](Real x) -> Real { return std::sqrt(x); });
        return result;
    }

    Array Log(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(),v.end(),result.begin(),
                       [](Real x) -> Real { return std::log(x); });
        return result;
    }

    Array Log(Array&& v) {
        Array result = std::move(v);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](Real x) -> Real { return std::log(x); });
        return result;
    }

    Array Exp(const Array& v) {
        Array result(v.size());
        std::transform(v.begin(), v.end(), result.begin(),
                       [](Real x) -> Real { return std::exp(x); });
        return result;
    }

    Array Exp(Array&& v) {
        Array result = std::move(v);
        std::transform(result.begin(), result.end(), result.begin(),
                       [](Real x) -> Real { return std::exp(x); });
        return result;
    }

    Array Pow(const Array& v, Real alpha) {
        Array result(v.size());
        std::transform(v.begin(), v.end(), result.begin(),
                       [=](Real x) -> Real { return std::pow(x, alpha); });
        return result;
    }

    Array Pow(Array&& v, Real alpha) {
        Array result = std::move(v);
        std::transform(result.begin(), result.end(), result.begin(),
                       [=](Real x) -> Real { return std::pow(x, alpha); });
        return result;
    }

    void swap(Array& v, Array& w) noexcept {
        v.swap(w);
    }

    std::ostream& operator<<(std::ostream& out, const Array& a) {
        std::streamsize width = out.width();
        out << "[ ";
        if (!a.empty()) {
            for (Size n=0; n<a.size()-1; ++n)
                out << std::setw(int(width)) << a[n] << "; ";
            out << std::setw(int(width)) << a.back();
        }
        out << " ]";
        return out;
    }

}
