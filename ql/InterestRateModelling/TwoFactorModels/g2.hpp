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

            virtual bool hasDiscountBondOptionFormula() const { return true; }
            virtual double discountBondOption(Option::Type type,
                                              double strike,
                                              Time maturity,
                                              Time bondMaturity) const;

          private:
            inline double sigmaP(Time t, Time s) const {
                double temp = 1.0 - QL_EXP(-(a()+b())*t);
                double temp1 = 1.0 - QL_EXP(-a()*(s-t));
                double temp2 = 1.0 - QL_EXP(-b()*(s-t));
                double a3 = a()*a()*a();
                double b3 = b()*b()*b();
                double sigma2 = sigma()*sigma();
                double eta2 = eta()*eta();
                double value =
                    0.5*sigma2*temp1*temp1*(1.0 - QL_EXP(-2.0*a()*t))/a3 +
                    0.5*eta2*temp2*temp2*(1.0 - QL_EXP(-2.0*b()*t))/b3 +
                    2.0*rho()*sigma()*eta()/(a()*b()*(a()+b()))*
                        temp1*temp2*temp;
                return QL_SQRT(value);
            }

            Parameter& a_;
            Parameter& sigma_;
            Parameter& b_;
            Parameter& eta_;
            Parameter& rho_;

            double a() const { return a_(0.0); }
            double sigma() const { return sigma_(0.0); }
            double b() const { return b_(0.0); }
            double eta() const { return eta_(0.0); }
            double rho() const { return rho_(0.0); }

            class OwnConstraint;
        };

    }

}

#endif
