/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file exercisetype.hpp
    \brief Exercise type class (European, Bermudan or American)

    \fullpath
    ql/%exercisetype.hpp
*/

// $Id$

#ifndef quantlib_exercise_type_h
#define quantlib_exercise_type_h

#include <ql/calendar.hpp>
#include <ql/date.hpp>

#include <ql/Calendars/target.hpp>

#include <vector>

namespace QuantLib {

    class Exercise {
      public:
        enum Type { American, Bermudan, European };

        Exercise(Type type, const std::vector<Date>& dates)
        : type_(type), dates_(dates), calendar_(Calendars::TARGET()), 
          convention_(ModifiedFollowing), settlementDays_(0) {}

        Type type() const { return type_; }

        Date date(Size index = 0) const { return dates_[index]; }
        const std::vector<Date>& dates() const { return dates_; }

        RollingConvention rollingConvention() const { return convention_; }
        Calendar calendar() const { return calendar_; }
        int settlementDays() const { return settlementDays_; }
      protected:
        Type type_;
        std::vector<Date> dates_;
        Calendar calendar_;
        RollingConvention convention_;
        int settlementDays_;
    };

    class AmericanExercise : public Exercise {
      public:
        AmericanExercise(Date earliestDate, Date latestDate)
        : Exercise(American, std::vector<Date>(2)) {
            dates_[0] = earliestDate;
            dates_[1] = latestDate;
        }
    };

    class BermudanExercise : public Exercise {
      public:
        BermudanExercise(const std::vector<Date>& dates)
        : Exercise(Bermudan, dates) {}
    };

    class EuropeanExercise : public Exercise {
      public:
        EuropeanExercise(Date date)
        : Exercise(European, std::vector<Date>(1,date)) {}
    };

}

#endif
