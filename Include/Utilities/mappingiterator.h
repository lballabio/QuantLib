
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
    Revision 1.2  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.1  2001/01/15 17:06:20  lballabio
    Added MappingIterator class

*/

#ifndef quantlib_mapping_iterator_h
#define quantlib_mapping_iterator_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

        //! Iterator mapping a unary function to an underlying sequence
        /*! This iterator advances an underlying iterator and returns the values
            obtained by applying a unary function to the values such iterator
            points to.
        */
        template <class Iterator, class UnaryFunction>
        class MappingIterator {
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
            MappingIterator(const Iterator&, const UnaryFunction&);
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
            MappingIterator& operator++();
            MappingIterator  operator++(int);
            MappingIterator& operator--();
            MappingIterator  operator--(int);
            MappingIterator& operator+=(difference_type);
            MappingIterator& operator-=(difference_type);
            MappingIterator operator+(difference_type);
            MappingIterator operator-(difference_type);
            //@}
            //! \name Difference
            //@{
            difference_type operator-(
                const MappingIterator<Iterator,UnaryFunction>&);
            //@}
            //! \name Comparisons
            //@{
            bool operator==(const MappingIterator<Iterator,UnaryFunction>&);
            bool operator!=(const MappingIterator<Iterator,UnaryFunction>&);
            bool operator< (const MappingIterator<Iterator,UnaryFunction>&);
            bool operator> (const MappingIterator<Iterator,UnaryFunction>&);
            bool operator<=(const MappingIterator<Iterator,UnaryFunction>&);
            bool operator>=(const MappingIterator<Iterator,UnaryFunction>&);
            //@}
          private:
            Iterator it_;
            UnaryFunction f_;
            value_type x_;
        };


        // inline definitions

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>::MappingIterator(
          const Iterator& it, const UnaryFunction& f)
        : it_(it), f_(f) {
            x_ = f_(*it_);
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>&
        MappingIterator<Iterator,UnaryFunction>::operator++() {
            x_ = f_(*(++it_));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>
        MappingIterator<Iterator,UnaryFunction>::operator++(int) {
            MappingIterator<Iterator,UnaryFunction> temp = *this;
            x_ = f_(*(++it_));
            return temp;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>&
        MappingIterator<Iterator,UnaryFunction>::operator--() {
            x_ = f_(*(--it_));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>
        MappingIterator<Iterator,UnaryFunction>::operator--(int) {
            MappingIterator<Iterator,UnaryFunction> temp = *this;
            x_ = f_(*(--it_));
            return temp;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>&
        MappingIterator<Iterator,UnaryFunction>::operator+=(
          MappingIterator<Iterator,UnaryFunction>::difference_type i) {
            x_ = f_(*(it_+=i));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>&
        MappingIterator<Iterator,UnaryFunction>::operator-=(
          MappingIterator<Iterator,UnaryFunction>::difference_type i) {
            x_ = f_(*(it_-=i));
            return *this;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>::reference
        MappingIterator<Iterator,UnaryFunction>::operator*() const {
            return x_;
        }

        template <class Iterator, class UnaryFunction>
        inline const MappingIterator<Iterator,UnaryFunction>::value_type
            QL_PTR_CONST
        MappingIterator<Iterator,UnaryFunction>::operator->() const {
            return &x_;
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>::value_type
        MappingIterator<Iterator,UnaryFunction>::operator[](int i) const {
            return f_(*(it_+i));
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>
        MappingIterator<Iterator,UnaryFunction>::operator+(
          MappingIterator<Iterator,UnaryFunction>::difference_type i) {
            return MappingIterator<Iterator,UnaryFunction>(it_+i,f_);
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>
        MappingIterator<Iterator,UnaryFunction>::operator-(
          MappingIterator<Iterator,UnaryFunction>::difference_type i) {
            return MappingIterator<Iterator,UnaryFunction>(it_-i,f_);
        }

        template <class Iterator, class UnaryFunction>
        inline MappingIterator<Iterator,UnaryFunction>::difference_type
        MappingIterator<Iterator,UnaryFunction>::operator-(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_-i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool MappingIterator<Iterator,UnaryFunction>::operator==(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_ == i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool MappingIterator<Iterator,UnaryFunction>::operator!=(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_ != i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool MappingIterator<Iterator,UnaryFunction>::operator<(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_ < i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool MappingIterator<Iterator,UnaryFunction>::operator>(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_ > i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool MappingIterator<Iterator,UnaryFunction>::operator<=(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_ <= i.it_);
        }

        template <class Iterator, class UnaryFunction>
        inline bool MappingIterator<Iterator,UnaryFunction>::operator>=(
          const MappingIterator<Iterator,UnaryFunction>& i) {
            return (it_ >= i.it_);
        }

    }

}


#endif
