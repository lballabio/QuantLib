/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file swaptionpricer.cpp
    \brief Swaption pricer

    \fullpath
    ql/Pricers/%swaptionpricer.cpp
*/

// $Id$

#include <ql/Pricers/swaptionpricer.hpp>

namespace QuantLib {

    namespace Pricers {

        void DiscretizedSwap::adjustValues() {
            Size i;

            for (i=0; i<parameters_.fixedPayTimes.size(); i++) {
                if (isOnTime(parameters_.fixedPayTimes[i])) {
                    if (parameters_.payFixed)
                        values_ -= parameters_.fixedCoupons[i];
                    else
                        values_ += parameters_.fixedCoupons[i];
                }
            }

            for (i=0; i<parameters_.floatingResetTimes.size(); i++) {
                if (isOnTime(parameters_.floatingResetTimes[i])) {
                    Handle<DiscretizedAsset> bond(new 
                        DiscretizedDiscountBond(method()));
                    method()->initialize(bond, 
                        parameters_.floatingPayTimes[i]);
                    method()->rollback(bond,time_);

                    double nominal = parameters_.nominal;
                    for (Size j=0; j<values_.size(); j++) {
                        double coupon = nominal*(1.0 - bond->values()[j]);
                        if (parameters_.payFixed)
                            values_[j] += coupon;
                        else
                            values_[j] -= coupon;
                    }
                }
            }
        }

        void DiscretizedSwaption::adjustValues() {
            method()->rollback(swap_, time());

            if (parameters_.exerciseType != Exercise::American) {
                for (Size i=0; i<parameters_.exerciseTimes.size(); i++) {
                    if (isOnTime(parameters_.exerciseTimes[i])) {
                        applySpecificCondition();
                    }
                }
            } else {
                if (
                  (time_ >= parameters_.exerciseTimes[0]) &&
                  (time_ <= parameters_.exerciseTimes[1]))
                    applySpecificCondition();
            }
        }


    }

}
