
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

/*! \file flatforward.hpp
    \brief flat forward rate term structure

    \fullpath
    ql/TermStructures/%flatforward.hpp
*/

// $Id$

#ifndef quantlib_flat_forward_curve_h
#define quantlib_flat_forward_curve_h

#include "ql/termstructure.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace TermStructures {

        class FlatForward : public TermStructure {
          public:
            // constructor
            FlatForward(Currency currency, 
                const Handle<DayCounter>& dayCounter,
                const Date& todaysDate, const Handle<Calendar>& calendar, 
                int settlementDays, Rate forward);
            // inspectors
            Currency currency() const;
            Handle<DayCounter> dayCounter() const;
            Date todaysDate() const;
            int settlementDays() const;
            Handle<Calendar> calendar() const;
            Handle<DayCounter> dayCounter() const;
            Date settlementDate() const;
            Date maxDate() const;
            Date minDate() const;
            // zero yield
            Rate zeroYield(const Date&, bool extrapolate = false) const;
            // discount
            DiscountFactor discount(const Date&,
                                    bool extrapolate = false) const;
            // forward (instantaneous)
            Rate forward(const Date&, bool extrapolate = false) const;

            // zero yield
            Rate zeroYield(Time, bool extrapolate = false) const;
            // discount
            DiscountFactor discount(Time,
                                    bool extrapolate = false) const;
            // forward (instantaneous)
            Rate forward(Time, bool extrapolate = false) const;

          private:
            Currency currency_;
            Handle<DayCounter> dayCounter_;
            Date todaysDate_;
            Handle<Calendar> calendar_;
            int settlementDays_;
            Date settlementDate_;
            Rate forward_;
        };

        // inline definitions

        inline FlatForward::FlatForward(Currency currency,
            const Handle<DayCounter>& dayCounter, const Date& todaysDate, 
            const Handle<Calendar>& calendar, int settlementDays, 
            Rate forward)
        : currency_(currency), dayCounter_(dayCounter),
          todaysDate_(todaysDate), calendar_(calendar), 
          settlementDays_(settlementDays), forward_(forward) {
            settlementDate_ = calendar_->advance(
                todaysDate_,settlementDays_,Days);
        }

        inline Currency FlatForward::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> FlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date FlatForward::todaysDate() const {
            return todaysDate_;
        }

        inline Handle<Calendar> FlatForward::calendar() const {
            return calendar_;
        }

        inline Handle<DayCounter> FlatForward::dayCounter() const {
            return dayCounter_;
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

        /*! \pre the given date must be in the range of definition
        of the term structure */
        inline Rate FlatForward::zeroYield(const Date& d,
                                           bool extrapolate) const {
            QL_REQUIRE(d>=minDate() && (d<=maxDate() || extrapolate),
                "FlatForward::zeroYield : "
                "date " + DateFormatter::toString(d) +
                " outside curve definition [" +
                DateFormatter::toString(minDate()) + "-" +
                DateFormatter::toString(maxDate()) + "]");
            return forward_;
        }

        /*! \pre the given date must be in the range of definition
        of the term structure */
        inline DiscountFactor FlatForward::discount(const Date& d,
                                                    bool extrapolate) const {
            QL_REQUIRE(d>=minDate() && (d<=maxDate() || extrapolate),
                "FlatForward::discount : "
                "date " + DateFormatter::toString(d) +
                " outside curve definition [" +
                DateFormatter::toString(minDate()) + "-" +
                DateFormatter::toString(maxDate()) + "]");
            double t = dayCounter_->yearFraction(settlementDate_,d);
            return DiscountFactor(QL_EXP(-forward_*t));
        }

        /*! \pre the given date must be in the range of definition
        of the term structure */
        inline Rate FlatForward::forward(const Date& d,
                                         bool extrapolate) const {
            QL_REQUIRE(d>=minDate() && (d<=maxDate() || extrapolate),
                "FlatForward::forward : "
                "date " + DateFormatter::toString(d) +
                " outside curve definition [" +
                DateFormatter::toString(minDate()) + "-" +
                DateFormatter::toString(maxDate()) + "]");
            return forward_;
        }

        inline Rate FlatForward::zeroYield(Time t,
                                           bool extrapolate) const {
            return forward_;
        }
        
        inline DiscountFactor FlatForward::discount(Time t,
                                                    bool extrapolate) const {
            return DiscountFactor(QL_EXP(-forward_*t));
        }

        inline Rate FlatForward::forward(Time t,
                                         bool extrapolate) const {
            return forward_;
        }


    }

}


#endif
