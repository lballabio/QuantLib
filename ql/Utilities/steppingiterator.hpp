
/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file steppingiterator.hpp
    \brief Iterator advancing in constant steps
*/

#ifndef quantlib_stepping_iterator_h
#define quantlib_stepping_iterator_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! Iterator advancing in constant steps
    /*! This iterator advances an underlying random access
        iterator in steps of \f$ n \f$ positions, where
        \f$ n \f$ is an integer given upon construction.
    */
    template <class RandomAccessIterator>
    class stepping_iterator : public QL_ITERATOR<
        std::random_access_iterator_tag,
        typename QL_ITERATOR_TRAITS<RandomAccessIterator>::value_type,
        typename QL_ITERATOR_TRAITS<
        RandomAccessIterator>::difference_type,
        typename QL_ITERATOR_TRAITS<RandomAccessIterator>::pointer,
        typename QL_ITERATOR_TRAITS<RandomAccessIterator>::reference>
    {
      public:
        /* These typedefs are needed even though inherited from 
           QL_ITERATOR (see 14.6.2.3 of the standard).  */
        typedef typename QL_ITERATOR_TRAITS<
            RandomAccessIterator>::difference_type difference_type;
        typedef typename QL_ITERATOR_TRAITS<
            RandomAccessIterator>::pointer pointer;
        typedef typename QL_ITERATOR_TRAITS<
            RandomAccessIterator>::reference reference;

        stepping_iterator(const RandomAccessIterator& it,
                          difference_type step)
        : dn_(step), it_(it) {}
        //! \name Dereferencing
        //@{
        reference operator*() const {
            return *it_;
        }
        pointer operator->() const {
            return it_;
        }
        //@}
        //! \name Random access
        //@{
        reference operator[](int i) const {
            return it_[i*dn_];
        }
        //@}
        //! \name Increment and decrement
        //@{
        stepping_iterator& operator++() {
            it_ += dn_;
            return *this;
        }
        stepping_iterator  operator++(int) {
            stepping_iterator temp = *this;
            it_ += dn_;
            return temp;
        }
        stepping_iterator& operator--() {
            it_ -= dn_;
            return *this;
        }
        stepping_iterator operator--(int) {
            stepping_iterator temp = *this;
            it_ -= dn_;
            return temp;
        }
        stepping_iterator& operator+=(difference_type i) {
            it_ += i*dn_;
            return *this;
        }
        stepping_iterator& operator-=(difference_type i) {
            it_ -= i*dn_;
            return *this;
        }
        stepping_iterator operator+(difference_type i) {
            return stepping_iterator(it_+dn_*i,dn_);
        }
        stepping_iterator operator-(difference_type i) {
            return stepping_iterator(it_-dn_*i,dn_);
        }
        //@}
        //! \name Difference
        //@{
        difference_type operator-(const stepping_iterator& i) {
            QL_REQUIRE((it_-i.it_)%dn_ == 0,
                       "cannot subtract stepping iterators "
                       "not reachable from each other");
            return (it_-i.it_)/dn_;
        }
        //@}
        //! \name Comparisons
        //@{
        bool operator==(const stepping_iterator& i) {
            return it_ == i.it_;
        }
        bool operator!=(const stepping_iterator& i) {
            return it_ != i.it_;
        }
        bool operator< (const stepping_iterator& i) {
            return it_ < i.it_;
        }
        bool operator> (const stepping_iterator& i) {
            return it_ > i.it_;
        }
        bool operator<=(const stepping_iterator& i) {
            return it_ <= i.it_;
        }
        bool operator>=(const stepping_iterator& i) {
            return it_ >= i.it_;
        }
        //@}
      private:
        difference_type dn_;
        RandomAccessIterator it_;
    };

    //! helper function to create stepping iterators
    /*! \relates stepping_iterator */
    template <class Iterator>
    stepping_iterator<Iterator> make_stepping_iterator(
                 Iterator it,
                 typename stepping_iterator<Iterator>::difference_type step) {
        return stepping_iterator<Iterator>(it,step);
    }

}


#endif
