/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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


#include <ql/pricingengines/blackformula.hpp>
#include <ql/pricingengines/inflation/inflationcapfloorengines.hpp>
#include <ql/termstructures/volatility/inflation/yoyinflationoptionletvolatilitystructure.hpp>
#include <utility>

namespace QuantLib {


    YoYInflationCapFloorEngine::YoYInflationCapFloorEngine(
        ext::shared_ptr<YoYInflationIndex> index,
        Handle<YoYOptionletVolatilitySurface> volatility,
        Handle<YieldTermStructure> nominalTermStructure)
    : index_(std::move(index)), volatility_(std::move(volatility)),
      nominalTermStructure_(std::move(nominalTermStructure)) {
        registerWith(index_);
        registerWith(volatility_);
        registerWith(nominalTermStructure_);
    }


    void YoYInflationCapFloorEngine::setVolatility(
            const Handle<YoYOptionletVolatilitySurface> &v) {
        if (!volatility_.empty())
            unregisterWith(volatility_);
        volatility_ = v;
        registerWith(volatility_);
        update();
    }


    void YoYInflationCapFloorEngine::calculate() const {

        // copy black version then adapt to others

        Real value = 0.0;
        Size optionlets = arguments_.startDates.size();
        std::vector<Real> values(optionlets, 0.0);
        std::vector<Real> stdDevs(optionlets, 0.0);
        std::vector<Real> forwards(optionlets, 0.0);
        YoYInflationCapFloor::Type type = arguments_.type;

        Handle<YoYInflationTermStructure> yoyTS
        = index()->yoyInflationTermStructure();

        QL_DEPRECATED_DISABLE_WARNING
        Handle<YieldTermStructure> nominalTS =
            !nominalTermStructure_.empty() ?
            nominalTermStructure_ :
            yoyTS->nominalTermStructure();
        QL_DEPRECATED_ENABLE_WARNING

        Date settlement = nominalTS->referenceDate();

        for (Size i=0; i<optionlets; ++i) {
            Date paymentDate = arguments_.payDates[i];
            if (paymentDate > settlement) { // discard expired caplets
                DiscountFactor d = arguments_.nominals[i] *
                    arguments_.gearings[i] *
                    nominalTS->discount(paymentDate) *
                arguments_.accrualTimes[i];

                // We explicitly have the index and assume that
                // the fixing is natural, i.e. no convexity adjustment.
                // If that was required then we would also need
                // nominal vols in the pricing engine, i.e. a different engine.
                // This also means that we do not need the coupon to have
                // a pricing engine to return the swaplet rate and then
                // the adjusted fixing in the instrument.
                forwards[i] = yoyTS->yoyRate(arguments_.fixingDates[i],Period(0,Days));
                Rate forward = forwards[i];

                Date fixingDate = arguments_.fixingDates[i];
                Time sqrtTime = 0.0;
                if (fixingDate > volatility_->baseDate()){
                    sqrtTime = std::sqrt(
                        volatility_->timeFromBase(fixingDate));
                }

                if (type == YoYInflationCapFloor::Cap || type == YoYInflationCapFloor::Collar) {
                    Rate strike = arguments_.capRates[i];
                    if (sqrtTime>0.0) {
                        stdDevs[i] = std::sqrt(
                            volatility_->totalVariance(fixingDate, strike, Period(0,Days)));

                    }

                    // sttDev=0 for already-fixed dates so everything on forward
                    values[i] = optionletImpl(Option::Call, strike,
                                              forward, stdDevs[i], d);
                }
                if (type == YoYInflationCapFloor::Floor || type == YoYInflationCapFloor::Collar) {
                    Rate strike = arguments_.floorRates[i];
                    if (sqrtTime>0.0) {
                        stdDevs[i] = std::sqrt(
                            volatility_->totalVariance(fixingDate, strike, Period(0,Days)));
                    }
                    Real floorlet = optionletImpl(Option::Put, strike,
                                                  forward, stdDevs[i], d);
                    if (type == YoYInflationCapFloor::Floor) {
                        values[i] = floorlet;
                    } else {
                        // a collar is long a cap and short a floor
                        values[i] -= floorlet;
                    }

                }
                value += values[i];
            }
        }
        results_.value = value;

        results_.additionalResults["optionletsPrice"] = values;
        results_.additionalResults["optionletsAtmForward"] = forwards;
        if (type != YoYInflationCapFloor::Collar)
            results_.additionalResults["optionletsStdDev"] = stdDevs;
    }


    //======================================================================
    // pricer implementations
    //======================================================================

    YoYInflationBlackCapFloorEngine::YoYInflationBlackCapFloorEngine(
                    const ext::shared_ptr<YoYInflationIndex>& index,
                    const Handle<YoYOptionletVolatilitySurface>& volatility,
                    const Handle<YieldTermStructure>& nominalTermStructure)
    : YoYInflationCapFloorEngine(index, volatility, nominalTermStructure) {}


    Real YoYInflationBlackCapFloorEngine::optionletImpl(Option::Type type, Rate strike,
                                                        Rate forward, Real stdDev,
                                                        Real d) const
    {
        return blackFormula(type, strike,
                            forward, stdDev, d);
    }



    YoYInflationUnitDisplacedBlackCapFloorEngine
    ::YoYInflationUnitDisplacedBlackCapFloorEngine(
                    const ext::shared_ptr<YoYInflationIndex>& index,
                    const Handle<YoYOptionletVolatilitySurface>& volatility,
                    const Handle<YieldTermStructure>& nominalTermStructure)
    : YoYInflationCapFloorEngine(index, volatility, nominalTermStructure) {}


    Real YoYInflationUnitDisplacedBlackCapFloorEngine::optionletImpl(
                                                        Option::Type type, Rate strike,
                                                        Rate forward, Real stdDev,
                                                        Real d) const
    {
        // could use displacement parameter in blackFormula but this is clearer
        return blackFormula(type, strike+1.0,
                            forward+1.0, stdDev, d);
    }


    YoYInflationBachelierCapFloorEngine::YoYInflationBachelierCapFloorEngine(
                    const ext::shared_ptr<YoYInflationIndex>& index,
                    const Handle<YoYOptionletVolatilitySurface>& volatility,
                    const Handle<YieldTermStructure>& nominalTermStructure)
    : YoYInflationCapFloorEngine(index, volatility, nominalTermStructure) {}


    Real YoYInflationBachelierCapFloorEngine::optionletImpl(Option::Type type, Rate strike,
                                                        Rate forward, Real stdDev,
                                                        Real d) const
    {
        return bachelierBlackFormula(type, strike,
                                     forward, stdDev, d);
    }

}

