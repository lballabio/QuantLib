
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*! \file flatforward.hpp
    \brief flat forward rate term structure

    $Source$
    $Name$
    $Log$
    Revision 1.4  2001/05/24 11:34:07  nando
    smoothing #include xx.hpp

    Revision 1.3  2001/05/24 11:15:57  lballabio
    Stripped conventions from Currencies

    Revision 1.2  2001/05/14 17:09:47  lballabio
    Went for simplicity and removed Observer-Observable relationships from Instrument

*/

#ifndef quantlib_flat_forward_curve_h
#define quantlib_flat_forward_curve_h

#include "ql/termstructure.hpp"
#include "ql/dataformatters.hpp"

namespace QuantLib {

    namespace TermStructures {

        class FlatForward : public TermStructure {
          public:
            // constructor
            FlatForward(Currency currency, const Handle<DayCounter>& dayCounter,
                const Date& settlementDate, Rate forward);
            // inspectors
            Currency currency() const;
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
          private:
            Currency currency_;
            Handle<DayCounter> dayCounter_;
            Date settlementDate_;
            Rate forward_;
        };

        // inline definitions

        inline FlatForward::FlatForward(Currency currency,
            const Handle<DayCounter>& dayCounter, const Date& settlementDate,
            Rate forward)
        : currency_(currency), dayCounter_(dayCounter), 
          settlementDate_(settlementDate), forward_(forward) {}

        inline Currency FlatForward::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> FlatForward::dayCounter() const {
            return dayCounter_;
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

    }

}


#endif
