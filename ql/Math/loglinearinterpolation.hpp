
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano

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

#include <ql/types.hpp>
#include <ql/handle.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <vector>

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
          private:
            typedef LinearInterpolation<RandomAccessIterator1,
              typename std::vector<result_type>::const_iterator>
                inner_interpolation;
          public:
            LogLinearInterpolation(const RandomAccessIterator1& xBegin,
                                   const RandomAccessIterator1& xEnd,
                                   const RandomAccessIterator2& yBegin)
            : Interpolation<RandomAccessIterator1,RandomAccessIterator2>(
		xBegin, xEnd, yBegin),
	       logY_(xEnd-xBegin) {
		 int i;
                 for (i=0; i<xEnd-xBegin; i++) {
                     QL_REQUIRE(*(yBegin+i)>0.0,
                        "LogLinearInterpolation::LogLinearInterpolation : "
                        "negative values not allowed");
                     logY_[i]=QL_LOG(*(yBegin+i));
                 }
                 linearInterpolation_ =
                     Handle<inner_interpolation>(
                     new inner_interpolation(xBegin, xEnd, logY_.begin()));
             }
            result_type operator()(const argument_type& x,
                bool allowExtrapolation = false) const;
          private:
              std::vector<result_type> logY_;
              Handle<LinearInterpolation<RandomAccessIterator1,
                  typename std::vector<result_type>::const_iterator> >
                  linearInterpolation_;
        };


        // inline definitions

        template <class I1, class I2>
        inline typename LogLinearInterpolation<I1,I2>::result_type
        LogLinearInterpolation<I1,I2>::operator()(
            const LogLinearInterpolation<I1,I2>::argument_type& x,
            bool allowExtrapolation) const {
                result_type logResult = (*linearInterpolation_)(x,
                    allowExtrapolation);
                return QL_EXP(logResult);
        }

    }

}


#endif
