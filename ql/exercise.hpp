/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

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
    \brief Option exercise classes and payoff function

    \fullpath
    ql/%exercise.hpp
*/

// $Id$

#ifndef quantlib_exercise_type_h
#define quantlib_exercise_type_h

#include <ql/calendar.hpp>
#include <ql/option.hpp>
#include <vector>
#include <functional>

namespace QuantLib {

    //! Option payoff function
    class Payoff : std::unary_function<double,double> {
    public:
        Payoff() : type_(Option::Type(-1)), strike_(Null<double>()) {}
        Payoff(Option::Type type,
               double strike)
        : type_(type), strike_(strike) {}
        double operator()(double price) const;
        double strike() const { return strike_; };
        Option::Type optionType() const { return type_; };
    private:
        Option::Type type_;
        double strike_;
    };

    inline double Payoff::operator()(double price) const {
        switch (type_) {
          case Option::Call:
            return QL_MAX(price-strike_,0.0);
          case Option::Put:
            return QL_MAX(strike_-price,0.0);
          case Option::Straddle:
            return QL_FABS(strike_-price);
        }
        throw Error("Unknown option type");
    }

    //! Exercise class (American, Bermudan or European)
    /*! \warning the input dates must be effective (adjusted) exercise dates.
    */
    class Exercise {
      public:
        enum Type { American, Bermudan, European };

        Exercise() {type_ = Type(-1); }
        bool isNull() const {return type_==Type(-1); }
        Type type() const;
        Date date(Size index) const;
        const std::vector<Date>& dates() const;
        Date lastDate() const {return dates_[dates_.size()-1];};

      protected:
        std::vector<Date> dates_;
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

    inline Exercise::Type Exercise::type() const {
        return type_;
    }

    inline Date Exercise::date(Size index) const {
        return dates_[index];
    }

    inline const std::vector<Date>& Exercise::dates() const {
        return dates_;
    }

}

#endif
