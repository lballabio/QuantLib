
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

/*! \file hullandwhite.hpp
    \brief Hull & White model

    \fullpath
    ql/%hullandwhite.hpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/hullandwhite.hpp"
#include "ql/Math/normaldistribution.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class HullAndWhite::Process : public StochasticProcess {
          public:
            Process(HullAndWhite * model) 
            : StochasticProcess(ShortRate), model_(model) {}

            virtual double drift(double r, double t) const {
                return model_->theta(t) - r*model_->alpha_;
            }
            virtual double diffusion(double r, Time t) const {
                return model_->sigma_;
            }
          private:
            HullAndWhite * model_;
        };

        HullAndWhite::HullAndWhite(
            const RelinkableHandle<TermStructure>& termStructure) 
        : OneFactorModel(2, termStructure), dt_(0.001) {
            process_ = Handle<StochasticProcess>(new Process(this));

            std::vector<double> lower(2), higher(2);
            lower[0] = -2.0;
            lower[1] = 0.0001;
            higher[0] = 2.0;
            higher[1] = 0.5;
            constraint_ = Handle<Constraint>(new Constraint(lower, higher));
        }

        double HullAndWhite::theta(Time t) const {
            //Implying piecewise constant forward term structure
            if (t<0.0) return 0.0;
            double forwardDerivative = 0.0;
            double forwardRate = termStructure()->forward(t);
            double theta = forwardDerivative + alpha_*forwardRate +
                0.5*sigma_*sigma_*B(2.0*t);
            return theta;
        }

        double HullAndWhite::lnA(Time T, Time s) const {
            double discountT = termStructure()->discount(T);
            double discountS = termStructure()->discount(s);
            double forwardT = termStructure()->forward(T);
            double temp = sigma_*B(s-T);
            double value = QL_LOG(discountS/discountT) + B(s-T)*forwardT
                - 0.25*temp*temp*B(2.0*T);
            return value;
        }

        double HullAndWhite::discountBond(Time T, Time s, Rate r) const {
            return QL_EXP(lnA(T,s) - B(s-T)*r);
        }

        double HullAndWhite::discountBondOption(Option::Type type, 
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

            double sigmaP = sigma_*B(bondMaturity - maturity)
                *QL_SQRT(0.5*B(2.0*maturity));
            double d1 = QL_LOG(discountS/(strike*discountT))/sigmaP + 
                sigmaP/2.0;
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
