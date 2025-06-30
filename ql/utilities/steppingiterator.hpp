/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file steppingiterator.hpp
    \brief Iterator advancing in constant steps
*/

#ifndef quantlib_stepping_iterator_hpp
#define quantlib_stepping_iterator_hpp

#include <ql/errors.hpp>
#include <ql/types.hpp>
#include <iterator>
#include <type_traits>

namespace QuantLib {

    //! Iterator advancing in constant steps
    /*! This iterator advances an underlying random-access iterator in
        steps of \f$ n \f$ positions, where \f$ n \f$ is a positive
        integer given upon construction.
    */
#ifdef __cpp_concepts
    template <std::random_access_iterator Iterator>
#else
    template <class Iterator>
#endif
    class step_iterator {  // NOLINT(cppcoreguidelines-special-member-functions)
      private:
        Iterator base_{};
        // a Size would mess up integer division in distance_to
        BigInteger step_{};

      public:
        using iterator_category = typename std::iterator_traits<Iterator>::iterator_category;
        using difference_type = typename std::iterator_traits<Iterator>::difference_type;
        using value_type = typename std::iterator_traits<Iterator>::value_type;
        using pointer = typename std::iterator_traits<Iterator>::pointer;
        using reference = typename std::iterator_traits<Iterator>::reference;

        step_iterator() = default;

        explicit step_iterator(const Iterator& base, Size step)
        : base_(base), step_(static_cast<BigInteger>(step)) {}

        template <class OtherIterator>
        step_iterator(const step_iterator<OtherIterator>& i,
                      std::enable_if_t<std::is_convertible_v
                      <OtherIterator, Iterator>>* = nullptr)
        : base_(i.base_), step_(static_cast<BigInteger>(i.step())) {}

        Size step() const { return static_cast<Size>(this->step_); }

        step_iterator& operator=(const step_iterator& other) = default;

        step_iterator& operator++() {
            base_ += step_;
            return *this;
        }

        step_iterator operator++(int) {
            auto tmp = *this;
            base_ += step_;
            return tmp;
        }

        reference operator*() const {
            return *base_;
        }

        step_iterator& operator--() {
            base_ -= step_;
            return *this;
        }

        step_iterator operator--(int) {
            auto tmp = *this;
            base_ -= step_;
            return tmp;
        }

        step_iterator& operator+=(Size n) {
            base_ += n * step_;
            return *this;
        }

        step_iterator& operator-=(Size n) {
            base_ -= n * step_;
            return *this;
        }

        reference operator[](Size n) const {
            return *(base_ + n * step_);
        }

        friend step_iterator operator+(const step_iterator& i, Size n) {
            return step_iterator(i.base_ + n * i.step_, i.step_);
        }

        friend step_iterator operator+(Size n, const step_iterator& i) {
            return step_iterator(i.base_ + n * i.step_, i.step_);
        }

        friend step_iterator operator-(const step_iterator& i, Size n) {
            return step_iterator(i.base_ - n * i.step_, i.step_);
        }

        friend difference_type operator-(const step_iterator& lhs, const step_iterator& rhs) {
#ifdef QL_EXTRA_SAFETY_CHECKS
            QL_REQUIRE(lhs.step_ == rhs.step_, "step_iterators with different step cannot be added or subtracted");
#endif
            return (lhs.base_ - rhs.base_) / lhs.step_;
        }

        friend bool operator==(const step_iterator& lhs, const step_iterator& rhs) {
            return lhs.base_ == rhs.base_ && lhs.step_ == rhs.step_;
        }

        friend bool operator!=(const step_iterator& lhs, const step_iterator& rhs) {
            return lhs.base_ != rhs.base_ || lhs.step_ != rhs.step_;
        }

        friend bool operator<(const step_iterator& lhs, const step_iterator& rhs) {
#ifdef QL_EXTRA_SAFETY_CHECKS
            QL_REQUIRE(lhs.step_ == rhs.step_, "step_iterators with different step cannot be compared");
#endif
            return lhs.base_ < rhs.base_;
        }

        friend bool operator>(const step_iterator& lhs, const step_iterator& rhs) {
#ifdef QL_EXTRA_SAFETY_CHECKS
            QL_REQUIRE(lhs.step_ == rhs.step_, "step_iterators with different step cannot be compared");
#endif
            return lhs.base_ > rhs.base_;
        }

        friend bool operator<=(const step_iterator& lhs, const step_iterator& rhs) {
#ifdef QL_EXTRA_SAFETY_CHECKS
            QL_REQUIRE(lhs.step_ == rhs.step_, "step_iterators with different step cannot be compared");
#endif
            return lhs.base_ <= rhs.base_;
        }

        friend bool operator>=(const step_iterator& lhs, const step_iterator& rhs) {
#ifdef QL_EXTRA_SAFETY_CHECKS
            QL_REQUIRE(lhs.step_ == rhs.step_, "step_iterators with different step cannot be compared");
#endif
            return lhs.base_ >= rhs.base_;
        }
    };

    //! helper function to create step iterators
    /*! \relates step_iterator */
    template <class Iterator>
    step_iterator<Iterator> make_step_iterator(Iterator it, Size step) {
        return step_iterator<Iterator>(it,step);
    }

}


#endif
