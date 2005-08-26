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
#include <ql/FiniteDifferences/bsmoperator.hpp>
#include <ql/FiniteDifferences/bsmtermoperator.hpp>

namespace QuantLib {

    void FDVanillaEngine::setGridLimits() const {
        setGridLimits(getProcess()->stateVariable()->value(),
                      getResidualTime());
    }

    Real FDVanillaEngine::centerGridValue() const {
        boost::shared_ptr<StrikedTypePayoff> payoff =
            boost::dynamic_pointer_cast<StrikedTypePayoff>(
                                                           optionArguments_->payoff);
        return payoff->strike();
    }
    void FDVanillaEngine::setGridLimits(Real center, Time t) const {
        center_ = center;
        Date exerciseDate = optionArguments_->exercise->lastDate();

        Size newGridPoints = safeGridPoints(gridPoints_, t);
        if (newGridPoints > grid_.size()) {
            grid_ = Array(newGridPoints);
            intrinsicValues_ = Array(newGridPoints);
        }

        Real volSqrtTime = std::sqrt(getProcess()->blackVolatility()
                                     ->blackVariance(t, center_));

        // the prefactor fine tunes performance at small volatilities
        Real prefactor = 1.0 + 0.02/volSqrtTime;
        Real minMaxFactor = std::exp(4.0 * prefactor * volSqrtTime);
        sMin_ = center_/minMaxFactor;  // underlying grid min value
        sMax_ = center_*minMaxFactor;  // underlying grid max value
        // insure strike is included in the grid
        Real safetyZoneFactor = 1.1;
        Real centerValue = centerGridValue();
        if(sMin_ > centerValue/safetyZoneFactor){
            sMin_ = centerValue/safetyZoneFactor;
            // enforce central placement of the underlying
            sMax_ = center_/(sMin_/center_);
        }
        if(sMax_ < centerValue*safetyZoneFactor){
            sMax_ = centerValue*safetyZoneFactor;
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
        boost::shared_ptr<Payoff> payoff =
            boost::dynamic_pointer_cast<Payoff>(optionArguments_->payoff);
        for(Size j = 0; j < grid_.size(); j++)
            intrinsicValues_[j] = (*payoff)(grid_[j]);
    }

    void FDVanillaEngine::initializeOperator() const {
        if (timeDependent_)
            finiteDifferenceOperator_ = BSMTermOperator(grid_, getProcess(),
                                                        getResidualTime());
        else
            finiteDifferenceOperator_ = BSMOperator(grid_, getProcess(),
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
        return getProcess()->time(optionArguments_->exercise->lastDate());
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
