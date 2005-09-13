/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002-2005 StatPro Italia srl
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

#include <ql/PricingEngines/Vanilla/fdvanillaengine.hpp>
#include <ql/Instruments/payoffs.hpp>
#include <ql/FiniteDifferences/bsmoperator.hpp>
#include <ql/FiniteDifferences/bsmtermoperator.hpp>

namespace QuantLib {
    const Real FDVanillaEngine::safetyZoneFactor_ = 1.1;

    void FDVanillaEngine::setGridLimits() const {
        setGridLimits(process_->stateVariable()->value(),
                      getResidualTime());
    }
    void FDVanillaEngine::setupArguments(const OneAssetOption::arguments*args) const {
        process_ =
            boost::dynamic_pointer_cast<BlackScholesProcess>(args->stochasticProcess);
        QL_REQUIRE(process_, "Black-Scholes process required");
        exerciseDate_ = args->exercise->lastDate();
        payoff_ = args->payoff;
        requiredGridValue_ =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(payoff_)->strike();
    }

    void FDVanillaEngine::setGridLimits(Real center, Time t) const {
        center_ = center;
        Size newGridPoints = safeGridPoints(gridPoints_, t);
        if (newGridPoints > grid_.size()) {
            grid_ = Array(newGridPoints);
            intrinsicValues_ = Array(newGridPoints);
        }

        Real volSqrtTime = std::sqrt(process_->blackVolatility()
                                     ->blackVariance(t, center_));

        // the prefactor fine tunes performance at small volatilities
        Real prefactor = 1.0 + 0.02/volSqrtTime;
        Real minMaxFactor = std::exp(4.0 * prefactor * volSqrtTime);
        sMin_ = center_/minMaxFactor;  // underlying grid min value
        sMax_ = center_*minMaxFactor;  // underlying grid max value
        insureStrikeInGrid();
    }

    void FDVanillaEngine::insureStrikeInGrid() const {
        // insure strike is included in the grid
        boost::shared_ptr<StrikedTypePayoff> striked_payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);
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

    void FDVanillaEngine::initializeGrid() const {
        gridLogSpacing_ = (std::log(sMax_)-std::log(sMin_))/(grid_.size()-1);
        Real edx = std::exp(gridLogSpacing_);
        grid_[0] = sMin_;
        for (Size j=1; j<grid_.size(); j++)
            grid_[j] = grid_[j-1]*edx;
    }

    void FDVanillaEngine::initializeInitialCondition() const {
        for(Size j = 0; j < grid_.size(); j++)
            intrinsicValues_[j] = (*payoff_)(grid_[j]);
    }

    void FDVanillaEngine::initializeOperator() const {
        if (timeDependent_)
            finiteDifferenceOperator_ = BSMTermOperator(grid_, process_,
                                                        getResidualTime());
        else
            finiteDifferenceOperator_ = BSMOperator(grid_, process_,
                                                    getResidualTime());

        BCs_[0] = boost::shared_ptr<bc_type>(new NeumannBC(
                                      intrinsicValues_[1]-intrinsicValues_[0],
                                      NeumannBC::Lower));
        BCs_[1] = boost::shared_ptr<bc_type>(new NeumannBC(
                                      intrinsicValues_[grid_.size()-1] -
                                      intrinsicValues_[grid_.size()-2],
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
                            static_cast<Size>((minGridPoints +
                                               (residualTime-1.0) *
                                                minGridPointsPerYear))
                            : minGridPoints);
    }

}
