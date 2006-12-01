/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 François du Vignaud

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

/*! \file capletvariancecurve.hpp
    \brief caplet variance curve
*/

#include <ql/Volatilities/capstripper.hpp>

#include <ql/Instruments/makecapfloor.hpp>
#include <ql/Volatilities/capletvolatilitiesstructures.hpp>
#include <ql/Solvers1D/brent.hpp>
#include <ql/types.hpp>

namespace QuantLib {

    class ImpliedVolHelper{
    public:
        ImpliedVolHelper(boost::shared_ptr<CapFloor> cap,
                         Real targetValue,
                         Real& volatilityParameter):
                         targetValue_(targetValue), cap_(cap),
                         volatilityParameter_(volatilityParameter){};

        Real operator()(Real x) const {
            volatilityParameter_ = x;
            cap_->update();
            return cap_->NPV() - targetValue_;
        };
    private:
        Real targetValue_;
        boost::shared_ptr<CapFloor> cap_;
        Real& volatilityParameter_;
    };

    void fitVolatilityParameter(boost::shared_ptr<CapFloor> mkData,
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


    CapsStripper::CapsStripper(
         const std::vector<Period>& tenors,
         const std::vector<Rate>& strikes,
         const std::vector<std::vector<Handle<Quote> > >& vols,
         const boost::shared_ptr<Xibor>& index,
         const Handle< YieldTermStructure > termStructure,
         const DayCounter& volatilityDayCounter,
         Real impliedVolatilityAccuracy,
         Size maxEvaluations,
         const std::vector<boost::shared_ptr<SmileSectionInterface> >&
             smileSectionInterfaces)
    : CapletVolatilityStructure(0, index->calendar()),
      volatilityDayCounter_(volatilityDayCounter),
      tenors_(tenors), strikes_(strikes),
      impliedVolatilityAccuracy_(impliedVolatilityAccuracy),
      maxEvaluations_(maxEvaluations){

        QL_REQUIRE(vols.size()==tenors.size(),
                   "mismatch between tenors(" << tenors.size() <<
                   ") and vol rows(" << vols.size() << ")");
        QL_REQUIRE(vols[0].size()==strikes.size(),
                   "mismatch between strikes(" << strikes.size() <<
                   ") and vol columns(" << vols[0].size() << ")");

        marketDataCap_.resize(tenors.size());
        for (Size i = 0 ; i < tenors_.size(); i++) {
            // this caps is used to compute the atm rate only
             boost::shared_ptr<CapFloor> dummyCap = MakeCapFloor(CapFloor::Cap,
                 tenors_[i], index, strikes_.front(), 0*Days);
            Rate atmRate = dummyCap->atmRate();
            marketDataCap_[i].resize(strikes_.size());

           for (Size j = 0 ; j < strikes_.size(); j++) {
               boost::shared_ptr<PricingEngine> blackCapFloorEngine(new
                   BlackCapFloorEngine(vols[i][j], volatilityDayCounter));
               CapFloor::Type type =
                   (strikes_[j] < atmRate)? CapFloor::Floor : CapFloor::Cap;
               marketDataCap_[i][j] = MakeCapFloor(type, tenors_[i],
                        index, strikes_[j], 0*Days, blackCapFloorEngine);
               registerWith(marketDataCap_[i][j]);
           }
        }
        if (smileSectionInterfaces.empty())
            parametrizedCapletVolStructure_
               = boost::shared_ptr<ParametrizedCapletVolStructure>(
                new BilinInterpCapletVolStructure(referenceDate(),
                                                  volatilityDayCounter,
                                                  marketDataCap_,
                                                  strikes));
        else{
             boost::shared_ptr<SmileSectionsVolStructure> smileSectionsVolStructure(
                 new SmileSectionsVolStructure(referenceDate(),
                                               volatilityDayCounter,
                                               smileSectionInterfaces));
             parametrizedCapletVolStructure_
               = boost::shared_ptr<ParametrizedCapletVolStructure>(
                new HybridCapletVolatilityStructure(referenceDate(),
                                                  volatilityDayCounter,
                                                  marketDataCap_,
                                                  strikes,
                                                  smileSectionsVolStructure));
             registerWith(parametrizedCapletVolStructure_);
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


    void CapsStripper::performCalculations () const {
        Matrix& volatilityParameters =
            parametrizedCapletVolStructure_->volatilityParameters();
        Size i,j;
        Real capPrice = 0.0;
        try {
            for (j = 0 ; j < strikes_.size(); j++) {
                for (i = 0 ; i < tenors_.size(); i++) {
                    CapFloor & mktCap = *marketDataCap_[i][j];
                    Real capPrice = mktCap.NPV();
                    fitVolatilityParameter(calibCap_[i][j],
                        volatilityParameters[i][j],
                        capPrice, impliedVolatilityAccuracy_, maxEvaluations_);
                }
            }
        } catch(QuantLib::Error&) {
            QL_FAIL("CapsStripper::performCalculations:"
                    "\nbooststrap failure at option tenor " << tenors_[i] <<
                    ", strike " << strikes_[j] <<
                    ", cap price is " << capPrice);
        }
    }

    Size locateTime(Time x,
                     const std::vector<Time>& values){
        if (x <= values[0])
            return 0;
        if (x >= values.back())
            return values.size()-1;
        Size i = 0;
        while (x > values[i])
            i++;
        return i;
    }

    Volatility CapsStripper::volatilityImpl(Time t, Rate r) const {
            calculate();
            return parametrizedCapletVolStructure_->volatility(t, r, true);
    }
}
