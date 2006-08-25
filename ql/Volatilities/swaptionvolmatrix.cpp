/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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


#include <ql/Volatilities/swaptionvolmatrix.hpp>
#include <ql/Calendars/nullcalendar.hpp>

namespace QuantLib {

    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                    const Date& today,
                    const std::vector<Date>& dates,
                    const std::vector<Period>& lengths,
                    const Matrix& vols,
                    const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(today), dayCounter_(dayCounter),
      exerciseDates_(dates), lengths_(lengths), volatilities_(vols)
      {
        QL_REQUIRE(dates.size()==vols.rows(),
            "mismatch between number of exercise dates ("
            << dates.size() << ") and number of rows ("
            << vols.rows() << ") in the vol matrix");
        QL_REQUIRE(lengths.size()==vols.columns(),
            "mismatch between number of tenors ("
            << lengths.size() << ") and number of rows ("
            << vols.columns() << ") in the vol matrix");
        exerciseTimes_.resize(exerciseDates_.size());
        timeLengths_.resize(lengths_.size());
        Size i;
        for (i=0; i<exerciseDates_.size(); i++) {
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
        }

        Date startDate = exerciseDates_[0]; // as good as any
        for (i=0; i<lengths_.size(); i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            BilinearInterpolation(timeLengths_.begin(),
                                  timeLengths_.end(),
                                  exerciseTimes_.begin(),
                                  exerciseTimes_.end(),
                                  volatilities_);
    }

    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                       const std::vector<Date>& dates,
                       const std::vector<Period>& lengths,
                       const Matrix& vols,
                       const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(0, NullCalendar()), dayCounter_(dayCounter),
      exerciseDates_(dates), lengths_(lengths), volatilities_(vols)
    {
        QL_REQUIRE(dates.size()==vols.rows(),
            "mismatch between number of exercise dates ("
            << dates.size() << ") and number of rows ("
            << vols.rows() << ") in the vol matrix");
        QL_REQUIRE(lengths.size()==vols.columns(),
            "mismatch between number of tenors ("
            << lengths.size() << ") and number of rows ("
            << vols.columns() << ") in the vol matrix");
        exerciseTimes_.resize(exerciseDates_.size());
        timeLengths_.resize(lengths_.size());
        Size i;
        for (i=0; i<exerciseDates_.size(); i++) {
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
        }

        Date startDate = exerciseDates_[0]; // as good as any
        for (i=0; i<lengths_.size(); i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            BilinearInterpolation(timeLengths_.begin(),
                                  timeLengths_.end(),
                                  exerciseTimes_.begin(),
                                  exerciseTimes_.end(),
                                  volatilities_);
    }

    SwaptionVolatilityMatrix::SwaptionVolatilityMatrix(
                        const std::vector<Period>& expiries,
                        const Calendar& calendar,
                        const BusinessDayConvention bdc,
                        const std::vector<Period>& lengths,
                        const Matrix& vols,
                        const DayCounter& dayCounter)
    : SwaptionVolatilityStructure(0, NullCalendar()), dayCounter_(dayCounter),
      lengths_(lengths), volatilities_(vols)
    {
        QL_REQUIRE(expiries.size()==vols.rows(),
            "mismatch between number of expiries ("
            << expiries.size() << ") and number of rows ("
            << vols.rows() << ") in the vol matrix");
        QL_REQUIRE(lengths.size()==vols.columns(),
            "mismatch between number of tenors ("
            << lengths.size() << ") and number of rows ("
            << vols.columns() << ") in the vol matrix");

        exerciseTimes_.resize(expiries.size());
        exerciseDates_.resize(expiries.size());
        timeLengths_.resize(lengths_.size());
        Size i;
        for (i=0; i<expiries.size(); i++) {
            exerciseDates_[i] = calendar.advance(referenceDate(),
                                                 expiries[i], bdc);
            exerciseTimes_[i] = timeFromReference(exerciseDates_[i]);
        }

        Date startDate = exerciseDates_[0]; // as good as any
        for (i=0; i<lengths_.size(); i++) {
            Date endDate = startDate + lengths_[i];
            timeLengths_[i] = dayCounter_.yearFraction(startDate,endDate);
        }
        interpolation_ =
            BilinearInterpolation(timeLengths_.begin(),
                                  timeLengths_.end(),
                                  exerciseTimes_.begin(),
                                  exerciseTimes_.end(),
                                  volatilities_);
    }

    std::pair<Time,Time> SwaptionVolatilityMatrix::convertDates(
                              const Date& exerciseDate, const Period& length) const {
        Time exerciseTime = timeFromReference(exerciseDate);
        Date startDate = exerciseDates_[0]; // for consistency
        Date endDate = startDate + length;
        Time timeLength = dayCounter_.yearFraction(startDate,endDate);
        return std::make_pair(exerciseTime,timeLength);
    }
                              
    VarianceSmileSection SwaptionVolatilityMatrix::smileSection(Time start, Time length) const {

        //any strike
        const Real strike = .04;

        const Volatility atmVol = volatility(start, length, strike);

        std::vector<Real> strikes, volatilities(2, atmVol);
        
        strikes.push_back(strike);
        strikes.push_back(strike+1);

        return VarianceSmileSection(start, strikes, volatilities);
    }

}
