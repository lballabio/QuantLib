/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

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
/*! \file hullwhite.cpp
    \brief Hull & White model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%hullwhite.cpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/hullwhite.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        double HullWhite::B(Time t) const {
            if (a() == 0.0)
                return t;
            else
                return (1.0 - QL_EXP(-a()*t))/a();
        }

        double HullWhite::lnA(Time T, Time s) const {
            double discountT = termStructure()->discount(T);
            double discountS = termStructure()->discount(s);
            double forwardT = termStructure()->forward(T);
            double temp = sigma()*B(s-T);
            double value = QL_LOG(discountS/discountT) + B(s-T)*forwardT
                - 0.25*temp*temp*B(2.0*T);
            return value;
        }

        double HullWhite::discountBond(Time T, Time s, Rate r) const {
            return QL_EXP(lnA(T,s) - B(s-T)*r);
        }

        double HullWhite::discountBondOption(Option::Type type,
            double strike, Time maturity, Time bondMaturity) const {

            double discountT = termStructure()->discount(maturity);
            double discountS = termStructure()->discount(bondMaturity);

            if (maturity < QL_EPSILON) {
                switch(type) {
                  case Option::Call: return QL_MAX(discountS - strike, 0.0);
                  case Option::Put:  return QL_MAX(strike - discountS, 0.0);
                  default: throw Error("unsupported option type");
                }
            }

            double v = sigma()*B(bondMaturity - maturity)
                *QL_SQRT(0.5*B(2.0*maturity));
            double d1 = QL_LOG(discountS/(strike*discountT))/v + 0.5*v;
            double d2 = d1 - v;
            double sFactor;
            double tFactor;
            Math::CumulativeNormalDistribution f;
            switch(type) {
              case Option::Call:
                sFactor = f(d1);
                tFactor = -f(d2);
                break;

              case Option::Put:
                sFactor = -f(-d1);
                tFactor = f(-d2);
                break;

              default:
                throw Error("unsupported option type");
            }
            return discountS*sFactor + strike*discountT*tFactor;
        }

    }

}
