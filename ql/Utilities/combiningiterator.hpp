
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file combiningiterator.hpp
    \brief Iterator mapping a function to a set of underlying sequences

    \fullpath
    ql/Utilities/%combiningiterator.hpp
*/

// $Id$

#ifndef quantlib_combining_iterator_h
#define quantlib_combining_iterator_h

#include <iterator>

namespace QuantLib {

    namespace Utilities {

        //! Iterator mapping a function to a set of underlying sequences
        /*! This iterator advances a set of underlying iterators and
            returns the values obtained by applying a function to the
            sets of values such iterators point to.

            This class was implemented based on Christopher Baus and Thomas
            Becker, <i>Custom Iterators for the STL</i>, included in the
            proceedings of the First Workshop on C++ Template Programming,
            Erfurt, Germany, 2000 (http://www.oonumerics.org/tmpw00/)
        */
        template <class Iterator, class Function>
        class combining_iterator : public QL_ITERATOR<
            typename QL_ITERATOR_TRAITS<Iterator>::iterator_category,
            typename Function::result_type,
            typename QL_ITERATOR_TRAITS<Iterator>::difference_type,
            const typename Function::result_type*,
            const typename Function::result_type&>
        {
          public:
            /* These typedefs are needed even though inherited from 
               QL_ITERATOR (see 14.6.2.3 of the standard).  */
            typedef typename Function::result_type value_type;
            typedef typename QL_ITERATOR_TRAITS<Iterator>::difference_type
                difference_type;
            typedef const typename Function::result_type* pointer;
            typedef const typename Function::result_type& reference;
            // construct a combining iterator from a collection of iterators
            template <class IteratorCollectionIterator>
            combining_iterator(IteratorCollectionIterator it1,
                IteratorCollectionIterator it2, Function f)
            : iteratorVector_(it1,it2), f_(f) {}
            //! \name Dereferencing
            //@{
            reference operator*()  const;
            pointer   operator->() const;
            //@}
            //! \name Random access
            //@{
            value_type operator[](difference_type n) const;
            //@}
            //! \name Increment and decrement
            //@{
            combining_iterator& operator++();
            combining_iterator  operator++(int );
            combining_iterator& operator--();
            combining_iterator  operator--(int );
            combining_iterator& operator+=(difference_type n);
            combining_iterator& operator-=(difference_type n);
            combining_iterator  operator+ (difference_type n) const;
            combining_iterator  operator- (difference_type n) const;
            //@}
            //! \name Difference
            //@{
            difference_type operator-(
                const combining_iterator<Iterator,Function>& rhs ) const;
            //@}
            //! \name Comparisons
            //@{
            bool operator==(
                const combining_iterator<Iterator,Function>& rhs) const;
            bool operator !=(
                const combining_iterator<Iterator,Function>& rhs) const;
            //@}
          private:
            std::vector<Iterator> iteratorVector_;
            Function f_;
            mutable value_type x_;
        };


        //! helper function to create combining iterators
        /*! \relates combining_iterator */
        template <class It, class Function>
        combining_iterator<typename QL_ITERATOR_TRAITS<It>::value_type,
            Function>
        make_combining_iterator(It it1, It it2, Function f);


        // inline definitions

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>&
        combining_iterator<Iterator,Function>::operator++() {
            std::vector<Iterator>::iterator it = iteratorVector_.begin();
            while (it != iteratorVector_.end()) { ++*it; ++it; }
            return *this;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>
        combining_iterator<Iterator,Function>::operator++(int ) {
            combining_iterator<Iterator,Function> tmp = *this;
            ++*this;
            return tmp;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>&
        combining_iterator<Iterator,Function>::operator--() {
            std::vector<Iterator>::iterator it = iteratorVector_.begin();
            while (it != iteratorVector_.end()){ --*it; ++it; }
            return *this;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>
        combining_iterator<Iterator,Function>::operator--(int ) {
            combining_iterator<Iterator,Function> temp = *this;
            --*this;
            return temp;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>&
        combining_iterator<Iterator,Function>::operator+=(
         combining_iterator<Iterator,Function>::difference_type n) {
            std::vector<Iterator>::iterator it = iteratorVector_.begin();
            while ( it != iteratorVector_.end()){ *it +=n ; ++it ; }
            return *this;
        }
        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>&
        combining_iterator<Iterator,Function>::operator-=(
         combining_iterator<Iterator,Function>::difference_type n) {
            std::vector<Iterator>::iterator it = iteratorVector_.begin();
            while ( it != iteratorVector_.end()){ *it -=n ; ++it ; }
            return *this;
        }


        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>::reference
        combining_iterator<Iterator,Function>::operator*() const {
            x_ = f_(iteratorVector_.begin(), iteratorVector_.end());
            return x_;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>::pointer
        combining_iterator<Iterator,Function>::operator->() const {
            x_ = f_(iteratorVector_.begin(), iteratorVector_.end());
            return &x_;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>::value_type
        combining_iterator<Iterator,Function>::operator[](
            difference_type n) const {
                return *(*this+n) ;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>
        combining_iterator<Iterator,Function>::operator+(
            combining_iterator<Iterator,Function>::difference_type n) const {
                combining_iterator<Iterator,Function> tmp(*this);
                tmp += n;
                return tmp;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>
        combining_iterator<Iterator,Function>::operator-(
            combining_iterator<Iterator,Function>::difference_type n) const {
                combining_iterator<Iterator,Function> tmp(*this);
                tmp -= n;
                return tmp;
        }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>::difference_type
        combining_iterator<Iterator,Function>::operator-(
            const combining_iterator<Iterator,Function>& rhs) const {
                if (iteratorVector_.size()>0 && rhs.iteratorVector_.size()>0)
                    return iteratorVector_[0] - rhs.iteratorVector_[0];
                else
                    return 0;
        }

        template <class Iterator, class Function>
        inline bool combining_iterator<Iterator,Function>::operator==(
            const combining_iterator<Iterator,Function>& rhs) const {
                return iteratorVector_ == rhs.iteratorVector_;
        }

        template <class Iterator, class Function>
        inline bool combining_iterator<Iterator,Function>::operator!=(
            const combining_iterator<Iterator,Function>& rhs) const {
                return iteratorVector_ != rhs.iteratorVector_;
        }


        template <class It, class Function>
        inline combining_iterator<
            typename QL_ITERATOR_TRAITS<It>::value_type, Function>
        make_combining_iterator(It it1, It it2, Function f) {
            typedef QL_ITERATOR_TRAITS<It>::value_type Iterator;
            return combining_iterator<Iterator,Function>(it1,it2,f);
        }

    }

}


#endif
