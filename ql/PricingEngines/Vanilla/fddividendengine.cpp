/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/PricingEngines/Vanilla/fddividendengine.hpp>

namespace QuantLib {

    /*
    void FDDividendEngine::initializeControlVariate() const{
        Real riskless = 0.0;
        for (Size i=0; i<dividends_.size(); i++)
            riskless += dividends_[i]*std::exp(-riskFreeRate_*dates_[i]);
        Real spot = underlying_ + addElements(dividends_) - riskless;
        DiscountFactor discount = std::exp(-riskFreeRate_*residualTime_);
        DiscountFactor qDiscount = std::exp(-dividendYield_*residualTime_);
        Real forward = spot*qDiscount/discount;
        Real variance = volatility_*volatility_*residualTime_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        // theta, rho, and dividend rho should be corrected. However,
        // the control variate machinery won't use them.
        analytic_ = boost::shared_ptr<BlackFormula>(
                       new BlackFormula(forward, discount, variance, payoff));
    }
    */

    void FDDividendEngine::setGridLimits() const {
        Real paidDividends = 0.0;
        for (Size i=0; i<events_.size(); i++) {
            if (getDividendTime(i) >= 0.0)
                paidDividends += getDividend(i);
        }
        FDVanillaEngine::setGridLimits(
                             process_->stateVariable()->value()-paidDividends,
                             getResidualTime());
    }

    void FDDividendEngine::executeIntermediateStep(Size step) const{

        Real newSMin = sMin_ + getDividend(step);
        Real newSMax = sMax_ + getDividend(step);

        FDVanillaEngine::setGridLimits(center_ + getDividend(step),
                                       getResidualTime());
        if (sMin_ < newSMin) {
            sMin_ = newSMin;
            sMax_ = center_/(sMin_/center_);
        }
        if (sMax_ > newSMax) {
            sMax_ = newSMax;
            sMin_ = center_/(sMax_/center_);
        }

        initializeInitialCondition();

        prices_.shiftGrid(getDividend(step));
        prices_.regrid(intrinsicValues_.grid(),
                       std::ptr_fun<Real,Real>(std::log));

        initializeOperator();
        initializeModel();
        initializeStepCondition();
        stepCondition_ -> applyTo(prices_.values(), getDividendTime(step));
    }

}
