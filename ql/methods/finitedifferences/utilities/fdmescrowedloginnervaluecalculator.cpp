/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Klaus Spanderen

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

/*! \file fdmescrowedloginnervaluecalculator.cpp
*/

#include <ql/instruments/payoffs.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/utilities/fdmescrowedloginnervaluecalculator.hpp>

namespace QuantLib {

    FdmEscrowedLogInnerValueCalculator::FdmEscrowedLogInnerValueCalculator(
        std::shared_ptr<EscrowedDividendAdjustment> escrowedDividendAdj,
        std::shared_ptr<Payoff> payoff,
        std::shared_ptr<FdmMesher> mesher,
        Size direction)
    : escrowedDividendAdj_(std::move(escrowedDividendAdj)),
      payoff_(std::move(payoff)), mesher_(std::move(mesher)),
      direction_(direction) {}

    Real FdmEscrowedLogInnerValueCalculator::innerValue(
        const FdmLinearOpIterator& iter, Time t) {

        const Real s_t = std::exp(mesher_->location(iter, direction_));
        const Real spot = s_t - escrowedDividendAdj_->dividendAdjustment(t);

        return (*payoff_)(spot);
    }

    Real FdmEscrowedLogInnerValueCalculator::avgInnerValue(
        const FdmLinearOpIterator& iter, Time t) {
        return innerValue(iter, t);
    }
}
