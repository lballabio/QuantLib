
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

/*! \file piecewiseflatforward.hpp
    \brief piecewise flat forward term structure

    $Source$
    $Log$
    Revision 1.1  2001/05/16 09:57:27  lballabio
    Added indexes and piecewise flat forward curve

*/

#ifndef quantlib_piecewise_flat_forward_curve_h
#define quantlib_piecewise_flat_forward_curve_h

#include "ql/qldefines.hpp"
#include "ql/TermStructures/ratehelpers.hpp"
#include "ql/solver1d.hpp"

namespace QuantLib {

    namespace TermStructures {

        /* derived directly from term structure since we are rewriting all of 
           forward, discount and zeroYield to take advantage of the internal 
           structure. */
        class PiecewiseFlatForward : public TermStructure {
          public:
            // constructor
            PiecewiseFlatForward(const Handle<Currency>& currency,
                const Handle<DayCounter>& dayCounter, const Date& today,
                const std::vector<Handle<RateHelper> >& instruments);
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
            // inner classes
            // objective function for solver
            class FFObjFunction;
            friend class FFObjFunction;
            class FFObjFunction : public ObjectiveFunction {
              public:
                FFObjFunction(PiecewiseFlatForward*,
                    const Handle<RateHelper>&, int segment);
                double operator()(double discountGuess) const;
              private:
                PiecewiseFlatForward* curve_;
                Handle<RateHelper> rateHelper_;
                int segment_;
            };
            // instrument sorter
            class RateHelperSorter {
              public:
                bool operator()(const Handle<RateHelper>&,
                                const Handle<RateHelper>&) const;
            };
            // methods
            int referenceNode(const Date& d, bool extrapolate) const;
            // data members
            Handle<Currency> currency_;
            Handle<DayCounter> dayCounter_;
            Date today_;
            std::vector<Date> nodes_;
            std::vector<Time> times_;
            std::vector<DiscountFactor> discounts_;
            std::vector<Rate> forwards_, zeroYields_;
            static const double accuracy_;
        };

        // inline definitions

        inline Handle<Currency> PiecewiseFlatForward::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> PiecewiseFlatForward::dayCounter() const {
            return dayCounter_;
        }

        inline Date PiecewiseFlatForward::todaysDate() const {
            return today_;
        }

        inline Date PiecewiseFlatForward::settlementDate() const {
            return currency_->settlementDate(today_);
        }

        inline Handle<Calendar> PiecewiseFlatForward::calendar() const {
            return currency_->settlementCalendar();
        }

        inline Date PiecewiseFlatForward::maxDate() const {
            return nodes_.back();
        }

        inline Date PiecewiseFlatForward::minDate() const {
            return settlementDate();
        }

    }

}


#endif
