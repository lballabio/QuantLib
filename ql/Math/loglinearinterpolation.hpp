
/*
 Copyright (C) 2002 Andre Louw.

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file loglinearinterpolation.hpp
    \brief log-linear interpolation between discrete points

    \fullpath
    ql/Math/%loglinearinterpolation.hpp
*/

// $Id$

#ifndef quantlib_loglinear_interpolation_h
#define quantlib_loglinear_interpolation_h

#include <ql/Math/interpolation.hpp>
#include <algorithm>

namespace QuantLib {

    namespace Math {

        //! log linear interpolation between discrete points
        template <class RandomAccessIterator1, class RandomAccessIterator2>
        class LogLinearInterpolation
        : public Interpolation<RandomAccessIterator1,RandomAccessIterator2> {
          public:
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIterator1>::value_type
                argument_type;
            typedef
              typename QL_ITERATOR_TRAITS<RandomAccessIterator2>::value_type
                result_type;
             LogLinearInterpolation(const RandomAccessIterator1& xBegin,
                                    const RandomAccessIterator1& xEnd,
                                    const RandomAccessIterator2& yBegin)
             : Interpolation<RandomAccessIterator1,RandomAccessIterator2>(
                 xBegin,xEnd,yBegin) {}
            result_type operator()(const argument_type& x,
                bool allowExtrapolation = false) const;
        };


        // inline definitions

        template <class I1, class I2>
        inline LogLinearInterpolation<I1,I2>::result_type
        LogLinearInterpolation<I1,I2>::operator()(
            const LogLinearInterpolation<I1,I2>::argument_type& x,
            bool allowExtrapolation) const {
                locate(x);
                if (isOutOfRange_) {
                    QL_REQUIRE(allowExtrapolation,
                        "LogLinearInterpolation::operator() : "
                        "extrapolation not allowed");
                }
                I2 j = yBegin_+(position_-xBegin_);
                return (QL_POW(*j,(x/(*position_)*((*(position_+1))-x)/
                                   ((*(position_+1))-(*position_))))) * 
                    (QL_POW(*(j+1),(x/(*(position_+1))*(x-(*position_))/
                                    ((*(position_+1))-(*position_)))));
        }

    }

}


#endif
