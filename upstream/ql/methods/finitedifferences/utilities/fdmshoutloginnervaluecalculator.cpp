/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file fdmshoutinnervaluecalculator.cpp
    \brief inner value for a shout option
*/

#include <ql/instruments/payoffs.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdmshoutloginnervaluecalculator.hpp>
#include <ql/pricingengines/blackformula.hpp>
#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>

#include <utility>

namespace QuantLib {

    FdmShoutLogInnerValueCalculator::FdmShoutLogInnerValueCalculator(
        Handle<BlackVolTermStructure> blackVolatility,
        ext::shared_ptr<EscrowedDividendAdjustment> escrowedDividendAdj,
        Time maturity,
        ext::shared_ptr<PlainVanillaPayoff> payoff,
        ext::shared_ptr<FdmMesher> mesher,
        Size direction)
    : blackVolatility_(std::move(blackVolatility)),
      escrowedDividendAdj_(std::move(escrowedDividendAdj)),
      maturity_(maturity), payoff_(std::move(payoff)),
      mesher_(std::move(mesher)), direction_(direction) {}


    Real FdmShoutLogInnerValueCalculator::innerValue(
        const FdmLinearOpIterator& iter, Time t) {

        const Real s_t = std::exp(mesher_->location(iter, direction_));

        const DiscountFactor qf =
            escrowedDividendAdj_->dividendYield()->discount(maturity_)/
            escrowedDividendAdj_->dividendYield()->discount(t);

        const DiscountFactor df =
            escrowedDividendAdj_->riskFreeRate()->discount(maturity_)/
            escrowedDividendAdj_->riskFreeRate()->discount(t);

        const Real fwd = s_t*qf/df;
        const Volatility stdDev = blackVolatility_->blackForwardVol(
            t, maturity_, s_t)*std::sqrt(maturity_-t);

        const Real npv = blackFormula(
            payoff_->optionType(), s_t, fwd, stdDev, df);

        const Real spot = s_t - escrowedDividendAdj_->dividendAdjustment(t);

        const Real intrinsic = (payoff_->optionType() == Option::Call)
            ? spot - payoff_->strike() : payoff_->strike() - spot;

        return std::max(0.0, npv + intrinsic*df);
    }

    Real FdmShoutLogInnerValueCalculator::avgInnerValue(
        const FdmLinearOpIterator& iter, Time t) {
        return innerValue(iter, t);
    }
}
