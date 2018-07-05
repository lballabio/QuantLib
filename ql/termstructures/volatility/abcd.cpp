/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2007 Ferdinando Ametrano
 Copyright (C) 2006 Cristina Duminuco
 Copyright (C) 2005, 2006 Klaus Spanderen
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/termstructures/volatility/abcd.hpp>
#include <ql/math/comparison.hpp>
#include <algorithm>

namespace QuantLib {

    AbcdFunction::AbcdFunction(Real a, Real b, Real c, Real d)
    : AbcdMathFunction(a, b, c, d) {}

    Real AbcdFunction::volatility(Time tMin, Time tMax, Time T) const {
        if (tMax==tMin)
            return instantaneousVolatility(tMax, T);
        QL_REQUIRE(tMax>tMin, "tMax must be > tMin");
        return std::sqrt(variance(tMin, tMax, T)/(tMax-tMin));
    }

    Real AbcdFunction::variance(Time tMin, Time tMax, Time T) const {
        return covariance(tMin, tMax, T, T);
    }

    Real AbcdFunction::covariance(Time t, Time T, Time S) const {
        return (*this)(T-t) * (*this)(S-t);
    }

    Real AbcdFunction::covariance(Time t1, Time t2, Time T, Time S) const {
        QL_REQUIRE(t1<=t2,
                   "integrations bounds (" << t1 <<
                   "," << t2 << ") are in reverse order");
        Time cutOff = std::min(S,T);
        if (t1>=cutOff) {
            return 0.0;
        } else {
            cutOff = std::min(t2, cutOff);
            return primitive(cutOff, T, S) - primitive(t1, T, S);
        }
    }

    // INSTANTANEOUS
    Real AbcdFunction::instantaneousVolatility(Time u, Time T) const {
        return std::sqrt(instantaneousVariance(u, T));
    }

    Real AbcdFunction::instantaneousVariance(Time u, Time T) const {
        return instantaneousCovariance(u, T, T);
    }
    Real AbcdFunction::instantaneousCovariance(Time u, Time T, Time S) const {
        return (*this)(T-u)*(*this)(S-u);
    }

    // PRIMITIVE
    Real AbcdFunction::primitive(Time t, Time T, Time S) const {
        if (T<t || S<t) return 0.0;

        if (close(c_,0.0)) {
            Real v = a_+d_;
            return t*(v*v+v*b_*S+v*b_*T-v*b_*t+b_*b_*S*T-0.5*b_*b_*t*(S+T)+b_*b_*t*t/3.0);
        }

        Real k1=std::exp(c_*t), k2=std::exp(c_*S), k3=std::exp(c_*T);

        return (b_*b_*(-1 - 2*c_*c_*S*T - c_*(S + T)
                     + k1*k1*(1 + c_*(S + T - 2*t) + 2*c_*c_*(S - t)*(T - t)))
                + 2*c_*c_*(2*d_*a_*(k2 + k3)*(k1 - 1)
                         +a_*a_*(k1*k1 - 1)+2*c_*d_*d_*k2*k3*t)
                + 2*b_*c_*(a_*(-1 - c_*(S + T) + k1*k1*(1 + c_*(S + T - 2*t)))
                         -2*d_*(k3*(1 + c_*S) + k2*(1 + c_*T)
                               - k1*k3*(1 + c_*(S - t))
                               - k1*k2*(1 + c_*(T - t)))
                         )
                ) / (4*c_*c_*c_*k2*k3);
    }

//===========================================================================//
//                               AbcdSquared                                //
//===========================================================================//

    AbcdSquared::AbcdSquared(Real a, Real b, Real c, Real d, Time T, Time S)
    : abcd_(new AbcdFunction(a,b,c,d)),
      T_(T), S_(S) {}

    Real AbcdSquared::operator()(Time t) const {
        return abcd_->covariance(t, T_, S_);
    }
}
