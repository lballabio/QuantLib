
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
*/
/*! \file exercisetype.hpp
    \brief Exercise type class (European, Bermudan or American)

    \fullpath
    ql/%exercisetype.hpp
*/

// $Id$

#ifndef quantlib_exercise_type_h
#define quantlib_exercise_type_h

#include <ql/qldefines.hpp>
#include <ql/date.hpp>

#include <vector>

namespace QuantLib {

    class Exercise {
      public:
        enum Type { American, Bermudan, European };

        Exercise(Type type, const std::vector<Date>& dates) 
        : type_(type), dates_(dates) {}

        Type type() const { return type_; }

        Date date(Size index = 0) const { return dates_[index]; }
        const std::vector<Date>& dates() const { return dates_; }
      private:
        Type type_;
        std::vector<Date> dates_;
    };

    class AmericanExercise : public Exercise {
      public:
        AmericanExercise( Date date) 
        : Exercise(American, std::vector<Date>(1,date)) {}
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
