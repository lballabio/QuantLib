/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 Katiuscia Manzoni

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


#include <ql/Volatilities/swaptionvolcubebylinear.hpp>

namespace QuantLib {

    SwaptionVolatilityCubeByLinear::SwaptionVolatilityCubeByLinear(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const std::vector<std::vector<Handle<Quote> > >& volatilitySpreads,
        const Calendar& calendar,
        Integer swapSettlementDays,
        Frequency fixedLegFrequency,
        BusinessDayConvention fixedLegConvention,
        const DayCounter& fixedLegDayCounter,
        const boost::shared_ptr<Xibor>& iborIndex,
        Time shortTenor,
        const boost::shared_ptr<Xibor>& iborIndexShortTenor) :
     SwaptionVolatilityCube(
        atmVolStructure,
        expiries,
        lengths,
        strikeSpreads,
        calendar,
        swapSettlementDays,
        fixedLegFrequency,
        fixedLegConvention,
        fixedLegDayCounter,
        iborIndex,
        shortTenor,
        iborIndexShortTenor),
     volSpreads_(nStrikes_, Matrix(expiries.size(), lengths.size(), 0.0)),
     volSpreadsInterpolator_(nStrikes_) {

        QL_REQUIRE(!volatilitySpreads.empty(), "empty vol spreads matrix");
        for (Size j=0; j<nExercise_; j++) {
             for (Size k=0; k<nlengths_; k++) {
                 QL_REQUIRE(nStrikes_==volatilitySpreads[j*k].size(),
                     "mismatch between number of strikes ("
                     << nStrikes_ << ") and number of columns ("
                     << volatilitySpreads[j*k].size() << ") in row ("
                     << j*nlengths_+k << ")");
             }
        }
        QL_REQUIRE(nExercise_*nlengths_==volatilitySpreads.size(),
                 "mismatch between number of option expiries * swap tenors ("
                 << nExercise_*nlengths_ << ") and number of rows ("
                 << volatilitySpreads.size() <<")");

        for (Size i=0; i<nStrikes_; i++){
            for (Size j=0; j<nExercise_; j++) {
                for (Size k=0; k<nlengths_; k++) {
                    volSpreads_[i][j][k] =
                        volatilitySpreads[j*nlengths_+k][i]->value();
                    registerWith(volatilitySpreads[j*nlengths_+k][i]);
                }
            }
            volSpreadsInterpolator_[i] = BilinearInterpolation(
                timeLengths_.begin(), timeLengths_.end(),
                exerciseTimes_.begin(), exerciseTimes_.end(),
                volSpreads_[i]);
            volSpreadsInterpolator_[i].enableExtrapolation();
        }

    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeByLinear::smileSection(Time start,
                                                 Time length) const {

        Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));
        Rounding rounder(0);
        Period swaptenor(static_cast<Integer>(rounder(length/12.0)), Months);
        const Rate atmForward = atmStrike(exerciseDate, swaptenor);
        const Volatility atmVol =
            atmVolStructure_->volatility(start, length, atmForward);
        std::vector<Real> strikes, volatilities;
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(atmForward + strikeSpreads_[i]);
            volatilities.push_back(
                          atmVol + volSpreadsInterpolator_[i](length, start));
        }
        return boost::shared_ptr<SmileSectionInterface>(new
            InterpolatedSmileSection(start, strikes, volatilities));
    }

    Volatility SwaptionVolatilityCubeByLinear::volatilityImpl(
                        Time start, Time length, Rate strike) const {
            return smileSection(start, length)->volatility(strike);
    }

    boost::shared_ptr<SmileSectionInterface>
    SwaptionVolatilityCubeByLinear::smileSection(const Date& exerciseDate,
                                                 const Period& length) const {

        const Rate atmForward = atmStrike(exerciseDate, length);
        const Volatility atmVol =
            atmVolStructure_->volatility(exerciseDate, length, atmForward);
        std::vector<Real> strikes, volatilities;
        const std::pair<Time, Time> p = convertDates(exerciseDate, length);
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(atmForward + strikeSpreads_[i]);
            volatilities.push_back(
                      atmVol + volSpreadsInterpolator_[i](p.second, p.first));
        }
        return boost::shared_ptr<SmileSectionInterface>(new
            InterpolatedSmileSection(p.first, strikes, volatilities));
    }

    Volatility SwaptionVolatilityCubeByLinear::volatilityImpl(
        const Date& exerciseDate, const Period& length, Rate strike) const {
            return smileSection(exerciseDate, length)->volatility(strike);
    }

}
