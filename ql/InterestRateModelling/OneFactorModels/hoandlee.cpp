
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

/*! \file hoandlee.cpp
    \brief Ho-Lee model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%hoandlee.cpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/hoandlee.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class HoAndLee::Process : public StochasticProcess {
          public:
            Process(HoAndLee * model) 
            : StochasticProcess(ShortRate), model_(model) {}

            virtual double drift(double r, double t) const {
                return model_->theta(t);
            }
            virtual double diffusion(double r, Time t) const {
                return model_->sigma_;
            }
          private:
            HoAndLee * model_;
        };

        HoAndLee::HoAndLee(const RelinkableHandle<TermStructure>& termStructure)
        : OneFactorModel(1, termStructure), dt_(0.001) {
            process_ = Handle<StochasticProcess>(new Process(this));
            constraint_ = Handle<Constraint>(new Constraint(
                  std::vector<double>(1, 0.0001),
                  std::vector<double>(1, 0.5)));
        }

        double HoAndLee::lnA(Time T, Time s) const {
            double discountT = termStructure()->discount(T);
            double discountS = termStructure()->discount(s);
            double forwardT = termStructure()->forward(T);
            double bts = B(T,s);
            double value = QL_LOG(discountS/discountT) + bts*forwardT
                - 0.5*sigma_*sigma_*T*bts*bts;
            return value;
        }

        double HoAndLee::discountBondOption(Option::Type type, double strike, Time maturity, Time bondMaturity) const {
            double discountT = termStructure()->discount(maturity);
            double discountS = termStructure()->discount(bondMaturity);
            if (maturity < QL_EPSILON) {
                switch(type) {
                    case Option::Call: return QL_MAX(discountS - strike, 0.0);
                    case Option::Put:  return QL_MAX(strike - discountS, 0.0);
                    default: throw Error("unsupported option type");
                }
            }
            double sigmaP = sigma_*(bondMaturity - maturity)*QL_SQRT(maturity);
            double d1 = QL_LOG(discountS/(strike*discountT))/sigmaP + 0.5*sigmaP;
            double d2 = d1 - sigmaP;
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
