
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

/*! \file piecewiseconstantforwards.hpp
    \brief piecewise constant forward rate term structure

    \fullpath
    Include/ql/TermStructures/%piecewiseconstantforwards.hpp
*/

// $Id$
// $Log$
// Revision 1.1  2001/09/03 14:08:42  nando
// source (*.hpp and *.cpp) moved under topdir/ql
//
// Revision 1.13  2001/08/09 14:59:47  sigmud
// header modification
//
// Revision 1.12  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.11  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.10  2001/07/24 16:59:34  nando
// documentation revised
//
// Revision 1.9  2001/06/25 10:04:01  nando
// R019-branch-merge5 merged into trunk
//


#ifndef quantlib_piecewise_constant_forward_curve_h
#define quantlib_piecewise_constant_forward_curve_h

#include "ql/termstructure.hpp"
#include "ql/depositrate.hpp"

namespace QuantLib {

    namespace TermStructures {

        /*!
        \class PiecewiseConstantForwards \
            ql/TermStructures/piecewiseconstantforwards.hpp
        \ingroup deprecated

        \deprecated
        the PiecewiseFlatForward class should be used instead.
        */
        class PiecewiseConstantForwards : public TermStructure {
          public:
            // constructor
            PiecewiseConstantForwards(Currency currency,
                const Handle<DayCounter>& dayCounter, const Date& todaysDate,
                const Handle<Calendar>& calendar, int settlementDays,
                const std::vector<DepositRate>& deposits);
            // inspectors
            Currency currency() const;
            Handle<DayCounter> dayCounter() const;
            Date todaysDate() const;
            int settlementDays() const;
            Handle<Calendar> calendar() const;
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
            // methods
            int nextNode_(const Date& d, bool extrapolate) const;
            // data members
            Currency currency_;
            Handle<DayCounter> dayCounter_;
            Date todaysDate_;
            Handle<Calendar> calendar_;
            int settlementDays_;
            Date settlementDate_;
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

        inline Currency PiecewiseConstantForwards::currency() const {
            return currency_;
        }

        inline Handle<DayCounter> PiecewiseConstantForwards::dayCounter() const{
            return dayCounter_;
        }

        inline Date PiecewiseConstantForwards::todaysDate() const {
            return todaysDate_;
        }

        inline Handle<Calendar> PiecewiseConstantForwards::calendar() const {
            return calendar_;
        }

        inline int PiecewiseConstantForwards::settlementDays() const {
            return settlementDays_;
        }

        inline Date PiecewiseConstantForwards::settlementDate() const {
            return settlementDate_;
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
