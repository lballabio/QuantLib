
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

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
/*! \file flatforward.hpp
    \brief flat forward rate term structure

    \fullpath
    ql/TermStructures/%flatforward.hpp
*/

// $Id$

#ifndef quantlib_flat_forward_curve_h
#define quantlib_flat_forward_curve_h

#include <ql/termstructure.hpp>
#include <ql/dataformatters.hpp>

namespace QuantLib {

    namespace TermStructures {

        class FlatForward : public TermStructure,
                            public Patterns::Observer {
          public:
            // constructors
            FlatForward(const Date& todaysDate,
                        const Date& settlementDate,
                        Rate forward,
                        const DayCounter& dayCounter);
            FlatForward(const Date& todaysDate,
                        const Date& settlementDate,
                        const RelinkableHandle<MarketElement>& forward,
                        const DayCounter& dayCounter);
            // inspectors
            DayCounter dayCounter() const;
            Date todaysDate() const {return todaysDate_; }
            Date settlementDate() const;
            Date maxDate() const;
            // Observer interface
            void update();
          protected:
            Rate zeroYieldImpl(Time, bool extrapolate = false) const;
            DiscountFactor discountImpl(Time,
                bool extrapolate = false) const;
            Rate forwardImpl(Time, bool extrapolate = false) const;
          private:
            DayCounter dayCounter_;
            Date settlementDate_;
            RelinkableHandle<MarketElement> forward_;
        };

        // inline definitions

        inline FlatForward::FlatForward(const Date& settlementDate,
            Rate forward, const DayCounter& dayCounter)
        : settlementDate_(settlementDate), dayCounter_(dayCounter) {
            forward_.linkTo(
                Handle<MarketElement>(new SimpleMarketElement(forward)));
            registerWith(forward_);
        }

        inline FlatForward::FlatForward(const Date& settlementDate,
            const RelinkableHandle<MarketElement>& forward,
            const DayCounter& dayCounter)
        : settlementDate_(settlementDate), forward_(forward),
          dayCounter_(dayCounter) {
            registerWith(forward_);
        }

        inline DayCounter FlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date FlatForward::settlementDate() const {
            return settlementDate_;
        }

        inline Date FlatForward::maxDate() const {
            return Date::maxDate();
        }

        inline void FlatForward::update() {
            notifyObservers();
        }

        inline Rate FlatForward::zeroYieldImpl(Time t, bool) const {
            // no forward limit on time
            QL_REQUIRE(t >= 0.0,
                "FlatForward: zero yield undefined for time (" +
                DoubleFormatter::toString(t) + ")");
            return forward_->value();
        }

        inline DiscountFactor FlatForward::discountImpl(Time t, bool) const {
            // no forward limit on time
            QL_REQUIRE(t >= 0.0,
                "FlatForward: discount undefined for time (" +
                DoubleFormatter::toString(t) + ")");
            return DiscountFactor(QL_EXP(-forward_->value()*t));
        }

        inline Rate FlatForward::forwardImpl(Time t, bool) const {
            // no forward limit on time
            QL_REQUIRE(t >= 0.0,
                "FlatForward: forward undefined for time (" +
                DoubleFormatter::toString(t) + ")");
            return forward_->value();
        }

    }

}

#endif
