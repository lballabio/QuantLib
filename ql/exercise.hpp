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
/*! \file exercise.hpp
    \brief Exercise type class (European, Bermudan or American)

    \fullpath
    ql/%exercise.hpp
*/

// $Id$

#ifndef quantlib_exercise_type_h
#define quantlib_exercise_type_h

#include <ql/calendar.hpp>
#include <ql/date.hpp>

#include <ql/Calendars/target.hpp>

#include <vector>

namespace QuantLib {

    //! Exercise class (American, Bermudan or European)
    /*! \warning the input dates must be effective (adjusted) exercise dates.
    */
    class Exercise {
      public:
        enum Type { American, Bermudan, European };

        Exercise(Type type, const std::vector<Date>& dates);

        Type type() const;

        Date date(Size index = 0) const;
        const std::vector<Date>& dates() const;

      protected:
        std::vector<Date> dates_;

      private:
        Type type_;
    };

    //! American exercise class
    /*! An American option can be exercised at any time between two predefined
        dates
    */
    class AmericanExercise : public Exercise {
      public:
        AmericanExercise(Date earliestDate, Date latestDate);
    };

    //! Bermudan exercise class
    /*! A Bermudan option can only be exercised at a set of fixed dates.
    */
    class BermudanExercise : public Exercise {
      public:
        BermudanExercise(const std::vector<Date>& dates);
    };

    //! European exercise class
    /*! A European option can only be exercised at one date.
    */
    class EuropeanExercise : public Exercise {
      public:
        EuropeanExercise(Date date);
    };

    // inline definitions

    inline Exercise::Exercise(Type type, const std::vector<Date>& dates)
    : dates_(dates), type_(type) {}

    inline Exercise::Type Exercise::type() const {
        return type_;
    }

    inline Date Exercise::date(Size index) const {
        return dates_[index];
    }

    inline const std::vector<Date>& Exercise::dates() const {
        return dates_;
    }

    inline AmericanExercise::AmericanExercise(Date earliest, Date latest)
    : Exercise(American, std::vector<Date>(2)) {
        dates_[0] = earliest;
        dates_[1] = latest;
    }

    inline BermudanExercise::BermudanExercise(const std::vector<Date>& dates)
    : Exercise(Bermudan, dates) {}

    inline EuropeanExercise::EuropeanExercise(Date date)
    : Exercise(European, std::vector<Date>(1,date)) {}

}

#endif
