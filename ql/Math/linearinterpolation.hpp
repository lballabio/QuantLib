
/*
 Copyright (C) 2000-2004 StatPro Italia srl

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

/*! \file linearinterpolation.hpp
    \brief linear interpolation between discrete points
*/

#ifndef quantlib_linear_interpolation_hpp
#define quantlib_linear_interpolation_hpp

#include <ql/Math/interpolation.hpp>
#include <vector>

namespace QuantLib {

    //! %Linear interpolation between discrete points
    class LinearInterpolation : public Interpolation {
      protected:
        template <class I1, class I2>
        class Impl : public Interpolation::templateImpl<I1,I2> {
          public:
            Impl(const I1& xBegin, const I1& xEnd, const I2& yBegin)
            : Interpolation::templateImpl<I1,I2>(xBegin,xEnd,yBegin),
              primitiveConst_(xEnd-xBegin), s_(xEnd-xBegin) {
                primitiveConst_[0] = 0.0;
                for (Size i=1; i<Size(xEnd-xBegin); i++) {
                    Real dx = xBegin_[i]-xBegin_[i-1];
                    s_[i-1] = (yBegin_[i]-yBegin_[i-1])/dx;
                    primitiveConst_[i] = primitiveConst_[i-1]
                        + dx*(yBegin_[i-1] +0.5*dx*s_[i-1]);
                }
            }
            Real value(Real x) const {
                Size i = locate(x);
                return yBegin_[i] + (x-xBegin_[i])*s_[i];
            }
            Real primitive(Real x) const {
                Size i = locate(x);
                Real dx = x-xBegin_[i];
                return primitiveConst_[i] +
                    dx*(yBegin_[i] + 0.5*dx*s_[i]);
            }
            Real derivative(Real x) const {
                Size i = locate(x);
                return s_[i];
            }
            Real secondDerivative(Real) const {
                return 0.0;
            }
          private:
            std::vector<Real> primitiveConst_, s_;
        };
      public:
        /*! \pre the \f$ x \f$ values must be sorted. */
        template <class I1, class I2>
        LinearInterpolation(const I1& xBegin, const I1& xEnd,
                            const I2& yBegin) {
            impl_ = boost::shared_ptr<Interpolation::Impl>(
                  new LinearInterpolation::Impl<I1,I2>(xBegin, xEnd, yBegin));
        }
    };

}


#endif
