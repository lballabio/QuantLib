
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file exercise.hpp
    \brief Option exercise classes and payoff function
*/

#ifndef quantlib_exercise_type_h
#define quantlib_exercise_type_h

#include <ql/date.hpp>
#include <vector>

namespace QuantLib {

    //! Base exercise class
    class Exercise {
      public:

        enum Type { American, Bermudan, European };

        virtual ~Exercise() {}

        // inspectors
        bool isNull() const { return type_==Type(-1); }
        Type type() const { return type_; }
        Date date(Size index) const { return dates_[index]; }
        const std::vector<Date>& dates() const { return dates_; }
        Date lastDate() const { return dates_.back(); }

      protected:
        std::vector<Date> dates_;
        Type type_;
    };

    //! Early-exercise base class
    /*! The payoff can be at exercise (default case) or at expiry

        \todo derive a plain American Exercise class (no
              earliestDate, no payoffAtExpiry)
    */
    class EarlyExercise : public Exercise {
      public:
          EarlyExercise(bool payoffAtExpiry = false)
          : payoffAtExpiry_(payoffAtExpiry) {}
        bool payoffAtExpiry() const { return payoffAtExpiry_; }
      private:
        bool payoffAtExpiry_;
    };

    //! American exercise
    /*! An American option can be exercised at any time between two predefined
        dates

        \todo check that everywhere the American condition is applied
              from the earliestDate and not earlier

    */
    class AmericanExercise : public EarlyExercise {
      public:
        AmericanExercise(Date earliestDate,
                         Date latestDate,
                         bool payoffAtExpiry = false);
    };

    //! Bermudan exercise
    /*! A Bermudan option can only be exercised at a set of fixed dates.

        \todo it would be nice to have a way for making a Bermudan with
              one exercise date equivalent to an European
    */
    class BermudanExercise : public EarlyExercise {
      public:
        BermudanExercise(const std::vector<Date>& dates,
                         bool payoffAtExpiry = false);
    };

    //! European exercise
    /*! A European option can only be exercised at one (expiry) date.
    */
    class EuropeanExercise : public Exercise {
      public:
        EuropeanExercise(Date date);
    };


}


#endif
