
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

/*! \file couplingiterator.hpp
    \brief Iterator mapping a function to a pair of underlying sequences
*/

#ifndef quantlib_coupling_iterator_h
#define quantlib_coupling_iterator_h

#include <ql/Utilities/iteratorcategories.hpp>

namespace QuantLib {

    #ifndef QL_DISABLE_DEPRECATED

    //! Iterator mapping a function to a pair of underlying sequences
    /*! This iterator advances two underlying iterators and returns
        the values obtained by applying a function to the two values
        such iterators point to.

        \deprecated use a combination of boost::zip_iterator and
                    boost::transform_iterator instead
    */
    template <class Iterator1, class Iterator2, class Function>
    class coupling_iterator : public QL_ITERATOR<
        typename lowest_category_iterator<
        typename QL_ITERATOR_TRAITS<Iterator1>::iterator_category,
        typename QL_ITERATOR_TRAITS<Iterator2>::iterator_category>::
    iterator_category,
        typename Function::result_type,
        typename QL_ITERATOR_TRAITS<Iterator1>::difference_type,
        const typename Function::result_type*,
        const typename Function::result_type&>
    {
      public:
        /* These typedefs are needed even though inherited from
           QL_ITERATOR (see 14.6.2.3 of the standard).  */
        typedef typename Function::result_type value_type;
        typedef typename QL_ITERATOR_TRAITS<Iterator1>::difference_type
        difference_type;
        typedef const typename Function::result_type* pointer;
        typedef const typename Function::result_type& reference;

        coupling_iterator(Iterator1 it1, Iterator2 it2, Function f)
        : it1_(it1), it2_(it2), f_(f) {}
        //! \name Dereferencing
        //@{
        reference operator*()  const {
            x_ = f_(*it1_,*it2_);
            return x_;
        }
        pointer   operator->() const {
            x_ = f_(*it1_,*it2_);
            return &x_;
        }
        //@}
        //! \name Random access
        //@{
        value_type operator[](difference_type n) const {
            return *(*this+n) ;
        }
        //@}
        //! \name Increment and decrement
        //@{
        coupling_iterator& operator++() {
            ++it1_; ++it2_;
            return *this;
        }
        coupling_iterator  operator++(int ) {
            coupling_iterator tmp = *this;
            ++it1_; ++it2_;
            return tmp;
        }
        coupling_iterator& operator--() {
            --it1_; --it2_;
            return *this;
        }
        coupling_iterator  operator--(int ) {
            coupling_iterator tmp = *this;
            --it1_; --it2_;
            return tmp;
        }
        coupling_iterator& operator+=(difference_type n) {
            it1_+=n; it2_+=n;
            return *this;
        }
        coupling_iterator& operator-=(difference_type n) {
            it1_-=n; it2_-=n;
            return *this;
        }
        coupling_iterator  operator+ (difference_type n) const {
            return coupling_iterator(it1_+n,it2_+n,f_);
        }
        coupling_iterator  operator- (difference_type n) const {
            return coupling_iterator(it1_-n,it2_-n,f_);
        }
        //@}
        //! \name Difference
        //@{
        difference_type operator-(const coupling_iterator& rhs) const {
            return it1_-rhs.it1_;
        }
        //@}
        //! \name Comparisons
        //@{
        bool operator==(const coupling_iterator& rhs) const {
            return it1_ == rhs.it1_;
        }
        bool operator!=(const coupling_iterator& rhs) const {
            return it1_ != rhs.it1_;
        }
        //@}
      private:
        Iterator1 it1_;
        Iterator2 it2_;
        Function f_;
        mutable value_type x_;
    };


    //! helper function to create combining iterators
    /*! \relates coupling_iterator */
    template <class It1, class It2, class Function>
    coupling_iterator<It1,It2,Function>
    make_coupling_iterator(It1 it1, It2 it2, Function f);


    // inline definitions

    template <class It1, class It2, class Function>
    inline coupling_iterator<It1,It2,Function>
    make_coupling_iterator(It1 it1, It2 it2, Function f) {
        return coupling_iterator<It1,It2,Function>(it1,it2,f);
    }

    #endif

}


#endif
