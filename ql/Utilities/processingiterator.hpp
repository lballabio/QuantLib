
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

/*! \file processingiterator.hpp
    \brief Iterator mapping a unary function to an underlying sequence
*/

#ifndef quantlib_processing_iterator_h
#define quantlib_processing_iterator_h

#include <ql/qldefines.hpp>

namespace QuantLib {

    //! Iterator mapping a unary function to an underlying sequence
    /*! This iterator advances an underlying iterator and returns the
        values obtained by applying a unary function to the values
        such iterator points to.

        This class was implemented based on Christopher Baus and Thomas
        Becker, <i>Custom Iterators for the STL</i>, included in the
        proceedings of the First Workshop on C++ Template Programming,
        Erfurt, Germany, 2000 (http://www.oonumerics.org/tmpw00/)
    */
    template <class Iterator, class UnaryFunction>
    class processing_iterator : public QL_ITERATOR<
        typename QL_ITERATOR_TRAITS<Iterator>::iterator_category,
        typename UnaryFunction::result_type,
        typename QL_ITERATOR_TRAITS<Iterator>::difference_type,
        const typename UnaryFunction::result_type*,
        const typename UnaryFunction::result_type&>
    {
      public:
        /* These typedefs are needed even though inherited from 
           QL_ITERATOR (see 14.6.2.3 of the standard).  */
        typedef typename UnaryFunction::result_type value_type;
        typedef typename QL_ITERATOR_TRAITS<Iterator>::difference_type
        difference_type;
        typedef const value_type* pointer;
        typedef const value_type& reference;
        // constructor
        processing_iterator(const Iterator&, const UnaryFunction&);
        //! \name Dereferencing
        //@{
        reference operator*()  const;
        pointer   operator->() const;
        //@}
        //! \name Random access
        //@{
        value_type operator[](difference_type) const;
        //@}
        //! \name Increment and decrement
        //@{
        processing_iterator& operator++();
        processing_iterator  operator++(int );
        processing_iterator& operator--();
        processing_iterator  operator--(int );
        processing_iterator& operator+=(difference_type);
        processing_iterator& operator-=(difference_type);
        processing_iterator  operator+(difference_type);
        processing_iterator  operator-(difference_type);
        //@}
        //! \name Difference
        //@{
        difference_type operator-(
                          const processing_iterator<Iterator,UnaryFunction>&);
        //@}
        //! \name Comparisons
        //@{
        bool operator==(const processing_iterator<Iterator,UnaryFunction>&);
        bool operator!=(const processing_iterator<Iterator,UnaryFunction>&);
        bool operator< (const processing_iterator<Iterator,UnaryFunction>&);
        bool operator> (const processing_iterator<Iterator,UnaryFunction>&);
        bool operator<=(const processing_iterator<Iterator,UnaryFunction>&);
        bool operator>=(const processing_iterator<Iterator,UnaryFunction>&);
        //@}
      private:
        Iterator it_;
        UnaryFunction f_;
        mutable value_type x_;
    };

    //! helper function to create processing iterators
    /*! \relates processing_iterator */
    template <class Iterator, class UnaryFunction>
    processing_iterator<Iterator,UnaryFunction>
    make_processing_iterator(Iterator it, UnaryFunction p);



    // inline definitions

    template <class I, class F>
    inline processing_iterator<I,F>::processing_iterator(const I& it, 
                                                         const F& f)
    : it_(it), f_(f) {}

    template <class I, class F>
    inline processing_iterator<I,F>& 
    processing_iterator<I,F>::operator++() {
        ++it_;
        return *this;
    }

    template <class I, class F>
    inline processing_iterator<I,F>
    processing_iterator<I,F>::operator++(int ) {
        processing_iterator<I,F> temp = *this;
        ++it_;
        return temp;
    }

    template <class I, class F>
    inline processing_iterator<I,F>&
    processing_iterator<I,F>::operator--() {
        --it_;
        return *this;
    }

    template <class I, class F>
    inline processing_iterator<I,F>
    processing_iterator<I,F>::operator--(int ) {
        processing_iterator<I,F> temp = *this;
        --it_;
        return temp;
    }

    template <class I, class F>
    inline processing_iterator<I,F>&
    processing_iterator<I,F>::operator+=(
                       typename processing_iterator<I,F>::difference_type i) {
        it_+=i;
        return *this;
    }

    template <class I, class F>
    inline processing_iterator<I,F>&
    processing_iterator<I,F>::operator-=(
                       typename processing_iterator<I,F>::difference_type i) {
        it_-=i;
        return *this;
    }

    template <class I, class F>
    inline typename processing_iterator<I,F>::reference
    processing_iterator<I,F>::operator*() const {
        x_ = f_(*it_);
        return x_;
    }

    template <class I, class F>
    inline typename processing_iterator<I,F>::pointer
    processing_iterator<I,F>::operator->() const {
        x_ = f_(*it_);
        return &x_;
    }

    template <class I, class F>
    inline typename processing_iterator<I,F>::value_type
    processing_iterator<I,F>::operator[](
                 typename processing_iterator<I,F>::difference_type i) const {
        return *(*this+i);
    }

    template <class I, class F>
    inline processing_iterator<I,F>
    processing_iterator<I,F>::operator+(
                       typename processing_iterator<I,F>::difference_type i) {
        return processing_iterator<I,F>(it_+i,f_);
    }

    template <class I, class F>
    inline processing_iterator<I,F>
    processing_iterator<I,F>::operator-(
                       typename processing_iterator<I,F>::difference_type i) {
        return processing_iterator<I,F>(it_-i,f_);
    }

    template <class I, class F>
    inline typename processing_iterator<I,F>::difference_type
    processing_iterator<I,F>::operator-(const processing_iterator<I,F>& i) {
        return (it_-i.it_);
    }

    template <class I, class F>
    inline bool processing_iterator<I,F>::operator==(
                                          const processing_iterator<I,F>& i) {
        return (it_ == i.it_);
    }

    template <class I, class F>
    inline bool processing_iterator<I,F>::operator!=(
                                          const processing_iterator<I,F>& i) {
        return (it_ != i.it_);
    }

    template <class I, class F>
    inline bool processing_iterator<I,F>::operator<(
                                          const processing_iterator<I,F>& i) {
        return (it_ < i.it_);
    }

    template <class I, class F>
    inline bool processing_iterator<I,F>::operator>(
                                          const processing_iterator<I,F>& i) {
        return (it_ > i.it_);
    }

    template <class I, class F>
    inline bool processing_iterator<I,F>::operator<=(
                                          const processing_iterator<I,F>& i) {
        return (it_ <= i.it_);
    }

    template <class I, class F>
    inline bool processing_iterator<I,F>::operator>=(
                                          const processing_iterator<I,F>& i) {
        return (it_ >= i.it_);
    }

    template <class I, class F>
    inline processing_iterator<I,F>
    make_processing_iterator(I it, F p) {
        return processing_iterator<I,F>(it,p);
    }

}


#endif
