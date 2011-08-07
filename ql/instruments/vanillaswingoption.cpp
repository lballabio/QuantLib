/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010, 2011 Klaus Spanderen

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

/*! \file vanillaswingoption.cpp
    \brief vanilla swing option class
*/


#include <ql/event.hpp>
#include <ql/instruments/vanillaswingoption.hpp>

namespace QuantLib {

    namespace {
        const Size secPerDay = 24u*3600u;

        std::pair<std::vector<Date>, std::vector<Size> >
            createDateTimes(const Date& from, const Date& to, Size stepSize) {

            std::vector<Size> secs;
            std::vector<Date> dates;

            Date iterDate = from;
            Size iterStepSize = 0u;

            while (iterDate <= to) {
                dates.push_back(iterDate);
                secs.push_back(iterStepSize);

                iterStepSize+=stepSize;
                if (iterStepSize >= secPerDay) {
                    iterDate+=1L;
                    iterStepSize%=secPerDay;
                }
            }

            return std::pair<std::vector<Date>,std::vector<Size> >(dates, secs);
        }
    }

    SwingExercise::SwingExercise(const std::vector<Date>& dates,
                                 const std::vector<Size>& seconds)
    : BermudanExercise(dates),
      seconds_(seconds.empty() ? std::vector<Size>(dates.size(), 0u)
                               : seconds) {
        QL_REQUIRE(dates_.size() == seconds_.size(),
                   "dates and seconds must have the same size");
        for (Size i=0; i < dates_.size(); ++i) {
            QL_REQUIRE(seconds_[i] < secPerDay,
                       "a date can not have more than 24*3600 seconds");
            if (i > 0) {
                QL_REQUIRE(dates_[i-1] < dates_[i]
                           || (dates_[i-1] == dates_[i]
                               && seconds_[i-1] < seconds_[i]),
                           "date times must be sorted");
            }
        }
    }


    SwingExercise::SwingExercise(const Date& from,
                                 const Date& to, Size stepSizeSecs)
    : BermudanExercise(createDateTimes(from, to, stepSizeSecs).first),
      seconds_(createDateTimes(from, to, stepSizeSecs).second) {
    }

    const std::vector<Size>& SwingExercise::seconds() const { return seconds_; }

    std::vector<Time> SwingExercise::exerciseTimes(const DayCounter& dc,
                                                   const Date& refDate) const {
        std::vector<Time> exerciseTimes;
        exerciseTimes.reserve(dates().size());
        for (Size i=0; i<dates().size(); ++i) {
            Time t = dc.yearFraction(refDate, dates()[i]);

            const Time dt
                = dc.yearFraction(refDate, dates()[i]+Period(1u, Days)) - t;

            t += dt*seconds()[i]/(24*3600.);

            QL_REQUIRE(t >= 0, "exercise dates must not contain past date");
            exerciseTimes.push_back(t);
        }

        return exerciseTimes;
    }

    void VanillaSwingOption::arguments::validate() const {
        QL_REQUIRE(payoff, "no payoff given");
        QL_REQUIRE(exercise, "no exercise given");

        QL_REQUIRE(minExerciseRights <= maxExerciseRights,
                   "minExerciseRights <= maxExerciseRights")
        QL_REQUIRE(exercise->dates().size() >= maxExerciseRights,
                   "number of exercise rights exceeds "
                   "number of exercise dates");
    }

    void VanillaSwingOption::setupArguments(
                            PricingEngine::arguments* args) const {
        VanillaSwingOption::arguments* arguments =
            dynamic_cast<VanillaSwingOption::arguments*>(args);
        QL_REQUIRE(arguments != 0, "wrong argument type");

        arguments->payoff
            = boost::dynamic_pointer_cast<StrikedTypePayoff>(payoff_);
        arguments->exercise
            = boost::dynamic_pointer_cast<SwingExercise>(exercise_);
        arguments->minExerciseRights = minExerciseRights_;
        arguments->maxExerciseRights = maxExerciseRights_;
    }

    bool VanillaSwingOption::isExpired() const {
        return detail::simple_event(exercise_->lastDate()).hasOccurred();
    }
}
