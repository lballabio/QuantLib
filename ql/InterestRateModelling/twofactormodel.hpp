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
/*! \file twofactormodel.hpp
    \brief Abstract two-factor interest rate model class

    \fullpath
    ql/InterestRateModelling/%twofactormodel.hpp
*/

// $Id$

#ifndef quantlib_interest_rate_modelling_two_factor_model_h
#define quantlib_interest_rate_modelling_two_factor_model_h

#include <ql/InterestRateModelling/model.hpp>

namespace QuantLib {

    namespace InterestRateModelling {

        class TwoFactorModel : public Model {
          public:
            TwoFactorModel(
                Size nParams,
                const RelinkableHandle<TermStructure>& termStructure)
            : Model(nParams, TwoFactor, termStructure) {}
            virtual ~TwoFactorModel() {}

            virtual double discountBondOption(Option::Type type,
                                              double strike,
                                              Time maturity,
                                              Time bondMaturity) = 0;

        };

    }

}
#endif
