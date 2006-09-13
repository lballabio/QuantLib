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
    : atmVolStructure_(atmVolStructure),
      exerciseDates_(expiries.size()),
      exerciseTimes_(expiries.size()),
      exerciseDatesAsReal_(expiries.size()),
      lengths_(lengths),
      timeLengths_(lengths.size()),
      nStrikes_(strikeSpreads.size()),
      strikeSpreads_(strikeSpreads),
      localStrikes_(nStrikes_),
      localSmile_(nStrikes_),
      calendar_(calendar),
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
        exerciseDates_[0] = calendar_.advance(referenceDate(),
                                              expiries[0],
                                              Unadjusted); //FIXME
        exerciseDatesAsReal_[0] =
            static_cast<Real>(exerciseDates_[0].serialNumber());
        exerciseTimes_[0] = timeFromReference(exerciseDates_[0]);
        QL_REQUIRE(0.0<exerciseTimes_[0],
                   "first exercise time is negative ("
                   << exerciseTimes_[0] << ")");
        for (Size i=1; i<nExercise_; i++) {
            exerciseDates_[i] = calendar_.advance(referenceDate(),
                                                  expiries[i],
                                                  Unadjusted); //FIXME
            exerciseDatesAsReal_[i] =
                static_cast<Real>(exerciseDates_[i].serialNumber());
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
            QL_REQUIRE(exerciseTimes_[i-1]<exerciseTimes_[i],
                       "non increasing exercise times: time[" << i-1 <<
                       "] = " << exerciseTimes_[i-1] << ", time[" << i <<
                       "] = " << exerciseTimes_[i]);
        }

        exerciseInterpolator_ = LinearInterpolation(exerciseTimes_.begin(),
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
                }
            }
            volSpreadsInterpolator_[i] = BilinearInterpolation(
                timeLengths_.begin(), timeLengths_.end(),
                exerciseTimes_.begin(), exerciseTimes_.end(),
                volSpreads_[i]);
            volSpreadsInterpolator_[i].enableExtrapolation();
        }

    }

    boost::shared_ptr<Interpolation>
    SwaptionVolatilityCubeByLinear::smile(Time start, Time length) const
    {
        const Rate atmForward = atmStrike(start, length);

        const Volatility atmVol = atmVolStructure_->volatility(start, length, atmForward);
        for (Size i=0; i<nStrikes_; i++) {
            localStrikes_[i] = atmForward + strikeSpreads_[i];
            localSmile_[i]   = atmVol + volSpreadsInterpolator_[i](length, start);
        }
        return boost::shared_ptr<Interpolation>(new
            //SABRInterpolation(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin(), start, atmForward, Null<Real>(), Null<Real>(), Null<Real>(), Null<Real>())
            LinearInterpolation(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin())
            //NaturalCubicSpline(localStrikes_.begin(), localStrikes_.end(), localSmile_.begin())
            );
    }

    boost::shared_ptr<SmileSection> SwaptionVolatilityCubeByLinear::smileSection(Time start, Time length) const {

        std::vector<Real> strikes, volatilities;

        const Rate atmForward = atmStrike(start, length);

        const Volatility atmVol =
            atmVolStructure_->volatility(start, length, atmForward);
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(atmForward + strikeSpreads_[i]);
            volatilities.push_back(atmVol + volSpreadsInterpolator_[i](length, start));
        }
        return boost::shared_ptr<SmileSection>(
            new SmileSection(start, strikes, volatilities)
            );
    }


    Volatility SwaptionVolatilityCubeByLinear::
        volatilityImpl(Time start, Time length, Rate strike) const {
            return smile(start, length)->operator()(strike, true);
        }

    Rate SwaptionVolatilityCubeByLinear::atmStrike(Time start, Time length) const {

        Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));

        // vanilla swap's parameters
        Integer swapFixingDays = 2; // FIXME
        Date startDate = calendar_.advance(exerciseDate,swapFixingDays,Days);

        Rounding rounder(0);
        Date endDate = NullCalendar().advance(startDate,rounder(length),Years);

        // (lenght<shortTenor_, iborIndexShortTenor_, iborIndex_);

        Schedule fixedSchedule(startDate, endDate,
            Period(fixedLegFrequency_), calendar_,
            fixedLegConvention_, fixedLegConvention_,
            true, true);
        //Frequency floatingLegFrequency_ = iborIndex_->frequency();
        BusinessDayConvention floatingLegBusinessDayConvention_ =
            iborIndex_->businessDayConvention();
        Schedule floatSchedule(startDate, endDate,
            iborIndex_->tenor(), calendar_,
            floatingLegBusinessDayConvention_, floatingLegBusinessDayConvention_,
            true, true);
        Real nominal_= 1.0;
        Rate fixedRate_= 0.0;
        Spread spread_= 0.0;
        Handle<YieldTermStructure> termStructure;
        termStructure.linkTo(iborIndex_->termStructure());
        VanillaSwap swap(true, nominal_,
            fixedSchedule, fixedRate_, fixedLegDayCounter_,
            floatSchedule, iborIndex_,
            iborIndex_->settlementDays(), spread_, iborIndex_->dayCounter(),
            termStructure);

        return swap.fairRate();
    }

}

