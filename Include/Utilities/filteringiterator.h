
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

/*! \file filteringiterator.h
    \brief Iterator filtering undesired data

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.4  2001/01/11 11:02:08  nando
    added public

    Revision 1.3  2001/01/09 17:45:26  lballabio
    Typedefs changed

    Revision 1.2  2001/01/09 14:35:17  lballabio
    Fixed typo in (so far) not instantiated method

    Revision 1.1  2001/01/09 11:51:10  lballabio
    Added a couple of smart iterators

*/

#ifndef quantlib_filtering_iterator_h
#define quantlib_filtering_iterator_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

        template <class IteratorTag>
        class FilteringTag {
          public:
            typedef IteratorTag iterator_category;
        };

        template <>
        class FilteringTag<std::random_access_iterator_tag> {
          public:
            typedef std::bidirectional_iterator_tag iterator_category;
        };

        //! Iterator filtering undesired data
        /*! This iterator advances an underlying iterator
            returning only those data satisfying a given
            condition.
        */
        template <class Iterator, class UnaryPredicate>
        class FilteringIterator {
          public:
            typedef typename std::iterator_traits<Iterator>::iterator_category
                underlying_category;
            typedef FilteringTag<underlying_category>::iterator_category
                iterator_category;
            typedef typename std::iterator_traits<Iterator>::value_type
                value_type;
            typedef typename std::iterator_traits<Iterator>::difference_type
                difference_type;
            typedef typename std::iterator_traits<Iterator>::pointer
                pointer;
            typedef typename std::iterator_traits<Iterator>::reference
                reference;
            FilteringIterator(const Iterator&, const UnaryPredicate&,
                const Iterator& beforeBegin, const Iterator& end);
            //! \name Dereferencing
            //@{
            reference operator*()  const;
            pointer   operator->() const;
            //@}
            //! \name Increment and decrement
            //@{
            FilteringIterator& operator++();
            FilteringIterator  operator++(int);
            FilteringIterator& operator--();
            FilteringIterator  operator--(int);
            //@}
            //! \name Comparisons
            //@{
            bool operator==(const FilteringIterator<Iterator,UnaryPredicate>&);
            bool operator!=(const FilteringIterator<Iterator,UnaryPredicate>&);
            //@}
          private:
            UnaryPredicate p_;
            Iterator it_;
            Iterator beforeBegin_, end_;
        };


        // inline definitions

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>::FilteringIterator(
          const Iterator& it, const UnaryPredicate& p,
          const Iterator& beforeBegin, const Iterator& end)
        : p_(p), it_(it), beforeBegin_(beforeBegin), end_(end) {
            while (!p_(*it_) && it_ != end_)
                it_++;
        }

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>&
        FilteringIterator<Iterator,UnaryPredicate>::operator++() {
            do
                it_++;
            while (!p_(*it_) && it_ != end_);
            return *this;
        }

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>
        FilteringIterator<Iterator,UnaryPredicate>::operator++(int) {
            FilteringIterator<Iterator,UnaryPredicate> temp = *this;
            do
                it_++;
            while (!p_(*it_) && it_ != end_);
            return temp;
        }

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>&
        FilteringIterator<Iterator,UnaryPredicate>::operator--() {
            do
                it_--;
            while (!p_(*it_) && it_ != beforeBegin_);
            return *this;
        }

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>
        FilteringIterator<Iterator,UnaryPredicate>::operator--(int) {
            FilteringIterator<Iterator,UnaryPredicate> temp = *this;
            do
                it_--;
            while (!p_(*it_) && it_ != beforeBegin_);
            return temp;
        }

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>::reference
        FilteringIterator<Iterator,UnaryPredicate>::operator*() const {
            return *it_;
        }

        template<class Iterator, class UnaryPredicate>
        inline FilteringIterator<Iterator,UnaryPredicate>::pointer
        FilteringIterator<Iterator,UnaryPredicate>::operator->() const {
            return it_.operator->();
        }

        template<class Iterator, class UnaryPredicate>
        inline bool FilteringIterator<Iterator,UnaryPredicate>::operator==(
          const FilteringIterator<Iterator,UnaryPredicate>& i) {
            return (it_ == i.it_);
        }

        template<class Iterator, class UnaryPredicate>
        inline bool FilteringIterator<Iterator,UnaryPredicate>::operator!=(
          const FilteringIterator<Iterator,UnaryPredicate>& i) {
            return (it_ != i.it_);
        }

    }

}


#endif
