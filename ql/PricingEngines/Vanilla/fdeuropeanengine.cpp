
/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Vanilla/fdeuropeanengine.hpp>
#include <ql/PricingEngines/greeks.hpp>
#include <ql/FiniteDifferences/fdtypedefs.hpp>
#include <ql/FiniteDifferences/valueatcenter.hpp>

namespace QuantLib {

    void FDEuropeanEngine::calculate() const {
        setGridLimits();
        initializeGrid();
        initializeInitialCondition();
        initializeOperator();

        StandardFiniteDifferenceModel model(finiteDifferenceOperator_, BCs_);

        euroPrices_ = intrinsicValues_;

        model.rollback(euroPrices_, getResidualTime(),
                       0, timeSteps_);

        results_.value = valueAtCenter(euroPrices_);
        results_.delta = firstDerivativeAtCenter(euroPrices_, grid_);
        results_.gamma = secondDerivativeAtCenter(euroPrices_, grid_);
        results_.theta = blackScholesTheta(arguments_.stochasticProcess,
                                           results_.value,
                                           results_.delta,
                                           results_.gamma);
    }

}

