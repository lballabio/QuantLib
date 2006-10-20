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


#include <ql/Instruments/makevanillaswap.hpp>
#include <ql/Volatilities/swaptionvolcube.hpp>
#include <ql/Volatilities/smilesection.hpp>
#include <ql/Math/sabrinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>

#include <fstream>
#include <string>
#include <ql/Utilities/dataformatters.hpp>

namespace QuantLib {

    SwaptionVolatilityCube::SwaptionVolatilityCube(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const Calendar& calendar,
        Integer swapSettlementDays,
        Frequency fixedLegFrequency,
        BusinessDayConvention fixedLegConvention,
        const DayCounter& fixedLegDayCounter,
        const boost::shared_ptr<Xibor>& iborIndex,
        Time shortTenor,
        const boost::shared_ptr<Xibor>& iborIndexShortTenor)
    : SwaptionVolatilityStructure(0, calendar),
      atmVolStructure_(atmVolStructure),
      exerciseDates_(expiries.size()),
      exerciseTimes_(expiries.size()),
      exerciseDatesAsReal_(expiries.size()),
      lengths_(lengths),
      timeLengths_(lengths.size()),
      nStrikes_(strikeSpreads.size()),
      strikeSpreads_(strikeSpreads),
      localStrikes_(nStrikes_),
      localSmile_(nStrikes_),
      swapSettlementDays_(swapSettlementDays),
      fixedLegFrequency_(fixedLegFrequency),
      fixedLegConvention_(fixedLegConvention),
      fixedLegDayCounter_(fixedLegDayCounter),
      iborIndex_(iborIndex),
      shortTenor_(shortTenor),
      iborIndexShortTenor_(iborIndexShortTenor) {

        if (!atmVolStructure_.empty())
            unregisterWith(atmVolStructure_);
        atmVolStructure_ = atmVolStructure;
        if (!atmVolStructure_.empty())
            registerWith(atmVolStructure_);
        notifyObservers();

        if (!iborIndex_)
            registerWith(iborIndex_);
        if (!iborIndexShortTenor_)
            registerWith(iborIndexShortTenor_);

        nExercise_ = expiries.size();
        exerciseDates_[0] = calendar.advance(referenceDate(),
                                             expiries[0],
                                             Following); //FIXME
        exerciseDatesAsReal_[0] =
            static_cast<Real>(exerciseDates_[0].serialNumber());
        exerciseTimes_[0] = timeFromReference(exerciseDates_[0]);
        QL_REQUIRE(0.0<exerciseTimes_[0],
                   "first exercise time is negative ("
                   << exerciseTimes_[0] << ")");
        for (Size i=1; i<nExercise_; i++) {
            exerciseDates_[i] = calendar.advance(referenceDate(),
                                                 expiries[i],
                                                 Following); //FIXME
            exerciseDatesAsReal_[i] =
                static_cast<Real>(exerciseDates_[i].serialNumber());
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
            QL_REQUIRE(exerciseTimes_[i-1]<exerciseTimes_[i],
                       "non increasing exercise times: time[" << i-1 <<
                       "] = " << exerciseTimes_[i-1] << ", time[" << i <<
                       "] = " << exerciseTimes_[i]);
        }

        exerciseInterpolator_ =
            LinearInterpolation(exerciseTimes_.begin(),
                                exerciseTimes_.end(),
                                exerciseDatesAsReal_.begin());
        exerciseInterpolator_.enableExtrapolation();

        nlengths_ = lengths_.size();
        Date startDate = exerciseDates_[0]; // as good as any
        Date endDate = startDate + lengths_[0];
        timeLengths_[0] = dayCounter().yearFraction(startDate,endDate);
        QL_REQUIRE(0.0<timeLengths_[0],
                   "first time length is negative");
        for (Size i=1; i<nlengths_; i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter().yearFraction(startDate,endDate);
            QL_REQUIRE(timeLengths_[i-1]<timeLengths_[i],
                       "non increasing time length");
        }

        QL_REQUIRE(nStrikes_>1, "too few strikes (" << nStrikes_ << ")");
        for (Size i=1; i<nStrikes_; i++) {
            QL_REQUIRE(strikeSpreads_[i-1]<strikeSpreads_[i],
                "non increasing strike spreads");
        }
    }

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

    boost::shared_ptr<SmileSection>
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
        return boost::shared_ptr<SmileSection>(new
            SmileSection(start, strikes, volatilities));
    }

    Volatility SwaptionVolatilityCubeByLinear::volatilityImpl(
                        Time start, Time length, Rate strike) const {
            return smileSection(start, length)->volatility(strike);
    }

    boost::shared_ptr<SmileSection>
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
        return boost::shared_ptr<SmileSection>(new
            SmileSection(p.first, strikes, volatilities));
    }

    Volatility SwaptionVolatilityCubeByLinear::volatilityImpl(
        const Date& exerciseDate, const Period& length, Rate strike) const {
            return smileSection(exerciseDate, length)->volatility(strike);
    }

    Rate SwaptionVolatilityCube::atmStrike(const Date& exerciseDate,
                                           const Period& swapTenor) const {

        // FIXME: delegate to SwapIndex

        //boost::shared_ptr<Xibor> iborIndexEffective(iborIndex_);
        //if (length<=shortTenor_) {
        //    iborIndexEffective = iborIndexShortTenor_;
        //}

        // vanilla swap's parameters
        Date startDate = calendar().advance(exerciseDate,
            swapSettlementDays_, Days);
        VanillaSwap swap = MakeVanillaSwap(swapTenor, iborIndex_, 0.0)
            .withEffectiveDate(startDate)
            .withFixedLegCalendar(calendar())
            .withFixedLegDayCount(fixedLegDayCounter_)
            .withFixedLegTenor(Period(fixedLegFrequency_))
            .withFixedLegConvention(fixedLegConvention_)
            .withFixedLegTerminationDateConvention(fixedLegConvention_);

        return swap.fairRate();
    }

}
