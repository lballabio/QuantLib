
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

/*! \file steppingiterator.h
    \brief Iterator advancing in constant steps

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/01/25 15:11:55  lballabio
    Added helper functions to make iterators

    Revision 1.4  2001/01/23 11:08:51  lballabio
    Renamed iterators in Include\Utilities and related files

    Revision 1.3  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.2  2001/01/11 12:20:26  lballabio
    Fixed constructor calls with wrong arguments

    Revision 1.1  2001/01/11 11:43:52  lballabio
    Renamed StepIterator to stepping_iterator

    Revision 1.1  2001/01/09 11:51:10  lballabio
    Added a couple of smart iterators

*/

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
        class stepping_iterator {
          public:
            typedef std::iterator_traits<RandomAccessIterator> traits;
            typedef std::random_access_iterator_tag   iterator_category;
            typedef typename traits::value_type       value_type;
            typedef typename traits::difference_type  difference_type;
            typedef typename traits::pointer          pointer;
            typedef typename traits::reference        reference;
            stepping_iterator(const RandomAccessIterator&, 
                difference_type step);
            //! \name Dereferencing
            //@{
            reference operator*()  const;
            pointer   operator->() const;
            //@}
            //! \name Random access
            //@{
            reference operator[](int) const;
            //@}
            //! \name Increment and decrement
            //@{
            stepping_iterator& operator++();
            stepping_iterator  operator++(int);
            stepping_iterator& operator--();
            stepping_iterator  operator--(int);
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
        stepping_iterator<RandomAccessIterator>::operator++(int) {
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
        stepping_iterator<RandomAccessIterator>::operator--(int) {
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
                  "from each other")
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
