

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file g2.hpp
    \brief Two-additive-factor Gaussian Model G2++

    \fullpath
    ql/InterestRateModelling/TwoFactorModels/%g2.hpp
*/

// $Id$

#ifndef quantlib_two_factor_models_g2_h
#define quantlib_two_factor_models_g2_h

#include "ql/InterestRateModelling/twofactormodel.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        class G2 : public TwoFactorModel {
          public:
            G2(const RelinkableHandle<TermStructure>& termStructure);
            virtual ~G2() {}

            virtual bool hasDiscountBondOptionFormula() { return true; }
            virtual double discountBondOption(Option::Type type,
                                              double strike,
                                              Time maturity,
                                              Time bondMaturity);

            virtual std::string name() { return "G2++"; }

          private:
            inline double sigmaP(Time t, Time s) const {
                double temp = 1.0 - QL_EXP(-(a_+b_)*t);
                double temp1 = 1.0 - QL_EXP(-a_*(s-t));
                double temp2 = 1.0 - QL_EXP(-b_*(s-t));
                double a3 = a_*a_*a_;
                double b3 = b_*b_*b_;
                double value =
                    0.5*sigma_*sigma_*temp1*temp1*(1.0 - QL_EXP(-2.0*a_*t))/a3 +
                    0.5*eta_*eta_*temp2*temp2*(1.0 - QL_EXP(-2.0*b_*t))/b3 +
                    2.0*rho_*sigma_*eta_/(a_*b_*(a_+b_))*temp1*temp2*temp;
                return QL_SQRT(value);
            }

            const double& a_;
            const double& sigma_;
            const double& b_;
            const double& eta_;
            const double& rho_;
        };

    }

}

#endif
