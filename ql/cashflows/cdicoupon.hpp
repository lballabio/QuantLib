/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Roland Lichters
 Copyright (C) 2009 Ferdinando Ametrano
 Copyright (C) 2014 Peter Caspers
 Copyright (C) 2017 Joseph Jeisman
 Copyright (C) 2017 Fabrice Lecuyer

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cdicoupon.hpp
    \brief Overnight indexed coupon accruing the Brazilian CDI rate
*/

#ifndef quantlib_cdi_coupon_hpp
#define quantlib_cdi_coupon_hpp

#include <ql/cashflows/overnightindexedcoupon.hpp>
#include <ql/indexes/ibor/cdi.hpp>
#include <ql/time/daycounters/business252.hpp>

namespace QuantLib {

    //! overnight cdi coupon
    /*! %Overnight indexed coupon paying the Brazilian overnight rate (CDI),
     * with accrual conventions specific to Brazilian Zero Coupon Swaps.
     * See: https://www.cmegroup.com/education/files/otc-irs-brl-overview.pdf
     * The implementation is provided by the compounding overnight coupon pricer
     * The following conventions are used
     *  1) CDI ON compound factor with spread s: [(1+cdi_i)*(1+s)]^(1/252)
     *  2) CDI ON compound factor with gearing ("DI over") g: [(1+cdi_i)^(1/252)-1] * g + 1
     *     If gearing is nontrivial, then the telescopic formula cannot be applied.
     */

    class CdiCoupon : public OvernightIndexedCoupon {
      public:
        CdiCoupon(const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const ext::shared_ptr<Cdi>& cdiIndex,
                  Real gearing = 1.0,
                  Spread spread = 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const DayCounter& dayCounter = Business252(),
                  bool telescopicValueDates = false,
                  Natural lookbackDays = Null<Natural>(),
                  Natural lockoutDays = 0,
                  bool applyObservationShift = false)
        : OvernightIndexedCoupon(paymentDate,
                                 nominal,
                                 startDate,
                                 endDate,
                                 cdiIndex,
                                 gearing,
                                 spread,
                                 refPeriodStart,
                                 refPeriodEnd,
                                 dayCounter,
                                 telescopicValueDates,
                                 RateAveraging::Compound,
                                 lookbackDays,
                                 lockoutDays,
                                 applyObservationShift) {}

        bool isCdiIndexed() const override { return true; }

        // Explanation for this...
        bool canApplyTelescopicFormula() const override {
            const bool canApply = OvernightIndexedCoupon::canApplyTelescopicFormula();
            return canApply;
            // return canApply && (gearing_ == 1.0); // check for this in a proper way
        }
    };

}


#endif