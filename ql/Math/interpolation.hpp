
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
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

/*! \file interpolation.hpp
    \brief abstract base classes for interpolations
*/

#ifndef quantlib_interpolation_h
#define quantlib_interpolation_h

#include <ql/errors.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    //! abstract base class for 1-D interpolations
    /*! Classes derived from this class will override operator() to
        provide interpolated values from two sequences of equal length,
        representing discretized values of a variable and a function of
        the former, respectively.
    */
    template <class RandomAccessIterator1, class RandomAccessIterator2>
    class Interpolation {
      public:
        // We do not inherit from std::unary_function because we wouldn't 
        // inherit these typedefs anyway (see 14.6.2.3 of the standard)
        typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIterator1>::value_type
                                                                argument_type;
        typedef
            typename QL_ITERATOR_TRAITS<RandomAccessIterator2>::value_type
                                                                  result_type;
	    virtual ~Interpolation() {}
        Interpolation(const RandomAccessIterator1& xBegin,
                      const RandomAccessIterator1& xEnd,
                      const RandomAccessIterator2& yBegin);
        /*! This operator must be overridden to provide an implementation
            of the actual interpolation.

            \pre The sequence of values for x must have been sorted for
                 the result to make sense.
        */
        virtual result_type operator()(const argument_type& x,
                                       bool allowExtrapolation = false) 
                                                                    const = 0;
      protected:
        void locate(const argument_type& x) const;
        mutable bool isOutOfRange_;
        mutable RandomAccessIterator1 position_;
        RandomAccessIterator1 xBegin_, xEnd_;
        RandomAccessIterator2 yBegin_;
        Size n_;
    };

    // inline definitions

    template <class I1, class I2>
    inline Interpolation<I1,I2>::Interpolation(const I1& xBegin,
                                               const I1& xEnd, 
                                               const I2& yBegin)
    : isOutOfRange_(false), position_(xBegin),
      xBegin_(xBegin), xEnd_(xEnd), yBegin_(yBegin), n_(xEnd-xBegin) {
        QL_REQUIRE(n_ >= 2,
                   "not enough points to interpolate");
        I1 xi = xBegin_+1;
        for (Size i=1; i<n_; i++, xi++) {
            QL_REQUIRE(double(*xi-*(xi-1)) > 0.0,
                       "Interpolation::Interpolation : "
                       "x[i] not sorted");
        }
    }

    template <class I1, class I2>
    inline void Interpolation<I1,I2>::locate(const argument_type& x) const {
        if (x < *xBegin_) {
            isOutOfRange_ = true;
            position_ = xBegin_;
        } else if (x > *(xEnd_-1)) {
            isOutOfRange_ = true;
            position_ = xEnd_-2;
        } else {
            isOutOfRange_ = false;
            position_ = std::upper_bound(xBegin_,xEnd_-1,x)-1;
        }
    }

}


#endif
