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
#include <ql/Instruments/dividendvanillaoption.hpp>

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
xi        Real variance = volatility_*volatility_*residualTime_;
        boost::shared_ptr<StrikedTypePayoff> payoff(
                                             new PlainVanillaPayoff(payoff_));
        // theta, rho, and dividend rho should be corrected. However,
        // the control variate machinery won't use them.
        analytic_ = boost::shared_ptr<BlackFormula>(
                       new BlackFormula(forward, discount, variance, payoff));
    }
    */

    void FDDividendEngineBase::setupArguments(const Arguments *a) const {
        const DividendVanillaOption::arguments *args =
            dynamic_cast<const DividendVanillaOption::arguments *>(a);
        QL_REQUIRE(args, "incorrect argument type");
        FDMultiPeriodEngine::setupArguments(a, args->getEventList());
    }


    // The value of the x axis is the NPV of the underlying minus the 
    // value of the paid dividends.

    // Note that to get the PDE to work, I have to scale the values
    // and not shift them.  This means that the price curve assumes
    // that the dividends are scaled with the value of the underlying.
    //

    void FDDividendEngineMerton73::setGridLimits() const {
        Real paidDividends = 0.0;
        for (Size i=0; i<events_.size(); i++) {
            if (getDividendTime(i) >= 0.0)
                paidDividends += getDiscountedDividend(i);
        }

        FDVanillaEngine::setGridLimits(
                             process_->stateVariable()->value()-paidDividends,
                             getResidualTime());
        ensureStrikeInGrid();
    }

    void FDDividendEngineMerton73::executeIntermediateStep(Size step) const{
        Real scaleFactor = getDiscountedDividend(step) / 
            center_ + 1.0;
        sMin_ *= scaleFactor;
        sMax_ *= scaleFactor;
        center_ *= scaleFactor;

        intrinsicValues_.scaleGrid(scaleFactor);
        initializeInitialCondition();
        prices_.scaleGrid(scaleFactor);
        initializeOperator();
        initializeModel();

        initializeStepCondition();
        stepCondition_ -> applyTo(prices_.values(), getDividendTime(step));
    }

    void FDDividendEngineShift::setGridLimits() const {
        Real paidDividends = 0.0;
        for (Size i=0; i<events_.size(); i++) {
            if (getDividendTime(i) >= 0.0)
                paidDividends += getDividend(i);
        }

        FDVanillaEngine::setGridLimits(
                             process_->stateVariable()->value()-paidDividends,
                             getResidualTime());
        ensureStrikeInGrid();
    }

    void FDDividendEngineShift::executeIntermediateStep(Size step) const{
        sMin_ += getDividend(step);
        sMax_ += getDividend(step);
        center_ += getDividend(step);

        intrinsicValues_.shiftGrid(getDividend(step));
        initializeInitialCondition();
        prices_.scaleGrid(getDividend(step));
        initializeOperator();
        initializeModel();

        initializeStepCondition();
        stepCondition_ -> applyTo(prices_.values(), getDividendTime(step));
    }

}
