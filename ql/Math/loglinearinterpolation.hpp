
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2004 StatPro Italia srl

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

/*! \file loglinearinterpolation.hpp
    \brief log-linear interpolation between discrete points
*/

#ifndef quantlib_loglinear_interpolation_hpp
#define quantlib_loglinear_interpolation_hpp

#include <ql/Math/linearinterpolation.hpp>
#include <vector>

namespace QuantLib {

    /*! %Log-linear interpolation between discrete points

        \todo Implement primitive, derivative, and secondDerivative functions.
    */
    class LogLinearInterpolation : public Interpolation {
      protected:
        template <class I1, class I2>
        class Impl : public Interpolation::templateImpl<I1,I2> {
          public:
            Impl(const I1& xBegin, const I1& xEnd, const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin,xEnd,yBegin),
              logY_(xEnd-xBegin) {
                for (Size i=0; i<logY_.size(); i++) {
                    QL_REQUIRE(yBegin[i]>0.0,
                               "negative values not allowed");
                    logY_[i]=QL_LOG(yBegin[i]);
                }
                linearInterpolation_ = LinearInterpolation(xBegin, xEnd, 
                                                           logY_.begin());
            }
            Real value(Real x) const {
                return QL_EXP(linearInterpolation_(x,true));
            }
            Real primitive(Real) const {
                QL_FAIL("not implemented");
            }
            Real derivative(Real) const {
                QL_FAIL("not implemented");
            }
            Real secondDerivative(Real) const {
                QL_FAIL("not implemented");
            }
          private:
            std::vector<Real> logY_;
            Interpolation linearInterpolation_;
        };
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LogLinearInterpolation(const I1& xBegin, const I1& xEnd,
                               const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                  new LogLinearInterpolation::Impl<I1,I2>(xBegin, xEnd,
                                                          yBegin));
        }
    };

}


#endif
