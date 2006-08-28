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
#include <ql/Math/sabrinterpolation.hpp>
#include <ql/Math/linearinterpolation.hpp>
#include <ql/Math/cubicspline.hpp>


namespace QuantLib {

    SwaptionVolatilityCube::SwaptionVolatilityCube(
        const Handle<SwaptionVolatilityStructure>& atmVolStructure,
        const std::vector<Period>& expiries,
        const std::vector<Period>& lengths,
        const std::vector<Spread>& strikeSpreads,
        const Matrix& volSpreads,
        const Calendar& calendar,
        Frequency fixedLegFrequency,
        BusinessDayConvention fixedLegConvention,
        const DayCounter& fixedLegDayCounter,
        const boost::shared_ptr<Xibor>& iborIndex,
        Time shortTenor,
        const boost::shared_ptr<Xibor>& iborIndexShortTenor)
    : atmVolStructure_(atmVolStructure),
      exerciseDates_(expiries.size()), exerciseTimes_(expiries.size()),
      exerciseDatesAsReal_(expiries.size()),
      lengths_(lengths), timeLengths_(lengths.size()),
      nStrikes_(strikeSpreads.size()), strikeSpreads_(strikeSpreads),
      volSpreads_(nStrikes_, Matrix(expiries.size(), lengths.size(), 0.0)),
      volSpreadsInterpolator_(nStrikes_),
      localStrikes_(nStrikes_), localSmile_(nStrikes_),
      calendar_(calendar), fixedLegFrequency_(fixedLegFrequency),
      fixedLegConvention_(fixedLegConvention),
      fixedLegDayCounter_(fixedLegDayCounter),
      iborIndex_(iborIndex), shortTenor_(shortTenor),
      iborIndexShortTenor_(iborIndexShortTenor)
    {
        Size i, nExercise = expiries.size();
        exerciseDates_[0] = calendar_.advance(referenceDate(),
                                              expiries[0],
                                              Unadjusted); //FIXME
        exerciseDatesAsReal_[0] =
            static_cast<Real>(exerciseDates_[0].serialNumber());
        exerciseTimes_[0] = timeFromReference(exerciseDates_[0]);
        QL_REQUIRE(0.0<exerciseTimes_[0],
                   "first exercise time is negative");
        for (i=1; i<nExercise; i++) {
            exerciseDates_[i] = calendar_.advance(referenceDate(),
                                                  expiries[i],
                                                  Unadjusted); //FIXME
            exerciseDatesAsReal_[i] =
                static_cast<Real>(exerciseDates_[i].serialNumber());
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
            QL_REQUIRE(exerciseTimes_[i-1]<exerciseTimes_[i],
                       "non increasing exercise times");
        }

        exerciseInterpolator_ = LinearInterpolation(exerciseTimes_.begin(),
                                                    exerciseTimes_.end(),
                                                    exerciseDatesAsReal_.begin());
        exerciseInterpolator_.enableExtrapolation();

        Size nlengths = lengths_.size();
        Date startDate = exerciseDates_[0]; // as good as any
        Date endDate = startDate + lengths_[0];
        timeLengths_[0] = dayCounter().yearFraction(startDate,endDate);
        QL_REQUIRE(0.0<timeLengths_[0],
                   "first time length is negative");
        for (i=1; i<nlengths; i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter().yearFraction(startDate,endDate);
            QL_REQUIRE(timeLengths_[i-1]<timeLengths_[i],
                       "non increasing time length");
        }

        QL_REQUIRE(nStrikes_>1, "too few strikes (" << nStrikes_ << ")");
        for (i=1; i<nStrikes_; i++) {
            QL_REQUIRE(strikeSpreads_[i-1]<strikeSpreads_[i],
                "non increasing strike spreads");
        }

        QL_REQUIRE(nStrikes_==volSpreads.columns(),
                   "nStrikes_!=volSpreads.columns()");
        QL_REQUIRE(nExercise*nlengths==volSpreads.rows(),
                   "nExercise*nlengths!=volSpreads.rows()");
        for (i=0; i<nStrikes_; i++)
        {
            for (Size j=0; j<nExercise; j++) {
                for (Size k=0; k<nlengths; k++) {
                    volSpreads_[i][j][k]=volSpreads[j*nlengths+k][i];
                }
            }
            volSpreadsInterpolator_[i] = BilinearInterpolation(
                timeLengths_.begin(), timeLengths_.end(),
                exerciseTimes_.begin(), exerciseTimes_.end(),
                volSpreads_[i]);
            volSpreadsInterpolator_[i].enableExtrapolation();
        }

        //registerWith(atmVolMatrix_);
    }

