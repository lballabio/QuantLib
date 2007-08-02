/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file capletvariancecurve.hpp
    \brief caplet variance curve
*/

#include <ql/termstructures/volatilities/caplet/capstripper.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/indexes/iborindex.hpp>

namespace {

    using namespace QuantLib;

    class ImpliedVolHelper{
    public:
        ImpliedVolHelper(CapFloor& cap,
                         Real targetValue,
                         Real& volatilityParameter):
                         targetValue_(targetValue), cap_(cap),
                         volatilityParameter_(volatilityParameter){};

        Real operator()(Real x) const {
            volatilityParameter_ = x;
            cap_.update();
            return cap_.NPV() - targetValue_;
        };
    private:
        Real targetValue_;
        CapFloor& cap_;
        Real& volatilityParameter_;
    };

    void fitVolatilityParameter(CapFloor& mkData,
                                Real& volatilityParameter,
                                Real targetValue,
                                Real accuracy = 1e-5,
                                Size maxEvaluations = 1000,
                                Volatility minVol = 1e-4,
                                Volatility maxVol = 4) {
        ImpliedVolHelper f(mkData, targetValue, volatilityParameter);
        Brent solver;
        solver.setMaxEvaluations(maxEvaluations);
        Real guess;
        // we take the previous value of the volatilityParameter as guess
        // only if it is not equal to one of the bounds
        if (volatilityParameter> minVol && volatilityParameter< maxVol)
            guess = volatilityParameter;
        else
            guess = 0.1;
        solver.solve(f, accuracy, guess, minVol, maxVol);
    }
}

namespace QuantLib {

    void changeCapFloorType(CapFloor& capFloor) {
        capFloor.type_ = (capFloor.type() == CapFloor::Cap) ?
                         CapFloor::Floor : CapFloor::Cap;
        std::swap(capFloor.capRates_, capFloor.floorRates_);
        capFloor.update();
    }

    void CapsStripper::createMarketData() const {
        marketDataCap_.resize(tenors_.size());
        Rate dummyAtmRate = .04; // we will use a real one during boostrap
        // market data cap (used to compute the price to invert) construction
        for (Size i = 0 ; i < tenors_.size(); i++) {
            marketDataCap_[i].resize(strikes_.size());

           for (Size j = 0 ; j < strikes_.size(); j++) {
               boost::shared_ptr<PricingEngine> blackCapFloorEngine(new
                   BlackCapFloorEngine(vols_[i][j], volatilityDayCounter_));
               CapFloor::Type type =
                   (strikes_[j] < dummyAtmRate)? CapFloor::Floor : CapFloor::Cap;
               marketDataCap_[i][j] = MakeCapFloor(type, tenors_[i],
                        index_, strikes_[j], 0*Days, blackCapFloorEngine);
               const_cast<CapsStripper*>(this)->registerWith(marketDataCap_[i][j]);
           }
        }
        // if the volatility surface given by the smile sections volatility
        // structure is empty we will use a simple caplet vol surface
        if (smileSectionInterfaces_.empty())
            if (decoupleInterpolation_)
                parametrizedCapletVolStructure_
                   = boost::shared_ptr<ParametrizedCapletVolStructure>(
                    new DecInterpCapletVolStructure(
                            referenceDate(),volatilityDayCounter_,
                            marketDataCap_,
                            strikes_));
            else
                parametrizedCapletVolStructure_
                   = boost::shared_ptr<ParametrizedCapletVolStructure>(
                    new BilinInterpCapletVolStructure(referenceDate(),
                                                    volatilityDayCounter_,
                                                    marketDataCap_,
                                                    strikes_));
        // otherwise we use an HybridCapletVolatilityStructure
        else{
             boost::shared_ptr<SmileSectionsVolStructure> smileSectionsVolStructure(
                 new SmileSectionsVolStructure(referenceDate(),
                                               volatilityDayCounter_,
                                               smileSectionInterfaces_));
             boost::shared_ptr<ParametrizedCapletVolStructure> volatilitiesFromCaps;
             if (decoupleInterpolation_)
                 volatilitiesFromCaps
                   = boost::shared_ptr<ParametrizedCapletVolStructure>(
                    new DecInterpCapletVolStructure(referenceDate(),
                                        volatilityDayCounter_,
                                        marketDataCap_,
                                        strikes_));
             else
                 volatilitiesFromCaps
                   = boost::shared_ptr<ParametrizedCapletVolStructure>(
                    new BilinInterpCapletVolStructure(referenceDate(),
                                        volatilityDayCounter_,
                                        marketDataCap_,
                                        strikes_));

             parametrizedCapletVolStructure_
                   = boost::shared_ptr<ParametrizedCapletVolStructure>(
                    new HybridCapletVolatilityStructure
                                        (referenceDate(),
                                        volatilityDayCounter_,
                                        volatilitiesFromCaps,
                                        smileSectionsVolStructure));
        }

       Handle<CapletVolatilityStructure> bilinInterpCapletVolStructureHandle(
           parametrizedCapletVolStructure_);
        boost::shared_ptr<PricingEngine> calibBlackCapFloorEngine(new
            BlackCapFloorEngine(bilinInterpCapletVolStructureHandle));
        calibCap_.resize(tenors_.size());
        for (Size i = 0 ; i < tenors_.size(); i++) {
            calibCap_[i].resize(strikes_.size());
            for (Size j = 0 ; j < strikes_.size(); j++) {
                calibCap_[i][j] = boost::shared_ptr<CapFloor>(new
                       CapFloor(*marketDataCap_[i][j]));
                calibCap_[i][j]->setPricingEngine(calibBlackCapFloorEngine);
            }
        }
    }

