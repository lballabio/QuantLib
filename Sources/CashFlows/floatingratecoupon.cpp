
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
 * if not, contact ferdinando@ametrano.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/Authors.txt
*/

/*! \file floatingratecoupon.cpp
    \brief Coupon at par on a term structure

    \fullpath
    Sources/CashFlows/%floatingratecoupon.cpp
*/

// $Source$
// $Log$
// Revision 1.6  2001/07/25 15:47:29  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/07/24 16:59:34  nando
// documentation revised
//


#include "ql/CashFlows/floatingratecoupon.hpp"
#include "ql/Indexes/xibormanager.hpp"

namespace QuantLib {

    using Indexes::Xibor;
    using Indexes::XiborManager;

    namespace CashFlows {

        FloatingRateCoupon::FloatingRateCoupon(double nominal,
          const RelinkableHandle<TermStructure>& termStructure,
          const Date& startDate, const Date& endDate,
          const Date& refPeriodStart, const Date& refPeriodEnd,
          const Handle<Index>& index, Spread spread)
        : nominal_(nominal), termStructure_(termStructure),
          startDate_(startDate), endDate_(endDate),
          refPeriodStart_(refPeriodStart),
          refPeriodEnd_(refPeriodEnd), spread_(spread) {

            termStructure_.registerObserver(this);

            if (!index.isNull()) {
                #if QL_ALLOW_TEMPLATE_METHOD_CALLS
                const Xibor* ptr = index.downcast<Xibor>();
                #else
                const Xibor* ptr = dynamic_cast<const Xibor*>(index.pointer());
                #endif
                if (ptr != 0)
                    index_ = Handle<Xibor>(new Xibor(*ptr));
            }
        }

        FloatingRateCoupon::~FloatingRateCoupon() {
            termStructure_.unregisterObserver(this);
        }

        double FloatingRateCoupon::amount() const {
            QL_REQUIRE(!termStructure_.isNull(),
                "null term structure set to par coupon");
            Date settlementDate = termStructure_->settlementDate();
            if (startDate_ < settlementDate) {
                // must have been fixed
                QL_REQUIRE(!index_.isNull(),
                    "null or non-libor index given");
                Rate pastFixing = XiborManager::getHistory(
                    index_->name())[startDate_];
                QL_REQUIRE(pastFixing != Null<double>(),
                    "Missing " + index_->name() + " fixing for " +
                        DateFormatter::toString(startDate_));
                return (pastFixing+spread_)*accrualPeriod()*nominal_;
            }
            if (startDate_ == settlementDate) {
                // might have been fixed
                try {
                    QL_REQUIRE(!index_.isNull(),
                        "null or non-libor index given");
                    Rate pastFixing = XiborManager::getHistory(
                        index_->name())[startDate_];
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
            if (spread_ == 0.0)
                return (startDiscount/endDiscount-1.0) * nominal_;
            else
                return ((startDiscount/endDiscount-1.0) +
                    spread_*accrualPeriod()) * nominal_;
        }

        double FloatingRateCoupon::accrualPeriod() const {
            QL_REQUIRE(!index_.isNull(),
                "null or non-libor index given");
            return index_->dayCounter()->yearFraction(
                startDate_,endDate_,refPeriodStart_,refPeriodEnd_);
        }

        void FloatingRateCoupon::update() {
            notifyObservers();
        }

    }

}

