/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

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

#include <ql/models/marketmodels/products/marketmodelratchet.hpp>
#include <ql/models/marketmodels/curvestate.hpp>

namespace QuantLib {

    MarketModelRatchet::MarketModelRatchet(
                                    const std::vector<Time>& rateTimes,
                                    const std::vector<Real>& fixedAccruals,
                                    const std::vector<Real>& floatingAccruals,
                                    const std::vector<Rate>& floatingSpreads,
                                    const std::vector<Time>& paymentTimes,
                                    double initialCoupon)
    : rateTimes_(rateTimes), fixedAccruals_(fixedAccruals),
      floatingAccruals_(floatingAccruals), floatingSpreads_(floatingSpreads),
      paymentTimes_(paymentTimes), initialCoupon_(initialCoupon) {
        // data checks
        lastIndex_ = rateTimes.size()-1;
        std::vector<Time> evolutionTimes(rateTimes_.size()-1);
        std::vector<Size> numeraires(evolutionTimes.size());
        for (Size i=0; i<evolutionTimes.size(); ++i) {
            evolutionTimes[i]=rateTimes_[i];
            // MoneyMarketPlus(1)
            numeraires[i]=i+1;
        }

        std::vector<std::pair<Size,Size> > relevanceRates(
            evolutionTimes.size());
        for (Size i=0; i<evolutionTimes.size(); ++i)
            relevanceRates[i] = std::make_pair(i, i+1);

        evolution_ = EvolutionDescription(rateTimes_, evolutionTimes,
                                          relevanceRates);
    }

    const EvolutionDescription& MarketModelRatchet::evolution() const {
        return evolution_;
    }

    std::vector<Size> MarketModelRatchet::suggestedNumeraires() const {
        return std::vector<Size>(evolution_.evolutionTimes().size(),
                                 evolution_.rateTimes().size()-1);
    }

    bool MarketModelRatchet::nextTimeStep(
            const CurveState& currentState,
            std::vector<Size>& numberCashFlowsThisStep,
            std::vector<std::vector<MarketModelMultiProduct::CashFlow> >&
                                                               genCashFlows) {
        double liborRate = currentState.forwardRate(currentIndex_);
        currentCoupon_= std::max(liborRate,currentCoupon_);

        genCashFlows[0][0].timeIndex = currentIndex_;
        genCashFlows[0][0].amount =
            currentCoupon_*fixedAccruals_[currentIndex_];

        genCashFlows[0][1].timeIndex = currentIndex_;
        genCashFlows[0][1].amount = (liborRate+floatingSpreads_[currentIndex_])
                                    *floatingAccruals_[currentIndex_];

        numberCashFlowsThisStep[0] = 2;

        ++currentIndex_;

        return (currentIndex_ == lastIndex_);
    }

    std::auto_ptr<MarketModelMultiProduct> MarketModelRatchet::clone() const {
        return std::auto_ptr<MarketModelMultiProduct>(
                                               new MarketModelRatchet(*this));
    }

}
