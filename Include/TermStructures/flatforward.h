
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file flatforward.h
    \brief flat forward rate term structure

    $Source$
    $Name$
    $Log$
    Revision 1.8  2001/03/07 17:32:42  nando
    more complete error message

    Revision 1.7  2001/01/18 14:36:30  nando
    80 columns enforced
    private members with trailing underscore

    Revision 1.6  2001/01/18 13:18:50  nando
    now term structure allows extrapolation

    Revision 1.5  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.4  2000/12/27 14:05:56  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

    Revision 1.3  2000/12/14 12:40:13  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_flat_forward_curve_h
#define quantlib_flat_forward_curve_h

#include "qldefines.h"
#include "termstructure.h"

namespace QuantLib {

    namespace TermStructures {

        class FlatForward : public TermStructure {
          public:
            // constructor
            FlatForward(const Handle<Currency>& currency,
                        const Handle<DayCounter>& dayCounter,
                        const Date& today,
                        Rate forward);
            // clone
            Handle<TermStructure> clone() const;
            // inspectors
            Handle<Currency> currency() const;
            Handle<DayCounter> dayCounter() const;
            Date todaysDate() const;
            Date settlementDate() const;
            Handle<Calendar> calendar() const;
            Date maxDate() const;
            Date minDate() const;
            // zero yield
            Rate zeroYield(const Date&, bool extrapolate = false) const;
            // discount
            DiscountFactor discount(const Date&,
                                    bool extrapolate = false) const;
            // forward (instantaneous)
            Rate forward(const Date&, bool extrapolate = false) const;
          private:
            Handle<Currency> currency_;
            Handle<DayCounter> dayCounter_;
            Date today_;
            Rate forward_;
        };

        // inline definitions

        inline FlatForward::FlatForward(const Handle<Currency>& currency,
                                        const Handle<DayCounter>& dayCounter,
                                        const Date& today,
                                        Rate forward)
        : currency_(currency), dayCounter_(dayCounter), today_(today),
          forward_(forward) {}

        inline Handle<TermStructure> FlatForward::clone() const {
            return Handle<TermStructure>(new FlatForward(currency_,
                                                         dayCounter_,
                                                         today_,
                                                         forward_));
        }

        inline Handle<Currency> FlatForward::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> FlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date FlatForward::todaysDate() const {
            return today_;
        }

        inline Date FlatForward::settlementDate() const {
            return currency_->settlementDate(today_);
        }

        inline Handle<Calendar> FlatForward::calendar() const {
            return currency_->settlementCalendar();
        }

        inline Date FlatForward::maxDate() const {
            return Date::maxDate();
        }

        inline Date FlatForward::minDate() const {
            return settlementDate();
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
            double t = dayCounter_->yearFraction(settlementDate(),d);
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

    }

}


#endif
