
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file steppingiterator.hpp
    \fullpath Include/ql/Utilities/%steppingiterator.hpp
    \brief Iterator advancing in constant steps

*/

// $Id$
// $Log$
// Revision 1.8  2001/08/28 12:32:17  nando
// nothing relevant (spaces added)
//
// Revision 1.7  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.6  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.5  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.4  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.3  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_stepping_iterator_h
#define quantlib_stepping_iterator_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

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
            /* These typedefs are needed even though inherited from QL_ITERATOR
               (see 14.6.2.3 of the standard).  */
            typedef typename QL_ITERATOR_TRAITS<
                RandomAccessIterator>::difference_type difference_type;
            typedef typename QL_ITERATOR_TRAITS<
                RandomAccessIterator>::pointer pointer;
            typedef typename QL_ITERATOR_TRAITS<
                RandomAccessIterator>::reference reference;

            stepping_iterator(const RandomAccessIterator&,
                difference_type step);
            //! \name Dereferencing
            //@{
            reference operator*()  const;
            pointer   operator->() const;
            //@}
            //! \name Random access
            //@{
            reference operator[](int ) const;
            //@}
            //! \name Increment and decrement
            //@{
            stepping_iterator& operator++();
            stepping_iterator  operator++(int );
            stepping_iterator& operator--();
            stepping_iterator  operator--(int );
            stepping_iterator& operator+=(difference_type);
            stepping_iterator& operator-=(difference_type);
            stepping_iterator<RandomAccessIterator> operator+(difference_type);
            stepping_iterator<RandomAccessIterator> operator-(difference_type);
            //@}
            //! \name Difference
            //@{
            difference_type operator-(
                const stepping_iterator<RandomAccessIterator>&);
            //@}
            //! \name Comparisons
            //@{
            bool operator==(const stepping_iterator<RandomAccessIterator>&);
            bool operator!=(const stepping_iterator<RandomAccessIterator>&);
            bool operator< (const stepping_iterator<RandomAccessIterator>&);
            bool operator> (const stepping_iterator<RandomAccessIterator>&);
            bool operator<=(const stepping_iterator<RandomAccessIterator>&);
            bool operator>=(const stepping_iterator<RandomAccessIterator>&);
            //@}
          private:
            difference_type dn_;
            RandomAccessIterator it_;
        };

        //! helper function to create stepping iterators
        /*! \relates stepping_iterator */
        template <class Iterator>
        stepping_iterator<Iterator>
        make_stepping_iterator(Iterator it,
            typename stepping_iterator<Iterator>::difference_type step);

        // inline definitions

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>::stepping_iterator(
          const RandomAccessIterator& it,
          stepping_iterator<RandomAccessIterator>::difference_type step)
        : dn_(step), it_(it) {}

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>&
        stepping_iterator<RandomAccessIterator>::operator++() {
            it_ += dn_;
            return *this;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>
        stepping_iterator<RandomAccessIterator>::operator++(int ) {
            stepping_iterator<RandomAccessIterator> temp = *this;
            it_ += dn_;
            return temp;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>&
        stepping_iterator<RandomAccessIterator>::operator--() {
            it_ -= dn_;
            return *this;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>
        stepping_iterator<RandomAccessIterator>::operator--(int ) {
            stepping_iterator<RandomAccessIterator> temp = *this;
            it_ -= dn_;
            return temp;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>&
        stepping_iterator<RandomAccessIterator>::operator+=(
          stepping_iterator<RandomAccessIterator>::difference_type i) {
            it_ += i*dn_;
            return *this;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>&
        stepping_iterator<RandomAccessIterator>::operator-=(
          stepping_iterator<RandomAccessIterator>::difference_type i) {
            it_ -= i*dn_;
            return *this;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>::reference
        stepping_iterator<RandomAccessIterator>::operator*() const {
            return *it_;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>::pointer
        stepping_iterator<RandomAccessIterator>::operator->() const {
            return it_;
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>::reference
        stepping_iterator<RandomAccessIterator>::operator[](int i) const {
            return it_[i*dn_];
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>
        stepping_iterator<RandomAccessIterator>::operator+(
          stepping_iterator<RandomAccessIterator>::difference_type i) {
            return stepping_iterator<RandomAccessIterator>(it_+dn_*i,dn_);
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>
        stepping_iterator<RandomAccessIterator>::operator-(
          stepping_iterator<RandomAccessIterator>::difference_type i) {
            return stepping_iterator<RandomAccessIterator>(it_-dn_*i,dn_);
        }

        template<class RandomAccessIterator>
        inline stepping_iterator<RandomAccessIterator>::difference_type
        stepping_iterator<RandomAccessIterator>::operator-(
          const stepping_iterator<RandomAccessIterator>& i) {
            #ifdef QL_DEBUG
                QL_REQUIRE((it_-i.it_)%dn_ == 0,
                  "Cannot subtract stepping iterators not reachable "
                  "from each other");
            #endif
            return (it_-i.it_)/dn_;
        }

        template<class RandomAccessIterator>
        inline bool stepping_iterator<RandomAccessIterator>::operator==(
          const stepping_iterator<RandomAccessIterator>& i) {
            return (it_ == i.it_);
        }

        template<class RandomAccessIterator>
        inline bool stepping_iterator<RandomAccessIterator>::operator!=(
          const stepping_iterator<RandomAccessIterator>& i) {
            return (it_ != i.it_);
        }

        template<class RandomAccessIterator>
        inline bool stepping_iterator<RandomAccessIterator>::operator<(
          const stepping_iterator<RandomAccessIterator>& i) {
            return (it_ < i.it_);
        }

        template<class RandomAccessIterator>
        inline bool stepping_iterator<RandomAccessIterator>::operator>(
          const stepping_iterator<RandomAccessIterator>& i) {
            return (it_ > i.it_);
        }

        template<class RandomAccessIterator>
        inline bool stepping_iterator<RandomAccessIterator>::operator<=(
          const stepping_iterator<RandomAccessIterator>& i) {
            return (it_ <= i.it_);
        }

        template<class RandomAccessIterator>
        inline bool stepping_iterator<RandomAccessIterator>::operator>=(
          const stepping_iterator<RandomAccessIterator>& i) {
            return (it_ >= i.it_);
        }

        template <class Iterator>
        inline stepping_iterator<Iterator>
        make_stepping_iterator(Iterator it,
            typename stepping_iterator<Iterator>::difference_type step) {
                return stepping_iterator<Iterator>(it,step);
        }

    }

}


#endif
