
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file filteringiterator.hpp
    \brief Iterator filtering undesired data

    $Id$
*/

// $Source$
// $Log$
// Revision 1.6  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/06/22 16:38:16  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_filtering_iterator_h
#define quantlib_filtering_iterator_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

        template <class IteratorTag>
        struct filtering_iterator_tag {
            typedef IteratorTag iterator_category;
        };

        template <>
        struct filtering_iterator_tag<std::random_access_iterator_tag> {
            typedef std::bidirectional_iterator_tag iterator_category;
        };

        //! Iterator filtering undesired data
        /*! This iterator advances an underlying iterator
            returning only those data satisfying a given
            condition.
        */
        template <class Iterator, class UnaryPredicate>
        class filtering_iterator : public QL_ITERATOR<
            typename filtering_iterator_tag<
                typename QL_ITERATOR_TRAITS<Iterator>::iterator_category
                >::iterator_category,
            typename QL_ITERATOR_TRAITS<Iterator>::value_type,
            typename QL_ITERATOR_TRAITS<Iterator>::difference_type,
            typename QL_ITERATOR_TRAITS<Iterator>::pointer,
            typename QL_ITERATOR_TRAITS<Iterator>::reference>
        {
          public:
            /* These typedefs are needed even though inherited from QL_ITERATOR
               (see 14.6.2.3 of the standard).  */
            typedef typename QL_ITERATOR_TRAITS<Iterator>::pointer
                pointer;
            typedef typename QL_ITERATOR_TRAITS<Iterator>::reference
                reference;
            filtering_iterator(const Iterator&, const UnaryPredicate&,
                const Iterator& beforeBegin, const Iterator& end);
            //! \name Dereferencing
            //@{
            reference operator*()  const;
            pointer   operator->() const;
            //@}
            //! \name Increment and decrement
            //@{
            filtering_iterator& operator++();
            filtering_iterator  operator++(int);
            filtering_iterator& operator--();
            filtering_iterator  operator--(int);
            //@}
            //! \name Comparisons
            //@{
            bool operator==(const filtering_iterator<Iterator,UnaryPredicate>&);
            bool operator!=(const filtering_iterator<Iterator,UnaryPredicate>&);
            //@}
          private:
            UnaryPredicate p_;
            Iterator it_;
            Iterator beforeBegin_, end_;
        };

        //! helper function to create filtering iterators
        /*! \relates filtering_iterator */
        template <class Iterator, class UnaryPredicate>
        filtering_iterator<Iterator,UnaryPredicate>
        make_filtering_iterator(Iterator it, UnaryPredicate p,
            Iterator beforeBegin, Iterator end);



        // inline definitions

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>::filtering_iterator(
          const Iterator& it, const UnaryPredicate& p,
          const Iterator& beforeBegin, const Iterator& end)
        : p_(p), it_(it), beforeBegin_(beforeBegin), end_(end) {
            while (!p_(*it_) && it_ != end_)
                ++it_;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>&
        filtering_iterator<Iterator,UnaryPredicate>::operator++() {
            do
                ++it_;
            while (!p_(*it_) && it_ != end_);
            return *this;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>
        filtering_iterator<Iterator,UnaryPredicate>::operator++(int) {
            filtering_iterator<Iterator,UnaryPredicate> temp = *this;
            do
                ++it_;
            while (!p_(*it_) && it_ != end_);
            return temp;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>&
        filtering_iterator<Iterator,UnaryPredicate>::operator--() {
            do
                --it_;
            while (!p_(*it_) && it_ != beforeBegin_);
            return *this;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>
        filtering_iterator<Iterator,UnaryPredicate>::operator--(int) {
            filtering_iterator<Iterator,UnaryPredicate> temp = *this;
            do
                --it_;
            while (!p_(*it_) && it_ != beforeBegin_);
            return temp;
        }

        template<class Iterator, class UnaryPredicate>
        inline typename filtering_iterator<Iterator,UnaryPredicate>::reference
        filtering_iterator<Iterator,UnaryPredicate>::operator*() const {
            return *it_;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>::pointer
        filtering_iterator<Iterator,UnaryPredicate>::operator->() const {
            return it_.operator->();
        }

        template<class Iterator, class UnaryPredicate>
        inline bool filtering_iterator<Iterator,UnaryPredicate>::operator==(
          const filtering_iterator<Iterator,UnaryPredicate>& i) {
            return (it_ == i.it_);
        }

        template<class Iterator, class UnaryPredicate>
        inline bool filtering_iterator<Iterator,UnaryPredicate>::operator!=(
          const filtering_iterator<Iterator,UnaryPredicate>& i) {
            return (it_ != i.it_);
        }

        template <class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>
        make_filtering_iterator(Iterator it, UnaryPredicate p,
            Iterator beforeBegin, Iterator end) {
                return filtering_iterator<Iterator,UnaryPredicate>(
                    it,p,beforeBegin,end);
        }

    }

}


#endif
