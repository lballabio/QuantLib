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

#include <ql/termstructures/volatilities/interestrate/caplet/capstripper2.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/instruments/makecapfloor.hpp>
#include <ql/termstructures/volatilities/sabrinterpolatedsmilesection.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/termstructures/volatilities/interestrate/cap/capvolsurface.hpp>
#include <ql/indexes/iborindex.hpp>
#include <cmath>

namespace {

    using namespace QuantLib;

    class MatrixPointQuote : public Quote {
      public:
          MatrixPointQuote(const Matrix& matrix, Size i, Size j):
                          matrix_(matrix), i_(i), j_(j){}
      private:
        const Matrix& matrix_;
        Size i_, j_;
        Real value() const {
            return matrix_[i_][j_];
        }
        bool isValid() const { return true; }
    };

    class InterpolatedQuote : public Quote {
      public:
          InterpolatedQuote(const boost::shared_ptr<CapVolatilitySurface>& surface,
                          Time time, Real strike):surface_(surface),
                          time_(time), strike_(strike){}
      private:
        Time time_;
        Real strike_;
        const boost::shared_ptr<CapVolatilitySurface> surface_;
        Real value() const {
            return surface_->volatility(time_, strike_);
        }
        bool isValid() const { return true; }
    };


    class CapAtmRate : public Quote {
      public:
          CapAtmRate(const boost::shared_ptr<CapFloor>& cap):
                          cap_(cap){}
      private:
        boost::shared_ptr<CapFloor> cap_;
        Real value() const {
            return cap_->atmRate();
        }
        bool isValid() const { return true; }
    };

}

namespace QuantLib {

    CapsStripper2::CapsStripper2(
        const boost::shared_ptr<CapVolatilitySurface>& surface,
        const boost::shared_ptr<IborIndex>& index,
        Period timeStep)
        : strikes_(surface->strikes()),
        index_(index), surface_(surface) {

            Date evaluationDate = Settings::instance().evaluationDate();
            
            //  we construct the synthetic tenors grid
            const std::vector<Period> marketTenors = surface->optionTenors();
            for(Period tenor=marketTenors.front();
                tenor<=marketTenors.back(); tenor+=timeStep) 
                tenors_.push_back(tenor);

            maxDate_ = evaluationDate + marketTenors.back();

            forwardCapsVols_ = Matrix(tenors_.size()-1, strikes_.size());
            syntheticCapPrices_ = Matrix(tenors_.size(), strikes_.size());
            forwardCapsStdev_ = Matrix(tenors_.size(), strikes_.size());

            syntheticMarketDataCap_.resize(tenors_.size());
            forwardCaps_.resize(tenors_.size()-1);
            Rate dummyAtmRate = .04; 
            boost::shared_ptr<PricingEngine> dummyBlackCapFloorEngine;
            const Calendar& calendar = index->fixingCalendar();
            const DayCounter& dayCounter = index->termStructure()->dayCounter();
            for (Size i=0; i<tenors_.size(); i++) {
                Date optionDate = calendar.advance(evaluationDate, tenors_[i]);
                syntheticMarketDataCap_[i].resize(strikes_.size());
                if(i>0)
                    forwardCaps_[i-1].resize(strikes_.size());
                
                Time tenorTime = dayCounter.yearFraction(evaluationDate, 
                                            evaluationDate + tenors_[i]);
                tenorsTimes_.push_back(tenorTime);
                std::vector<Handle<Quote> > capletImplStdevs;
                for (Size j=0; j<strikes_.size(); j++) {

                    boost::shared_ptr<InterpolatedQuote> interpolatedQuote(
                        new InterpolatedQuote(surface, tenorTime, strikes_[j]));

                    boost::shared_ptr<PricingEngine> blackCapFloorEngine(new
                        BlackCapFloorEngine(Handle<Quote>(interpolatedQuote), dayCounter));
                    CapFloor::Type type =
                        (strikes_[j] < dummyAtmRate)? CapFloor::Floor : CapFloor::Cap;

                    syntheticMarketDataCap_[i][j] = MakeCapFloor(type, tenors_[i],
                        index_, strikes_[j], 0*Days, blackCapFloorEngine);

                    if(i>0) {
                        forwardCaps_[i-1][j] = MakeCapFloor(type, tenors_[i],
                            index_, strikes_[j], tenors_[i-1], dummyBlackCapFloorEngine);
                        boost::shared_ptr<Quote> capletImplStdev(new MatrixPointQuote(forwardCapsStdev_, i, j));
                        capletImplStdevs.push_back(Handle<Quote>(capletImplStdev));
                    }
                }

                if(i>0) {
                    boost::shared_ptr<Quote> capAtmRate(new CapAtmRate(forwardCaps_[i-1].front()));
                    boost::shared_ptr<SmileSection> smileSection(new SabrInterpolatedSmileSection(
                               optionDate,
                               strikes_,
                               capletImplStdevs,
                               Handle<Quote>(capAtmRate),
                               Null<Real>(),
                               Null<Real>(),
                               Null<Real>(),
                               Null<Real>(),
                               false,
                               false,
                               false,
                               false));
                    smileSections_.push_back(smileSection);
                }
            }
            this->registerWith(surface);
    }

    void CapsStripper2::performCalculations() const {
        for(Size i=0; i<tenors_.size(); ++i) {
            for(Size j=0; j<strikes_.size(); ++j) {
                syntheticCapPrices_[i][j] = syntheticMarketDataCap_[i][j]->NPV();
                if (i>0) {
                    Real forwardCapsPrice = syntheticCapPrices_[i][j] - syntheticCapPrices_[i-1][j];
                    forwardCapsPrices_[i-1][j] = forwardCapsPrice;
                    Real forwardCapsVol = forwardCaps_[i-1][j]->impliedVolatility(forwardCapsPrice);
                    forwardCapsVols_[i-1][j] = forwardCapsVol;
                    forwardCapsStdev_[i-1][j] = forwardCapsVol*std::sqrt(tenorsTimes_[i-1]);
                }
            }
        }
    }
}
