
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/CashFlows/inarrearindexedcoupon.hpp>

namespace QuantLib {

    void InArrearIndexedCoupon::setCapletVolatility(
                                 const Handle<CapletVolatilityStructure>& v) {
        if (capletVolatility_)
            unregisterWith(capletVolatility_);
        capletVolatility_ = v;
        if (capletVolatility_)
            registerWith(capletVolatility_);
        notifyObservers();
    }

    Rate InArrearIndexedCoupon::convexityAdjustment(Rate f0) const {
        if (capletVolatility_.empty()) {
            return 0.0;
        } else {
            // see Hull, 4th ed., page 550
            Date d1 = fixingDate(),
                 referenceDate = capletVolatility_->referenceDate();
            if (d1 <= referenceDate) {
                return 0.0;
            } else {
                Volatility sigma = capletVolatility_->volatility(d1,f0);
                DayCounter dayCount = capletVolatility_->dayCounter();
                Date d2 = xibor_->calendar().advance(
                                             d1, xibor_->tenor(),
                                             xibor_->businessDayConvention());
                Time tau = dayCount.yearFraction(d1,d2);
                Real variance = sigma*sigma*tau;

                Time t1 = dayCount.yearFraction(referenceDate,d1);
                return f0*f0*variance*t1/(1.0+f0*tau);
            }
        }
    }

}

