
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

/*! \file combiningiterator.h
    \brief Iterator mapping a function to a set of underlying sequences

    $Source$
    $Name$
    $Log$
    Revision 1.5  2001/02/12 19:00:39  lballabio
    Some more work on iterators

    Revision 1.4  2001/02/12 18:34:49  lballabio
    Some work on iterators

    Revision 1.3  2001/02/09 19:15:52  lballabio
    removed QL_PTR_CONST macro

    Revision 1.2  2001/01/25 15:11:54  lballabio
    Added helper functions to make iterators

    Revision 1.1  2001/01/24 14:56:48  aleppo
    Added iterator combining-iterator


*/

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
        class combining_iterator
        #if defined(QL_INHERIT_ITERATOR)
         : public QL_ITERATOR<
            typename std::iterator_traits<Iterator>::iterator_category,
            typename Function::result_type,
            typename std::iterator_traits<Iterator>::difference_type,
            const typename Function::result_type*,
            const typename Function::result_type&>
        #endif
        {
          public:
            typedef typename std::iterator_traits<Iterator>::iterator_category 
                iterator_category;
            typedef typename Function::result_type value_type;
            typedef typename std::iterator_traits<Iterator>::difference_type
                difference_type;
            typedef const typename Function::result_type* pointer;
            typedef const typename Function::result_type& reference;
            // construct a combining iterator from a collection of iterators
            template <class IteratorCollectionIterator>
            combining_iterator(IteratorCollectionIterator it1, 
                IteratorCollectionIterator it2, Function func)
            : iteratorVector_(it1,it2), f_(func) {}
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
            combining_iterator  operator++(int);
            combining_iterator& operator--();
            combining_iterator  operator--(int);
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
                const combining_iterator<Iterator,Function>& rhs) const {
                    return iteratorVector_ == rhs.iteratorVector_; }
            bool operator !=(
                const combining_iterator<Iterator,Function>& rhs) const {
                    return iteratorVector_ != rhs.iteratorVector_; }
            //@}
          private:
            std::vector<Iterator> iteratorVector_;
            Function f_;
            mutable value_type x_;
        };


        //! helper function to create combining iterators
        /*! \relates combining_iterator */
        template <class It, class Function>
        combining_iterator<typename std::iterator_traits<It>::value_type, 
            Function>
        make_combining_iterator(It it1, It it2, Function func);
            

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
        combining_iterator<Iterator,Function>::operator++(int) {
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
        combining_iterator<Iterator,Function>::operator--(int) {
            combining_iterator<Iterator,Function> temp = *this;
            --*this;
            return tmp;
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
        combining_iterator<Iterator, Function>::operator+(
          combining_iterator<Iterator, Function>::difference_type n) const 
          {
            combining_iterator<Iterator,Function> Tmp(*this);
            Tmp += n;
            return Tmp;
          }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>
        combining_iterator<Iterator, Function>::operator-(
          combining_iterator<Iterator, Function>::difference_type n) const 
          {
            combining_iterator<Iterator,Function> Tmp(*this);
            Tmp -= n;
            return Tmp;
          }

        template <class Iterator, class Function>
        inline combining_iterator<Iterator,Function>::difference_type 
        combining_iterator<Iterator, Function>::operator-(
          const combining_iterator<Iterator, Function>& rhs) const {
            if( 0 < iteratorVector_.size() && 0 < rhs.iteratorVector_())
                return iteratorVector_[0] - rhs.iteratorVector_[0];
            else 
                return 0;     
            
        }

        template <class It, class Function>
        inline combining_iterator<
            typename std::iterator_traits<It>::value_type, Function>
        make_combining_iterator(It it1, It it2, Function func) {
            typedef std::iterator_traits<It>::value_type Iterator;
            return combining_iterator<Iterator,Function>(it1,it2,func);
        }
        
    }

}


#endif
