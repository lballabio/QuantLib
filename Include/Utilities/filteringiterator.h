
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
    Revision 1.9  2001/02/12 19:00:39  lballabio
    Some more work on iterators

    Revision 1.8  2001/02/12 18:34:49  lballabio
    Some work on iterators

    Revision 1.7  2001/01/25 15:11:55  lballabio
    Added helper functions to make iterators

    Revision 1.6  2001/01/23 11:08:51  lballabio
    Renamed iterators in Include\Utilities and related files

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
        struct filtering_iterator_tag {
            typedef filtering_iterator_tag iterator_category;
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
        class filtering_iterator 
        #if defined(QL_INHERIT_ITERATOR)
        : public QL_ITERATOR<
            typename filtering_iterator_tag<
                typename std::iterator_traits<Iterator>::iterator_category
                >::iterator_category,
            typename std::iterator_traits<Iterator>::value_type,
            typename std::iterator_traits<Iterator>::difference_type,
            typename std::iterator_traits<Iterator>::pointer,
            typename std::iterator_traits<Iterator>::reference>
            #endif
        {
          public:
            typedef typename filtering_iterator_tag<
                typename std::iterator_traits<Iterator>::iterator_category
                >::iterator_category iterator_category;
            typedef typename std::iterator_traits<Iterator>::value_type 
                value_type;
            typedef typename std::iterator_traits<Iterator>::difference_type
                difference_type;
            typedef typename std::iterator_traits<Iterator>::pointer 
                pointer;
            typedef typename std::iterator_traits<Iterator>::reference 
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
                it_++;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>&
        filtering_iterator<Iterator,UnaryPredicate>::operator++() {
            do
                it_++;
            while (!p_(*it_) && it_ != end_);
            return *this;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>
        filtering_iterator<Iterator,UnaryPredicate>::operator++(int) {
            filtering_iterator<Iterator,UnaryPredicate> temp = *this;
            do
                it_++;
            while (!p_(*it_) && it_ != end_);
            return temp;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>&
        filtering_iterator<Iterator,UnaryPredicate>::operator--() {
            do
                it_--;
            while (!p_(*it_) && it_ != beforeBegin_);
            return *this;
        }

        template<class Iterator, class UnaryPredicate>
        inline filtering_iterator<Iterator,UnaryPredicate>
        filtering_iterator<Iterator,UnaryPredicate>::operator--(int) {
            filtering_iterator<Iterator,UnaryPredicate> temp = *this;
            do
                it_--;
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
