
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

/*! \file segmentintegral.hpp
    \brief Integral of a one-dimensional function
*/

#ifndef quantlib_segment_integral_h
#define quantlib_segment_integral_h

#include <ql/types.hpp>
#include <ql/errors.hpp>

namespace QuantLib {

    //! Integral of a one-dimensional function
    /*! Given a number \f$ N \f$ of intervals, the integral of
        a function \f$ f \f$ between \f$ a \f$ and \f$ b \f$ is 
        calculated by means of the trapezoid formula
        \f[
        \int_{a}^{b} f \mathrm{d}x = 
        \frac{1}{2} f(x_{0}) + f(x_{1}) + f(x_{2}) + \dots 
        + f(x_{N-1}) + \frac{1}{2} f(x_{N})
        \f]
        where \f$ x_0 = a \f$, \f$ x_N = b \f$, and 
        \f$ x_i = a+i \Delta x \f$ with 
        \f$ \Delta x = (b-a)/N \f$.
    */
    class SegmentIntegral{
      public:
        SegmentIntegral(Size intervals);
        template <class F>
        Real operator()(const F& f, Real a, Real b) const {

            if (a == b)
                return 0.0;
            if (a > b)
                return -(*this)(f,b,a);

            Real dx = (b-a)/intervals_;
            Real sum = 0.5*(f(a)+f(b));
            Real end = b - 0.5*dx;
            for (Real x = a+dx; x < end; x += dx)
                sum += f(x);
            return sum*dx;
        }
      private:
        Size intervals_;
    };


    // inline and template definitions

    inline SegmentIntegral::SegmentIntegral(Size intervals)
    : intervals_(intervals) {
        QL_REQUIRE(intervals > 0, "at least 1 interval needed, 0 given");
    }

}


#endif
