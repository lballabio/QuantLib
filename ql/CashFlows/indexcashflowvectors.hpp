
/*
 Copyright (C) 2003 Nicolas Di Césaré
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

/*! \file indexcashflowvectors.hpp
    \brief Index Cash flow vector builders
*/

#ifndef quantlib_index_cash_flow_vectors_hpp
#define quantlib_index_cash_flow_vectors_hpp

#include <ql/CashFlows/shortindexedcoupon.hpp>
#include <ql/schedule.hpp>

namespace QuantLib {

    //! helper function building a leg of floating coupons
    /*! Either ParCoupon, UpFrontIndexedCoupon, InArrearIndexedCoupon,
        or any other coupon can be used whose constructor takes the
        same arguments.

        \warning The last argument is used due to a known VC++ bug
                 regarding function template instantiation. It must be
                 passed explicitly when using the function with that
                 compiler; the simplest choice for the value to be
                 passed is <tt>(const Type*) 0</tt> where
                 <tt>Type</tt> is the desired coupon type.
    */
    template <class IndexedCouponType>
    std::vector<boost::shared_ptr<CashFlow> >
    IndexedCouponVector(const Schedule& schedule,
                        BusinessDayConvention paymentAdjustment,
                        const std::vector<Real>& nominals,
                        const boost::shared_ptr<Xibor>& index,
                        Integer fixingDays,
                        const std::vector<Spread>& spreads,
                        const DayCounter& dayCounter = DayCounter()
                        #ifdef QL_PATCH_MSVC6
                        , const IndexedCouponType* msvc6_bug = 0
                        #endif
                        ) {

        QL_REQUIRE(nominals.size() != 0, "unspecified nominals");
        QL_REQUIRE(paymentAdjustment != Unadjusted,
                   "invalid business-day convention "
                   "for payment-date adjustment");

        std::vector<boost::shared_ptr<CashFlow> > leg;
        // first period might be short or long
        Date start = schedule.date(0), end = schedule.date(1);
        Calendar calendar = schedule.calendar();
        Date paymentDate = calendar.adjust(end,paymentAdjustment);
        Spread spread;
        if (spreads.size() > 0)
            spread = spreads[0];
        else
            spread = 0.0;
        Real nominal = nominals[0];
        if (schedule.isRegular(1)) {
            leg.push_back(boost::shared_ptr<CashFlow>(
                new IndexedCouponType(nominal, paymentDate, index,
                                      start, end, fixingDays, spread,
                                      start, end, dayCounter)));
        } else {
            Integer tenor = 12/schedule.frequency();
            Date reference = end - tenor*Months;
            reference = calendar.adjust(reference,
                                        schedule.businessDayConvention());
            typedef Short<IndexedCouponType> ShortIndexedCouponType;
            leg.push_back(boost::shared_ptr<CashFlow>(
                new ShortIndexedCouponType(nominal, paymentDate, index,
                                           start, end, fixingDays, spread,
                                           reference, end, dayCounter)));
        }
        // regular periods
        for (Size i=2; i<schedule.size()-1; i++) {
            start = end; end = schedule.date(i);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if ((i-1) < spreads.size())
                spread = spreads[i-1];
            else if (spreads.size() > 0)
                spread = spreads.back();
            else
                spread = 0.0;
            if ((i-1) < nominals.size())
                nominal = nominals[i-1];
            else
                nominal = nominals.back();
            leg.push_back(boost::shared_ptr<CashFlow>(
                new IndexedCouponType(nominal, paymentDate, index,
                                      start, end, fixingDays, spread,
                                      start, end, dayCounter)));
        }
        if (schedule.size() > 2) {
            // last period might be short or long
            Size N = schedule.size();
            start = end; end = schedule.date(N-1);
            paymentDate = calendar.adjust(end,paymentAdjustment);
            if ((N-2) < spreads.size())
                spread = spreads[N-2];
            else if (spreads.size() > 0)
                spread = spreads.back();
            else
                spread = 0.0;
            if ((N-2) < nominals.size())
                nominal = nominals[N-2];
            else
                nominal = nominals.back();
            if (schedule.isRegular(N-1)) {
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new IndexedCouponType(nominal, paymentDate, index,
                                          start, end, fixingDays, spread,
                                          start, end, dayCounter)));
            } else {
                Integer tenor = 12/schedule.frequency();
                Date reference = start + tenor*Months;
                reference = calendar.adjust(reference,
                                            schedule.businessDayConvention());
                typedef Short<IndexedCouponType> ShortIndexedCouponType;
                leg.push_back(boost::shared_ptr<CashFlow>(
                    new ShortIndexedCouponType(nominal, paymentDate, index,
                                               start, end, fixingDays,
                                               spread, start, reference,
                                               dayCounter)));
            }
        }
        return leg;
    }

}


#endif
