/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004, 2005, 2007 StatPro Italia srl
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/pricingengines/vanilla/fdvanillaengine.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/exercise.hpp>
#include <ql/grid.hpp>
#include <ql/instruments/oneassetoption.hpp>
#include <ql/methods/finitedifferences/bsmoperator.hpp>
#include <ql/methods/finitedifferences/bsmtermoperator.hpp>

namespace QuantLib {

    const Real FDVanillaEngine::safetyZoneFactor_ = 1.1;

    void FDVanillaEngine::setGridLimits() const {
        setGridLimits(process_->stateVariable()->value(),
                      getResidualTime());
        ensureStrikeInGrid();
    }

    void FDVanillaEngine::setupArguments(
                                    const PricingEngine::arguments* a) const {
        const auto* args = dynamic_cast<const OneAssetOption::arguments*>(a);
        QL_REQUIRE(args, "incorrect argument type");
        exerciseDate_ = args->exercise->lastDate();
        payoff_ = args->payoff;
    }

    void FDVanillaEngine::setGridLimits(Real center, Time t) const {
        QL_REQUIRE(center > 0.0, "negative or null underlying given");
        QL_REQUIRE(t > 0.0, "negative or zero residual time");
        center_ = center;
        Size newGridPoints = safeGridPoints(gridPoints_, t);
        if (newGridPoints > intrinsicValues_.size()) {
            intrinsicValues_ = SampledCurve(newGridPoints);
        }

        Real volSqrtTime = std::sqrt(process_->blackVolatility()
                                     ->blackVariance(t, center_));

        // the prefactor fine tunes performance at small volatilities
        Real prefactor = 1.0 + 0.02/volSqrtTime;
        Real minMaxFactor = std::exp(4.0 * prefactor * volSqrtTime);
        sMin_ = center_/minMaxFactor;  // underlying grid min value
        sMax_ = center_*minMaxFactor;  // underlying grid max value
    }

    void FDVanillaEngine::ensureStrikeInGrid() const {
        // ensure strike is included in the grid
        ext::shared_ptr<StrikedTypePayoff> striked_payoff =
            ext::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);
        if (!striked_payoff)
            return;
        Real requiredGridValue = striked_payoff->strike();

        if(sMin_ > requiredGridValue/safetyZoneFactor_){
            sMin_ = requiredGridValue/safetyZoneFactor_;
            // enforce central placement of the underlying
            sMax_ = center_/(sMin_/center_);
        }
        if(sMax_ < requiredGridValue*safetyZoneFactor_){
            sMax_ = requiredGridValue*safetyZoneFactor_;
            // enforce central placement of the underlying
            sMin_ = center_/(sMax_/center_);
        }
    }

    void FDVanillaEngine::initializeInitialCondition() const {
        intrinsicValues_.setLogGrid(sMin_, sMax_);
        intrinsicValues_.sample(*payoff_);
    }

    void FDVanillaEngine::initializeOperator() const {
        if (timeDependent_) {
            finiteDifferenceOperator_ = BSMTermOperator(intrinsicValues_.grid(),
                                                        process_, getResidualTime());
        } else {
            const YieldTermStructure& R = **(process_->riskFreeRate());
            Rate r = R.zeroRate(exerciseDate_, R.dayCounter(), Continuous);
            const YieldTermStructure& Q = **(process_->dividendYield());
            Rate q = Q.zeroRate(exerciseDate_, Q.dayCounter(), Continuous);

            ext::shared_ptr<StrikedTypePayoff> striked_payoff =
                ext::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);
            Real K = striked_payoff != nullptr ? striked_payoff->strike() : process_->x0();
            Volatility sigma =
                process_->blackVolatility()->blackVol(exerciseDate_, K);

            finiteDifferenceOperator_ = BSMOperator(intrinsicValues_.grid(),
                                                    r, q, sigma);
        }
    }

    void FDVanillaEngine::initializeBoundaryConditions() const {
        BCs_[0] = ext::shared_ptr<bc_type>(new NeumannBC(
                                      intrinsicValues_.value(1)-
                                      intrinsicValues_.value(0),
                                      NeumannBC::Lower));
        BCs_[1] = ext::shared_ptr<bc_type>(new NeumannBC(
                       intrinsicValues_.value(intrinsicValues_.size()-1) -
                       intrinsicValues_.value(intrinsicValues_.size()-2),
                       NeumannBC::Upper));
    }

    Time FDVanillaEngine::getResidualTime() const {
        return process_->time(exerciseDate_);
    }

    // safety check to be sure we have enough grid points.
    Size FDVanillaEngine::safeGridPoints(Size gridPoints,
                                         Time residualTime) const {
        static const Size minGridPoints = 10;
        static const Size minGridPointsPerYear = 2;
        return std::max(gridPoints,
                        residualTime > 1.0 ?
                            ql_cast<Size>((minGridPoints +
                                               (residualTime-1.0) *
                                                minGridPointsPerYear))
                            : minGridPoints);
    }

}
