
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

/*! \file parcoupon.cpp
    \brief Coupon at par on a term structure

    $Id$
    $Source$
    $Log$
    Revision 1.1  2001/06/01 16:50:16  lballabio
    Term structure on deposits and swaps

*/

#include "ql/CashFlows/parcoupon.hpp"
#include "ql/dataformatters.hpp"
#include "ql/Indexes/libormanager.hpp"

namespace QuantLib {

    using Indexes::LiborManager;
    using Indexes::Xibor;
    
    namespace CashFlows {
        
        ParCoupon::ParCoupon(double nominal, const Xibor& index, 
            int n, TimeUnit unit, Spread spread, 
            const Handle<Calendar>& calendar, 
            const Handle<DayCounter>& dayCounter,
            const RelinkableHandle<TermStructure>& termStructure,
            const Date& startDate, const Date& endDate, 
            const Date& refPeriodStart, const Date& refPeriodEnd)
        : AccruingCoupon(calendar, Following, dayCounter, 
          startDate, endDate, refPeriodStart, refPeriodEnd), 
          nominal_(nominal), spread_(spread), index_(index), 
          n_(n), unit_(unit), termStructure_(termStructure) {
            QL_REQUIRE(calendar->isBusinessDay(startDate),
                "Start date for par coupon (" + 
                DateFormatter::toString(startDate) +
                ") is holiday for " +
                calendar->name() + " calendar");
            QL_REQUIRE(calendar->isBusinessDay(endDate),
                "End date for par coupon (" + 
                DateFormatter::toString(endDate) +
                ") is holiday for " +
                calendar->name() + " calendar");
        }

        double ParCoupon::amount() const {
            QL_REQUIRE(!termStructure_.isNull(),
                "null term structure set to par coupon");

            Date settlementDate = termStructure_->settlementDate();
            if (startDate_ < settlementDate) {
                // must have been fixed
                Rate pastFixing =LiborManager::getHistory(
                    index_.name(),n_,unit_)[startDate_];
                QL_REQUIRE(pastFixing != Null<double>(),
                    "Missing " + index_.name() + " fixing for " +
                        DateFormatter::toString(startDate_));
                return (pastFixing+spread_)*accrualPeriod()*nominal_;
            }
            if (startDate_ == settlementDate) {
                // might have been fixed
                try {
                    Rate pastFixing = LiborManager::getHistory(
                        index_.name(),n_,unit_)[startDate_];
                    if (pastFixing != Null<double>())
                        return (pastFixing+spread_)*accrualPeriod()*nominal_;
                    else
                        ;   // fall through and forecast
                } catch (Error&) {
                    ;       // fall through and forecast
                }
            }
            DiscountFactor startDiscount =
                termStructure_->discount(startDate_);
            DiscountFactor endDiscount =
                termStructure_->discount(endDate_);
            return ((startDiscount/endDiscount-1.0) + 
                    spread_*accrualPeriod()) * nominal_;
        }

    }

}
