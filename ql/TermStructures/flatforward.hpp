

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

        class FlatForward : public TermStructure {
          public:
            // constructor
            FlatForward(Currency currency,
                const DayCounter& dayCounter,
                const Date& todaysDate, const Calendar& calendar,
                int settlementDays, Rate forward);
            // inspectors
            Currency currency() const;
            DayCounter dayCounter() const;
            Date todaysDate() const;
            int settlementDays() const;
            Calendar calendar() const;
            Date settlementDate() const;
            Date maxDate() const;
            Date minDate() const;
            Time maxTime() const;
            Time minTime() const;
          protected:
            Rate zeroYieldImpl(Time, bool extrapolate = false) const;
            DiscountFactor discountImpl(Time,
                bool extrapolate = false) const;
            Rate forwardImpl(Time, bool extrapolate = false) const;
          private:
            Currency currency_;
            DayCounter dayCounter_;
            Date todaysDate_;
            Calendar calendar_;
            int settlementDays_;
            Date settlementDate_;
            Rate forward_;
        };

        // inline definitions

        inline FlatForward::FlatForward(Currency currency,
            const DayCounter& dayCounter, const Date& todaysDate,
            const Calendar& calendar, int settlementDays,
            Rate forward)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate), calendar_(calendar),
          settlementDays_(settlementDays), forward_(forward) {
            settlementDate_ = calendar_.advance(
                todaysDate_,settlementDays_,Days);
        }

        inline Currency FlatForward::currency() const {
            return currency_;
        }

        inline DayCounter FlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date FlatForward::todaysDate() const {
            return todaysDate_;
        }

        inline Calendar FlatForward::calendar() const {
            return calendar_;
        }

        inline int FlatForward::settlementDays() const {
            return settlementDays_;
        }

        inline Date FlatForward::settlementDate() const {
            return settlementDate_;
        }

        inline Date FlatForward::maxDate() const {
            return Date::maxDate();
        }

        inline Date FlatForward::minDate() const {
            return settlementDate_;
        }

        inline Time FlatForward::maxTime() const {
            return dayCounter().yearFraction(
                settlementDate(), Date::maxDate());
        }

        inline Time FlatForward::minTime() const {
            return 0.0;
        }

        inline Rate FlatForward::zeroYieldImpl(Time t, bool) const {
            // no forward limit on time
            QL_REQUIRE(t >= 0.0,
                "FlatForward: zero yield requested for negative time (" +
                DoubleFormatter::toString(t) + ")");
            return forward_;
        }

        inline DiscountFactor FlatForward::discountImpl(Time t, bool) const {
            // no forward limit on time
            QL_REQUIRE(t >= 0.0,
                "FlatForward: discount requested for negative time (" +
                DoubleFormatter::toString(t) + ")");
            return DiscountFactor(QL_EXP(-forward_*t));
        }

        inline Rate FlatForward::forwardImpl(Time t, bool) const {
            // no forward limit on time
            QL_REQUIRE(t >= 0.0,
                "FlatForward: forward requested for negative time (" +
                DoubleFormatter::toString(t) + ")");
            return forward_;
        }


    }

}


#endif
