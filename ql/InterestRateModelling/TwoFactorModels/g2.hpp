
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