    boost::shared_ptr<Interpolation>
    SwaptionVolatilityCube::smile(Time start, Time length) const
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

    VarianceSmileSection SwaptionVolatilityCube::smileSection(Time start, Time length) const {

        std::vector<Real> strikes, volatilities;

        const Rate atmForward = atmStrike(start, length);

        const Volatility atmVol =
            atmVolStructure_->volatility(start, length, atmForward);
        for (Size i=0; i<nStrikes_; i++) {
            strikes.push_back(atmForward + strikeSpreads_[i]);
            volatilities.push_back(atmVol + volSpreadsInterpolator_[i](length, start));
        }
        //add points to force flat extrapolation
        strikes.insert(strikes.begin(), strikes.front()-.1);
        strikes.insert(strikes.end(), strikes.back()+.1);

        volatilities.insert(volatilities.begin(),volatilities.front());
        volatilities.insert(volatilities.end(),volatilities.back());

        return VarianceSmileSection(start, strikes, volatilities);
        //return VarianceSmileSection(start, atmForward, strikes, volatilities);
    }

    Rate SwaptionVolatilityCube::atmStrike(Time start, Time length) const {

        Date exerciseDate = Date(static_cast<BigInteger>(
            exerciseInterpolator_(start)));

        // vanilla swap's parameters
        Integer swapFixingDays = 2; // FIXME
        Date startDate = calendar_.advance(exerciseDate,swapFixingDays,Days);

        Rounding rounder(0);
        Date endDate = NullCalendar().advance(startDate,rounder(length),Years);

        // (lenght<shortTenor_, iborIndexShortTenor_, iborIndex_);

        Schedule fixedSchedule = Schedule(calendar_, startDate, endDate,
            fixedLegFrequency_, fixedLegConvention_, Date(), true, false);
        Frequency floatingLegFrequency_ = iborIndex_->frequency();
        BusinessDayConvention floatingLegBusinessDayConvention_ =
            iborIndex_->businessDayConvention();
        Schedule floatSchedule = Schedule(calendar_, startDate, endDate,
            floatingLegFrequency_, floatingLegBusinessDayConvention_,
            Date(), true, false);
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

    Volatility SwaptionVolatilityCube::
        volatilityImpl(Time start, Time length, Rate strike) const {
            return smile(start, length)->operator()(strike, true);
        }

     VarianceSmileSection::VarianceSmileSection(Time timeToExpiry,
                        const std::vector<Rate>& strikes,
                        const std::vector<Rate>& volatilities) :
     timeToExpiry_(timeToExpiry),
         strikes_(strikes),
         volatilities_(volatilities) {
             interpolation_ = boost::shared_ptr<Interpolation>(new
                 LinearInterpolation(strikes_.begin(),
                 strikes_.end(), volatilities_.begin())
                 );
         }

      VarianceSmileSection::VarianceSmileSection(Time timeToExpiry,
          Rate forwardValue,
          const std::vector<Rate>& strikes,
          const std::vector<Rate>& volatilities) :
      timeToExpiry_(timeToExpiry), strikes_(strikes),
          volatilities_(volatilities) {
              interpolation_ = boost::shared_ptr<Interpolation>(new
                  SABRInterpolation(strikes_.begin(), strikes_.end(), volatilities_.begin(),
                  timeToExpiry, forwardValue, Null<Real>(), Null<Real>(), Null<Real>(),
                  Null<Real>())
                  );
          }

    Volatility VarianceSmileSection::operator ()(const Real& strike) const {
        const Real v = interpolation_->operator()(strike, true);
        return v*v*timeToExpiry_;
    }

}

