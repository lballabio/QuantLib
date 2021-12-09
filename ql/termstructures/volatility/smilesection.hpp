/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2013, 2015 Peter Caspers

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

/*! \file smilesection.hpp
    \brief Smile section base class
*/

#ifndef quantlib_smile_section_hpp
#define quantlib_smile_section_hpp

#include <ql/patterns/observable.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/utilities/null.hpp>
#include <ql/option.hpp>
#include <ql/termstructures/volatility/volatilitytype.hpp>

namespace QuantLib {

    //! interest rate volatility smile section
    /*! This abstract class provides volatility smile section interface */
    class SmileSection : public virtual Observable,
                         public virtual Observer {
      public:
        SmileSection(const Date& d,
                     DayCounter dc = DayCounter(),
                     const Date& referenceDate = Date(),
                     VolatilityType type = ShiftedLognormal,
                     Rate shift = 0.0);
        SmileSection(Time exerciseTime,
                     DayCounter dc = DayCounter(),
                     VolatilityType type = ShiftedLognormal,
                     Rate shift = 0.0);
        SmileSection() = default;

        ~SmileSection() override = default;

        void update() override;
        virtual Real minStrike() const = 0;
        virtual Real maxStrike() const = 0;
        Real variance(Rate strike) const;
        Volatility volatility(Rate strike) const;
        virtual Real atmLevel() const = 0;
        virtual const Date& exerciseDate() const { return exerciseDate_; }
        virtual VolatilityType volatilityType() const {
            return volatilityType_;
        }
        virtual Rate shift() const { return shift_; }
        virtual const Date& referenceDate() const;
        virtual Time exerciseTime() const { return exerciseTime_; }
        virtual const DayCounter& dayCounter() const { return dc_; }
        virtual Real optionPrice(Rate strike,
                                 Option::Type type = Option::Call,
                                 Real discount=1.0) const;
        virtual Real digitalOptionPrice(Rate strike,
                                        Option::Type type = Option::Call,
                                        Real discount=1.0,
                                        Real gap=1.0e-5) const;
        virtual Real vega(Rate strike,
                          Real discount=1.0) const;
        virtual Real density(Rate strike,
                             Real discount=1.0,
                             Real gap=1.0E-4) const;
        Volatility volatility(Rate strike, VolatilityType type, Real shift=0.0) const;
      protected:
        virtual void initializeExerciseTime() const;
        virtual Real varianceImpl(Rate strike) const;
        virtual Volatility volatilityImpl(Rate strike) const = 0;
      private:
        bool isFloating_;
        mutable Date referenceDate_;
        Date exerciseDate_;
        DayCounter dc_;
        mutable Time exerciseTime_;
        VolatilityType volatilityType_;
        Rate shift_;
    };


    // inline definitions

    inline Real SmileSection::variance(Rate strike) const {
        return varianceImpl(strike);
    }

    inline Volatility SmileSection::volatility(Rate strike) const {
        return volatilityImpl(strike);
    }

    inline const Date& SmileSection::referenceDate() const {
        QL_REQUIRE(referenceDate_!=Date(),
                   "referenceDate not available for this instance");
        return referenceDate_;
    }

    inline Real SmileSection::varianceImpl(Rate strike) const {
        Volatility v = volatilityImpl(strike);
        return v*v*exerciseTime();
    }

}

#endif
