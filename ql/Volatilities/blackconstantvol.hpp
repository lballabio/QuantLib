
/*
 Copyright (C) 2002 Ferdinando Ametrano

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
/*! \file constantvol.hpp
    \brief Black constant volatility, no time dependence, no strike dependence

    \fullpath
    ql/Volatilities/%constantvol.hpp
*/

// $Id$

#ifndef quantlib_blackconstantvol_hpp
#define quantlib_blackconstantvol_hpp

#include <ql/voltermstructure.hpp>
#include <ql/DayCounters/actual365.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace VolTermStructures {

        //! Constant Black volatility, no time-strike dependence
        /*! This class implements the BlackVolatilityTermStructure
            interface for
            a constant Black volatility (no time/strike dependence).
        */
        class BlackConstantVol : public BlackVolatilityTermStructure,
                                 public Patterns::Observer {
          public:
            // constructors
            BlackConstantVol(const Date& referenceDate,
                    double volatility,
                    const DayCounter& dayCounter=DayCounters::Actual365());
            BlackConstantVol(const Date& referenceDate,
                    const RelinkableHandle<MarketElement>& volatility,
                    const DayCounter& dayCounter=DayCounters::Actual365());
            // inspectors
            Date referenceDate() const;
            DayCounter dayCounter() const;
            Date maxDate() const;
            // Observer interface
            void update();
          protected:
            virtual double blackVolImpl(Time t, double strike,
                bool extrapolate = false) const;
          private:
            Date referenceDate_;
            RelinkableHandle<MarketElement> volatility_;
            DayCounter dayCounter_;
        };

        // inline definitions

        inline BlackConstantVol::BlackConstantVol(const Date& referenceDate,
            double volatility, const DayCounter& dayCounter)
        : referenceDate_(referenceDate), dayCounter_(dayCounter) {
            volatility_.linkTo(Handle<MarketElement>(new
                SimpleMarketElement(volatility)));
            registerWith(volatility_);
        }

        inline BlackConstantVol::BlackConstantVol(const Date& referenceDate,
            const RelinkableHandle<MarketElement>& volatility,
            const DayCounter& dayCounter)
        : referenceDate_(referenceDate), volatility_(volatility), 
          dayCounter_(dayCounter) {
            registerWith(volatility_);
        }

        inline DayCounter BlackConstantVol::dayCounter() const {
            return dayCounter_;
        }

        inline Date BlackConstantVol::referenceDate() const {
            return referenceDate_;
        }

        inline Date BlackConstantVol::maxDate() const {
            return Date::maxDate();
        }

        inline void BlackConstantVol::update() {
            notifyObservers();
        }

        inline double BlackConstantVol::blackVolImpl(Time t, double,
            bool) const {
            QL_REQUIRE(t >= 0.0,
                "ConstantVol::blackVolImpl: "
                "negative time (" + DoubleFormatter::toString(t) +
                ") not allowed");
            return volatility_->value();
        }

    }

}

#endif
