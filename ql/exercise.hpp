
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
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

namespace QuantLib {

    class Exercise {
      public:
        enum Type { American, Bermudan, European };

        Exercise(Type type) : type_(type) {}

        Type type() const { return type_; }

        virtual Date exerciseDate(unsigned index = 0) const = 0;
        virtual std::vector<Date> exerciseDates() const = 0;
      private:
        Type type_;
    };

    class AmericanExercise : public Exercise {
      public:
        AmericanExercise( Date exerciseDate) 
        : Exercise(American), exerciseDate_(exerciseDate) {}

        virtual Date exerciseDate(unsigned index = 0) const { 
            return exerciseDate_;
        }
        virtual std::vector<Date> exerciseDates() const { 
            return std::vector<Date>(1, exerciseDate_);
        }
      private:
        Date exerciseDate_;
    };

    class BermudanExercise : public Exercise {
      public:
        BermudanExercise(const std::vector<Date>& exerciseDates) 
        : Exercise(Bermudan), exerciseDates_(exerciseDates) {}
        virtual ~BermudanExercise() {}

        virtual Date exerciseDate(unsigned index = 0) const { 
            return exerciseDates_[index];
        }
        virtual std::vector<Date> exerciseDates() const { 
            return exerciseDates_;
        }
      private:
        std::vector<Date> exerciseDates_;
    };

    class EuropeanExercise : public Exercise {
      public:
        EuropeanExercise( Date exerciseDate) 
        : Exercise(European), exerciseDate_(exerciseDate) {}

        virtual Date exerciseDate(unsigned index = 0) const { 
            return exerciseDate_;
        }
        virtual std::vector<Date> exerciseDates() const { 
            return std::vector<Date>(1, exerciseDate_);
        }
      private:
        Date exerciseDate_;
    };

}

#endif
