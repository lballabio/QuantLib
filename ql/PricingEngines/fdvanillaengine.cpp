
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file fdvanillaengine.cpp
    \brief vanilla option engine using finite differences
*/

#include <ql/PricingEngines/vanillaengines.hpp>

namespace QuantLib {

    namespace PricingEngines {

        void FDVanillaEngine::calculate() const {

            results_.value = 0.0;
            results_.delta = 0.0;
            results_.gamma       = 0.0;
            results_.theta       = 0.0;
            results_.rho         = 0.0;
            results_.dividendRho = 0.0;
            results_.vega        = 0.0;

        }

    }

}

