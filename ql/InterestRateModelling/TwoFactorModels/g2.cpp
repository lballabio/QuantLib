
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file g2.hpp
    \brief Two-additive-factor Gaussian Model G2++

    \fullpath
    ql/InterestRateModelling/TwoFactorModels/%g2.hpp
*/

// $Id$

#include "ql/InterestRateModelling/TwoFactorModels/g2.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        G2::G2(const RelinkableHandle<TermStructure>& termStructure) 
        : TwoFactorModel(5, termStructure), 
          a_(params_[0]), sigma_(params_[1]), b_(params_[2]), eta_(params_[3]),
          rho_(params_[4]) {
            constraint_ = Handle<Constraint>(new Constraint(5));
            constraint_->setLowerBound(1, 0.000001);
            constraint_->setLowerBound(3, 0.000001);
            constraint_->setLowerBound(4, -1.0);
            constraint_->setUpperBound(4, 1.0);
        }

        double G2::discountBondOption(Option::Type type, 
            double strike, Time maturity, Time bondMaturity) {

            double discountT = termStructure()->discount(maturity);
            double discountS = termStructure()->discount(bondMaturity);

            if (maturity < QL_EPSILON) {
                switch(type) {
                  case Option::Call: return QL_MAX(discountS - strike, 0.0);
                  case Option::Put:  return QL_MAX(strike - discountS, 0.0);
                  default: throw Error("unsupported option type");
                }
            }

            double sigma = sigmaP(maturity, bondMaturity);
            double d1 = QL_LOG(discountS/(strike*discountT))/sigma + 
                sigma/2.0;
            double d2 = d1 - sigma;
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
