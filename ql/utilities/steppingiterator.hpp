/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file steppingiterator.hpp
    \brief Iterator advancing in constant steps
*/

#ifndef quantlib_stepping_iterator_hpp
#define quantlib_stepping_iterator_hpp

#include <ql/types.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

namespace QuantLib {

    //! Iterator advancing in constant steps
    /*! This iterator advances an underlying random-access iterator in
        steps of \f$ n \f$ positions, where \f$ n \f$ is a positive
        integer given upon construction.
    */
    template <class Iterator>
    class step_iterator :
        public boost::iterator_adaptor<step_iterator<Iterator>, Iterator> {
      private:
        typedef boost::iterator_adaptor<step_iterator<Iterator>, Iterator>
                                                                      super_t;
        // a Size would mess up integer division in distance_to
        BigInteger step_;
      public:
        step_iterator() = default;
        explicit step_iterator(const Iterator& base, Size step)
        : super_t(base), step_(static_cast<BigInteger>(step)) {}
        template <class OtherIterator>
        step_iterator(const step_iterator<OtherIterator>& i,
                      typename boost::enable_if_convertible
                      <OtherIterator,Iterator>::type* = 0)
        : super_t(i.base()), step_(static_cast<BigInteger>(i.step())) {}
        // inspector
        Size step() const { return static_cast<Size>(this->step_); }
        // iterator adapter interface
        void increment() {
            std::advance(this->base_reference(), step_);
        }
        void decrement() {
            std::advance(this->base_reference(), -step_);
        }
        void advance(typename super_t::difference_type n) {
            this->base_reference() += n*(this->step_);
        }
        typename super_t::difference_type
        distance_to(const step_iterator& i) const {
            return (i.base()-this->base())/(this->step_);
        }
    };

    //! helper function to create step iterators
    /*! \relates step_iterator */
    template <class Iterator>
    step_iterator<Iterator> make_step_iterator(Iterator it, Size step) {
        return step_iterator<Iterator>(it,step);
    }

}


#endif


#ifndef id_2408634cccc0e1efeddfdbf8bc7c6c92
#define id_2408634cccc0e1efeddfdbf8bc7c6c92
inline bool test_2408634cccc0e1efeddfdbf8bc7c6c92(const int* i) {
    return i != nullptr;
}
#endif
