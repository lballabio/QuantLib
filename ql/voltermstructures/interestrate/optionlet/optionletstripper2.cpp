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

#include <ql/voltermstructures/interestrate/optionlet/optionletstripperadapter.hpp>
#include <ql/voltermstructures/interestrate/optionlet/optionletstripper2.hpp>
#include <ql/voltermstructures/interestrate/optionlet/spreadedoptionletvol.hpp>
#include <ql/voltermstructures/interestrate/capfloor/capfloortermvolcurve.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/voltermstructures/interestrate/capfloor/capfloortermvolsurface.hpp>


namespace QuantLib {

//===========================================================================//
//                              OptionletStripper2                           //
//===========================================================================//

    OptionletStripper2::OptionletStripper2(
                    const Handle<OptionletStripper>& optionletStripper,
                    const Handle<CapFloorTermVolCurve>& atmCapFloorTermVolCurve)
    : optionletStripper_(optionletStripper),
      atmCapFloorTermVolCurve_(atmCapFloorTermVolCurve), 
      dc_(optionletStripper_->surface()->dayCounter()),
      nOptionExpiries_(atmCapFloorTermVolCurve->optionTenors().size()),
      atmStrikes_(nOptionExpiries_),
      spreadsVolImplied_(nOptionExpiries_),
      atmOptionPrice_(nOptionExpiries_),
      caps_(nOptionExpiries_),
      maxEvaluations_(10000),
      accuracy_(1.e-6) {
        registerWith(optionletStripper_);
        registerWith(atmCapFloorTermVolCurve_);
     }

    void OptionletStripper2::performCalculations() const {

        //// optionletStripper data
        const boost::shared_ptr<IborIndex>& index = optionletStripper_->index();

        // atmCapFloorTermVolCurve data
        const std::vector<Period>& optionExpiriesTenors = atmCapFloorTermVolCurve_->optionTenors();
        const std::vector<Time>& optionExpiriesTimes = atmCapFloorTermVolCurve_->optionTimes();

        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            // atm option price
            Rate dummyStrike = 0.;
            Volatility atmOptionVol = atmCapFloorTermVolCurve_->volatility(optionExpiriesTimes[optionIndex],dummyStrike);
            boost::shared_ptr<BlackCapFloorEngine> engine(new
                                BlackCapFloorEngine(atmOptionVol, dc_));
            caps_[optionIndex] = MakeCapFloor(CapFloor::Cap,
                                            optionExpiriesTenors[optionIndex], index,
                                            dummyStrike, 0*Days, engine);
            atmStrikes_[optionIndex] = caps_[optionIndex]->atmRate();
            caps_[optionIndex] = MakeCapFloor(CapFloor::Cap,
                                            optionExpiriesTenors[optionIndex], index,
                                            atmStrikes_[optionIndex], 0*Days, engine);
            atmOptionPrice_[optionIndex] = caps_[optionIndex]->NPV();
        }

        spreadsVolImplied_ = spreadsVolImplied();
    }

    std::vector<Volatility> OptionletStripper2::spreadsVolImplied() const {

        std::vector<Volatility> result;
        Volatility guess = 0.0001, minVol = -0.1, maxVol = 0.1;
        for (Size optionIndex=0; optionIndex<nOptionExpiries_; ++optionIndex) {
            ObjectiveFunction f(optionletStripper_, caps_[optionIndex], 
                                atmOptionPrice_[optionIndex]);
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

    std::vector<Rate> OptionletStripper2::atmOptionStrikes() const{
        calculate();
        return atmStrikes_;
    }

    std::vector<Real> OptionletStripper2::atmOptionPrices() const {
        calculate();
        return atmOptionPrice_;
    }

//===========================================================================//
//                 OptionletStripper2::ObjectiveFunction                     //
//===========================================================================//

    OptionletStripper2::ObjectiveFunction::ObjectiveFunction(
                const Handle<OptionletStripper>& optionletStripper,
                const boost::shared_ptr<CapFloor>& cap,
                Real targetValue):
       optionletStripper_(optionletStripper),
       cap_(cap), 
       targetValue_(targetValue){ }

    Real OptionletStripper2::ObjectiveFunction::operator()(Volatility spreadVol) const {
        
        Handle<OptionletVolatilityStructure> adapter( boost::shared_ptr<OptionletVolatilityStructure>(new
            OptionletStripperAdapter(optionletStripper_)));
    
        Handle<Quote> spreadHandle( boost::shared_ptr<SimpleQuote>(new 
             SimpleQuote(spreadVol)));

        Handle<OptionletVolatilityStructure> spreadedAdapter( 
            boost::shared_ptr<OptionletVolatilityStructure>(new 
                SpreadedOptionletVol(adapter, spreadHandle)));
        
        boost::shared_ptr<BlackCapFloorEngine> engine(new
                            BlackCapFloorEngine(spreadedAdapter));
        cap_->setPricingEngine(engine);

        return cap_->NPV()-targetValue_;
    }
}
