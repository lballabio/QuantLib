
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

/*! \file piecewiseconstantforwards.hpp
    \brief piecewise constant forward rate term structure

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:23  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.8  2001/01/18 16:22:18  nando
    deposit file and class renamed to DepositRate

    Revision 1.7  2001/01/18 14:36:30  nando
    80 columns enforced
    private members with trailing underscore

    Revision 1.6  2001/01/18 13:18:50  nando
    now term structure allows extrapolation

    Revision 1.5  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.4  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks

*/

#ifndef quantlib_piecewise_constant_forward_curve_h
#define quantlib_piecewise_constant_forward_curve_h

#include "qldefines.hpp"
#include "termstructure.hpp"
#include "depositrate.hpp"
#include <vector>

namespace QuantLib {

    namespace TermStructures {

        class PiecewiseConstantForwards : public TermStructure {
          public:
            // constructor
            PiecewiseConstantForwards(Handle<Currency> currency,
                                      Handle<DayCounter> dayCounter,
                                      const Date& today,
                                      const std::vector<DepositRate>& deposits);
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
            // methods
            int nextNode_(const Date& d, bool extrapolate) const;
            // data members
            Handle<Currency> currency_;
            Handle<DayCounter> dayCounter_;
            Date today_;
            int nodesNumber_;
            std::vector<Date> nodes_;
            std::vector<Time> times_;
            std::vector<DiscountFactor> discounts_;
            std::vector<Rate> forwards_, zeroYields_;
            // here to be used in the clone method
            // to be replaced by an observable pattern
            std::vector<DepositRate> deposits_;
        };

        // inline definitions

        inline Handle<TermStructure> PiecewiseConstantForwards::clone() const {
            return Handle<TermStructure>(new PiecewiseConstantForwards(
                                                                 currency_,
                                                                 dayCounter_,
                                                                 today_,
                                                                 deposits_));
        }

        inline Handle<Currency> PiecewiseConstantForwards::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> PiecewiseConstantForwards::dayCounter() const{
            return dayCounter_;
        }

        inline Date PiecewiseConstantForwards::todaysDate() const {
            return today_;
        }

        inline Date PiecewiseConstantForwards::settlementDate() const {
            return currency_->settlementDate(today_);
        }

        inline Handle<Calendar> PiecewiseConstantForwards::calendar() const {
            return currency_->settlementCalendar();
        }

        inline Date PiecewiseConstantForwards::maxDate() const {
            return nodes_.back();
        }

        inline Date PiecewiseConstantForwards::minDate() const {
            return settlementDate();
        }

    }

}


#endif
