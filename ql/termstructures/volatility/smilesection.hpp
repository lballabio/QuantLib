/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mario Pucci

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
#include <ql/time/date.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/utilities/null.hpp>
#include <vector>

namespace QuantLib {

    class SpreadedSmileSection;

    //! interest rate volatility smile section
    /*! This abstract class provides volatility smile section interface */
    class SmileSection : public virtual Observable, public Observer {
      public:
        friend class SpreadedSmileSection;
        SmileSection(const Date& d,
                     const DayCounter& dc = DayCounter(),
                     const Date& referenceDate = Date());
        SmileSection(Time exerciseTime,
                     const DayCounter& dc = DayCounter());
        SmileSection() {}

        virtual ~SmileSection() {}

        virtual void update();
        virtual Real minStrike() const = 0;
        virtual Real maxStrike() const = 0;
        Real variance(Rate strike = Null<Rate>()) const;
        Volatility volatility(Rate strike = Null<Rate>()) const;
        virtual Real atmLevel() const = 0;
        const Date& exerciseDate() const { return exerciseDate_; }
        Time exerciseTime() const { return exerciseTime_; }
        const DayCounter& dayCounter() const { return dc_; }
        void initializeExerciseTime() const;
      protected:
        Real varianceImpl(Rate strike) const;
        virtual Volatility volatilityImpl(Rate strike) const = 0;
      private:
        bool isFloating_;
        mutable Date referenceDate_;
        Date exerciseDate_;
        DayCounter dc_;
        mutable Time exerciseTime_;
    };

    inline Real SmileSection::variance(Rate strike) const {
        if (strike==Null<Rate>())
            strike = atmLevel();
        return varianceImpl(strike);
    }

    inline Volatility SmileSection::volatility(Rate strike) const {
        if (strike==Null<Rate>())
            strike = atmLevel();
        return volatilityImpl(strike);
    }

    inline Real SmileSection::varianceImpl(Rate strike) const {
        Volatility v = volatilityImpl(strike);
        return v*v*exerciseTime();
    }

    class SabrSmileSection : public SmileSection {
      public:
        SabrSmileSection(Time timeToExpiry,
                         Rate forward,
                         const std::vector<Real>& sabrParameters);
        SabrSmileSection(const Date& d,
                         Rate forward,
                         const std::vector<Real>& sabrParameters,
                         const DayCounter& dc = Actual365Fixed());
        Real minStrike () const { return 0.0; }
        Real maxStrike () const { return QL_MAX_REAL; }
        Real atmLevel() const { return forward_; }
      protected:
        Real varianceImpl(Rate strike) const;
        Volatility volatilityImpl(Rate strike) const;
      private:
        Real alpha_, beta_, nu_, rho_, forward_;
    };

}

#endif
