/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci
 Copyright (C) 2006 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <ql/cashflows/cmscoupon.hpp>
#include <ql/indexes/swapindex.hpp>

namespace QuantLib {

    CmsCoupon::CmsCoupon(
                    const Date& paymentDate,
                    const Real nominal,
                    const Date& startDate, 
                    const Date& endDate,
                    const Natural fixingDays,
                    const boost::shared_ptr<SwapIndex>& index,
                    const Real gearing,
                    const Spread spread,
                    const Date& refPeriodStart,
                    const Date& refPeriodEnd,
                    const DayCounter& dayCounter,
                    bool isInArrears)
    : FloatingRateCoupon(paymentDate, nominal, startDate, endDate,
                         fixingDays, index, gearing, spread, 
                         refPeriodStart, refPeriodEnd, dayCounter, isInArrears),
      swapIndex_(index){}

    void CmsCoupon::accept(AcyclicVisitor& v) {
        Visitor<CmsCoupon>* v1 = dynamic_cast<Visitor<CmsCoupon>*>(&v);
        if (v1 != 0)
            v1->visit(*this);
        else
            FloatingRateCoupon::accept(v);
    }

}
