
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file processingiterator.h
    \brief Iterator mapping a unary function to an underlying sequence

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/23 11:08:51  lballabio
    Renamed iterators in Include\Utilities and related files

*/

#ifndef quantlib_processing_iterator_h
#define quantlib_processing_iterator_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

        //! Iterator mapping a unary function to an underlying sequence
        /*! This iterator advances an underlying iterator and returns the values
            obtained by applying a unary function to the values such iterator
            points to.
            
            This class was implemented based on Christopher Baus and Thomas 
            Becker, <i>Custom Iterators for the STL</i>, included in the 
            proceedings of the First Workshop on C++ Template Programming, 
            Erfurt, Germany, 2000 (http://www.oonumerics.org/tmpw00/)
        */
        template <class Iterator, class UnaryFunction>
        class processing_iterator {
          public:
            typedef typename std::iterator_traits<Iterator>::iterator_category
                iterator_category;
            typedef typename UnaryFunction::result_type
                value_type;
            typedef typename std::iterator_traits<Iterator>::difference_type
                difference_type;
            typedef const value_type QL_PTR_CONST pointer;
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
            value_type operator[](int) const;
            //@}
            //! \name Increment and decrement
            //@{
            processing_iterator& operator++();
            processing_iterator  operator++(int);
            processing_iterator& operator--();
            processing_iterator  operator--(int);
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
            value_type x_;
        };


        // inline definitions

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>::processing_iterator(
          const Iterator& it, const UnaryFunction& f)
        : it_(it), f_(f) {
            x_ = f_(*it_);
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>&
        processing_iterator<Iterator,UnaryFunction>::operator++() {
            x_ = f_(*(++it_));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>
        processing_iterator<Iterator,UnaryFunction>::operator++(int) {
            processing_iterator<Iterator,UnaryFunction> temp = *this;
            x_ = f_(*(++it_));
            return temp;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>&
        processing_iterator<Iterator,UnaryFunction>::operator--() {
            x_ = f_(*(--it_));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>
        processing_iterator<Iterator,UnaryFunction>::operator--(int) {
            processing_iterator<Iterator,UnaryFunction> temp = *this;
            x_ = f_(*(--it_));
            return temp;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>&
        processing_iterator<Iterator,UnaryFunction>::operator+=(
          processing_iterator<Iterator,UnaryFunction>::difference_type i) {
            x_ = f_(*(it_+=i));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>&
        processing_iterator<Iterator,UnaryFunction>::operator-=(
          processing_iterator<Iterator,UnaryFunction>::difference_type i) {
            x_ = f_(*(it_-=i));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>::reference
        processing_iterator<Iterator,UnaryFunction>::operator*() const {
            return x_;
        }

        template <class Iterator, class UnaryFunction>
        inline const processing_iterator<Iterator,UnaryFunction>::value_type
            QL_PTR_CONST 
            processing_iterator<Iterator,UnaryFunction>::operator->() 
            const {
                return &x_;
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>::value_type
        processing_iterator<Iterator,UnaryFunction>::operator[](int i) const {
            return f_(*(it_+i));
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>
        processing_iterator<Iterator,UnaryFunction>::operator+(
          processing_iterator<Iterator,UnaryFunction>::difference_type i) {
            return processing_iterator<Iterator,UnaryFunction>(it_+i,f_);
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>
        processing_iterator<Iterator,UnaryFunction>::operator-(
          processing_iterator<Iterator,UnaryFunction>::difference_type i) {
            return processing_iterator<Iterator,UnaryFunction>(it_-i,f_);
        }

        template <class Iterator, class UnaryFunction>
        inline processing_iterator<Iterator,UnaryFunction>::difference_type
        processing_iterator<Iterator,UnaryFunction>::operator-(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_-i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool processing_iterator<Iterator,UnaryFunction>::operator==(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_ == i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool processing_iterator<Iterator,UnaryFunction>::operator!=(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_ != i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool processing_iterator<Iterator,UnaryFunction>::operator<(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_ < i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool processing_iterator<Iterator,UnaryFunction>::operator>(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_ > i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool processing_iterator<Iterator,UnaryFunction>::operator<=(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_ <= i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool processing_iterator<Iterator,UnaryFunction>::operator>=(
          const processing_iterator<Iterator,UnaryFunction>& i) {
            return (it_ >= i.it_);
        }

    }

}


#endif