    CapsStripper::CapsStripper(
         const std::vector<Period>& tenors,
         const std::vector<Rate>& strikes,
         const std::vector<std::vector<Handle<Quote> > >& vols,
         const boost::shared_ptr<IborIndex>& index,
         const Handle< YieldTermStructure >,
         const DayCounter& volatilityDayCounter,
         Real impliedVolatilityAccuracy,
         Size maxEvaluations,
         const std::vector<boost::shared_ptr<SmileSection> >&
             smileSectionInterfaces,
         bool allowExtrapolation,
         bool decoupleInterpolation)
    : CapletVolatilityStructure(0, index->fixingCalendar()),
      volatilityDayCounter_(volatilityDayCounter),
      tenors_(tenors), strikes_(strikes),
      impliedVolatilityAccuracy_(impliedVolatilityAccuracy),
      maxEvaluations_(maxEvaluations),
      atmRates_(tenors_.size()),
      vols_(vols),
      index_(index),
      smileSectionInterfaces_(smileSectionInterfaces),
      decoupleInterpolation_(decoupleInterpolation),
      evaluationDate_(Settings::instance().evaluationDate()){
        enableExtrapolation(allowExtrapolation);
        QL_REQUIRE(vols.size()==tenors.size(),
                   "mismatch between tenors(" << tenors.size() <<
                   ") and vol rows(" << vols.size() << ")");
        QL_REQUIRE(vols[0].size()==strikes.size(),
                   "mismatch between strikes(" << strikes.size() <<
                   ") and vol columns(" << vols[0].size() << ")");
        registerWith(Settings::instance().evaluationDate());
        createMarketData();
    }

    void CapsStripper::performCalculations () const {
        if(evaluationDate_ != Settings::instance().evaluationDate()) {
            createMarketData();
            evaluationDate_ = Settings::instance().evaluationDate();
        }
        Matrix& volatilityParameters =
            parametrizedCapletVolStructure_->volatilityParameters();
        Size i,j;
        Real capPrice = 0.0;
        // refresh the atm rates level
        for (i=0 ; i<atmRates_.size(); ++i)
            atmRates_[i] = marketDataCap_[i].front()->atmRate();

        try {
            for (j=0 ; j<strikes_.size(); ++j) {
                for (i=0 ; i<tenors_.size(); ++i) {
                    CapFloor::Type requestedType =
                    (strikes_[j]<atmRates_[i])? CapFloor::Floor:CapFloor::Cap;
                    CapFloor & mktCap = *marketDataCap_[i][j];
                    // change the marketDataCap_ type if necessary
                    if (requestedType!=mktCap.type())
                        changeCapFloorType(mktCap);
                    capPrice = mktCap.NPV();
                    // change the calibCap_ type if necessary
                    if (requestedType!=calibCap_[i][j]->type())
                        changeCapFloorType(*calibCap_[i][j]);
                    fitVolatilityParameter(*calibCap_[i][j],
                        volatilityParameters[i][j],
                        capPrice, impliedVolatilityAccuracy_, maxEvaluations_);
                }
            }
        } catch(QuantLib::Error&e) {
            QL_FAIL("CapsStripper::performCalculations:"
                    "\nbooststrap failure at option tenor " << tenors_[i] <<
                    ", strike " << io::rate(strikes_[j]) <<
                    ", cap price is " << capPrice <<
                    "\n"<< e.what());
        }
    }


    Volatility CapsStripper::volatilityImpl(Time t, Rate r) const {
            calculate();
            return parametrizedCapletVolStructure_->volatility(t, r, true);
    }
}
