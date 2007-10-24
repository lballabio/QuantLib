/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti

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

#include <ql/voltermstructures/interestrate/optionlet/optionletstripper2.hpp>
#include <ql/voltermstructures/interestrate/optionlet/optionletstripper1.hpp>
#include <ql/voltermstructures/interestrate/optionlet/optionletstripperadapter.hpp>
#include <ql/voltermstructures/interestrate/optionlet/spreadedoptionletvol.hpp>
#include <ql/voltermstructures/interestrate/capfloor/capfloortermvolcurve.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/indexes/iborindex.hpp>


namespace QuantLib {

//===========================================================================//
//                              OptionletStripper2                           //
//===========================================================================//

    OptionletStripper2::OptionletStripper2(
                    const boost::shared_ptr<OptionletStripper1>& optionletStripper1,
                    const Handle<CapFloorTermVolCurve>& atmCapFloorTermVolCurve)
    : OptionletStripper(optionletStripper1->termVolSurface(),
                        optionletStripper1->index()),
      optionletStripper1_(optionletStripper1),
      atmCapFloorTermVolCurve_(atmCapFloorTermVolCurve),
      dc_(optionletStripper1_->termVolSurface()->dayCounter()),
      nOptionExpiries_(atmCapFloorTermVolCurve->optionTenors().size()),
      atmCapFloorStrikes_(nOptionExpiries_),
      atmCapFloorPrices_(nOptionExpiries_),
      spreadsVolImplied_(nOptionExpiries_),
      caps_(nOptionExpiries_),
      maxEvaluations_(10000),
      accuracy_(1.e-6) {
        registerWith(optionletStripper1_);
        registerWith(atmCapFloorTermVolCurve_);

        QL_REQUIRE(dc_ == atmCapFloorTermVolCurve->dayCounter(), "different day counters provided");

     }

    void OptionletStripper2::performCalculations() const {

        //// optionletStripper data
        optionletDates_ = optionletStripper1_->optionletDates();
        optionletPaymentDates_ = optionletStripper1_->optionletPaymentDates();
        optionletAccrualPeriods_ = optionletStripper1_->optionletAccrualPeriods();
        optionletTimes_ = optionletStripper1_->optionletTimes();
        atmOptionletRate_ = optionletStripper1_->atmOptionletRate();
        for (Size i=0; i<optionletTimes_.size(); ++i) {
            optionletStrikes_[i] = optionletStripper1_->optionletStrikes(i);
            optionletVolatilities_[i] = optionletStripper1_->optionletVolatilities(i);
        }

        // atmCapFloorTermVolCurve data
        const std::vector<Period>& optionExpiriesTenors = atmCapFloorTermVolCurve_->optionTenors();
        const std::vector<Time>& optionExpiriesTimes = atmCapFloorTermVolCurve_->optionTimes();

        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            // atm option price
            Rate dummyStrike = 0.;
            Volatility atmOptionVol = atmCapFloorTermVolCurve_->volatility(optionExpiriesTimes[optionIndex],dummyStrike);
            boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(index_->termStructure(),
                                                    atmOptionVol, dc_));
            caps_[optionIndex] =
                MakeCapFloor(CapFloor::Cap,
                             optionExpiriesTenors[optionIndex], index_,
                             dummyStrike, 0*Days)
                .withPricingEngine(engine);
            atmCapFloorStrikes_[optionIndex] = caps_[optionIndex]->atmRate();
            caps_[optionIndex] =
                MakeCapFloor(CapFloor::Cap,
                             optionExpiriesTenors[optionIndex], index_,
                             atmCapFloorStrikes_[optionIndex], 0*Days)
                .withPricingEngine(engine);
            atmCapFloorPrices_[optionIndex] = caps_[optionIndex]->NPV();
        }

        spreadsVolImplied_ = spreadsVolImplied();

        OptionletStripperAdapter adapter(optionletStripper1_);

        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            for (Size i=0; i<optionletVolatilities_.size(); ++i) {
                if (i<=caps_[optionIndex]->leg().size()) {
                    Volatility unadjustedVol = adapter.volatility(optionletTimes_[i], atmCapFloorStrikes_[optionIndex]);
                    Volatility adjustedVol = unadjustedVol + spreadsVolImplied_[optionIndex];

                    // insert adjusted volatility
                    std::vector<Rate>::const_iterator previousNode =
                        std::lower_bound(optionletStrikes_[i].begin(), optionletStrikes_[i].end(),
                                         atmCapFloorStrikes_[optionIndex]);
                    Size insertIndex = previousNode - optionletStrikes_[i].begin();

                    optionletStrikes_[i].insert(optionletStrikes_[i].begin() + insertIndex, atmCapFloorStrikes_[optionIndex]);
                    optionletVolatilities_[i].insert(optionletVolatilities_[i].begin() + insertIndex, adjustedVol);
                }
            }
        }
    }

    std::vector<Volatility> OptionletStripper2::spreadsVolImplied() const {

        std::vector<Volatility> result;
        Volatility guess = 0.0001, minVol = -0.1, maxVol = 0.1;
        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            ObjectiveFunction f(optionletStripper1_, caps_[optionIndex],
                                atmCapFloorPrices_[optionIndex]);
            Brent solver;
            solver.setMaxEvaluations(maxEvaluations_);
            result.push_back(solver.solve(f, accuracy_, guess, minVol, maxVol));
        }
        return result;
    }

    std::vector<Volatility> OptionletStripper2::spreadsVol() const {
        calculate();
        return spreadsVolImplied_;
    }

    std::vector<Rate> OptionletStripper2::atmCapFloorStrikes() const{
        calculate();
        return atmCapFloorStrikes_;
    }

    std::vector<Real> OptionletStripper2::atmCapFloorPrices() const {
        calculate();
        return atmCapFloorPrices_;
    }

//===========================================================================//
//                 OptionletStripper2::ObjectiveFunction                     //
//===========================================================================//

    OptionletStripper2::ObjectiveFunction::ObjectiveFunction(
            const boost::shared_ptr<OptionletStripper1>& optionletStripper1,
            const boost::shared_ptr<CapFloor>& cap,
            Real targetValue):
       optionletStripper1_(optionletStripper1),
       cap_(cap),
       targetValue_(targetValue){ }

    Real OptionletStripper2::ObjectiveFunction::operator()(Volatility spreadVol) const {

        boost::shared_ptr<OptionletVolatilityStructure> adapter(new
            OptionletStripperAdapter(optionletStripper1_));

        boost::shared_ptr<SimpleQuote> spreadQuote(new SimpleQuote(spreadVol));

        boost::shared_ptr<OptionletVolatilityStructure> spreadedAdapter(new
            SpreadedOptionletVol(Handle<OptionletVolatilityStructure>(adapter),
                                 Handle<Quote>(spreadQuote)));

        boost::shared_ptr<BlackCapFloorEngine> engine(new
            BlackCapFloorEngine(optionletStripper1_->index()->termStructure(),
                                Handle<OptionletVolatilityStructure>(spreadedAdapter)));

        cap_->setPricingEngine(engine);
        return cap_->NPV()-targetValue_;
    }
}
