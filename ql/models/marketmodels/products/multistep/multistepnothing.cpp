/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/models/marketmodels/products/multistep/multistepnothing.hpp>

namespace QuantLib {

    MultiStepNothing::MultiStepNothing(const EvolutionDescription& evolution,
                                       Size numberOfProducts,
                                       Size doneIndex)
    : MultiProductMultiStep(evolution.rateTimes()),
      numberOfProducts_(numberOfProducts), doneIndex_(doneIndex) {}

    bool MultiStepNothing::nextTimeStep(
        const CurveState&,
        std::vector<Size>& numberCashFlowsThisStep,
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&) {
        for (auto i = numberCashFlowsThisStep.begin(); i != numberCashFlowsThisStep.end(); ++i)
            *i = 0;
        ++currentIndex_;
        return (currentIndex_ >= doneIndex_);
    }

    std::unique_ptr<MarketModelMultiProduct>
    MultiStepNothing::clone() const {
        return std::unique_ptr<MarketModelMultiProduct>(new MultiStepNothing(*this));
    }

}
