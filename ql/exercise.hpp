
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

#include <vector>

namespace QuantLib {

    class Exercise {
      public:
        enum Type { American, Bermudan, European };

        Exercise(Type type) : type_(type) {}

        Type type() const { return type_; }

        virtual Date date(unsigned index = 0) const = 0;
        virtual std::vector<Date> dates() const = 0;
      private:
        Type type_;
    };

    class AmericanExercise : public Exercise {
      public:
        AmericanExercise( Date date) 
        : Exercise(American), date_(date) {}

        virtual Date date(unsigned index = 0) const { 
            return date_;
        }
        virtual std::vector<Date> dates() const { 
            return std::vector<Date>(1, date_);
        }
      private:
        Date date_;
    };

    class BermudanExercise : public Exercise {
      public:
        BermudanExercise(const std::vector<Date>& dates) 
        : Exercise(Bermudan), dates_(dates) {}
        virtual ~BermudanExercise() {}

        virtual Date date(unsigned index = 0) const { 
            return dates_[index];
        }
        virtual std::vector<Date> dates() const { 
            return dates_;
        }
      private:
        std::vector<Date> dates_;
    };

    class EuropeanExercise : public Exercise {
      public:
        EuropeanExercise( Date date) 
        : Exercise(European), date_(date) {}

        virtual Date date(unsigned index = 0) const { 
            return date_;
        }
        virtual std::vector<Date> dates() const { 
            return std::vector<Date>(1, date_);
        }
      private:
        Date date_;
    };

}

#endif
