/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ferdinando Ametrano

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

/*! \file makeois.hpp
    \brief Helper class to instantiate overnight indexed swaps.
*/

#ifndef quantlib_makeois_hpp
#define quantlib_makeois_hpp

#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>

namespace QuantLib {

    //! helper class
    /*! This class provides a more comfortable way
        to instantiate overnight indexed swaps.
    */
    class MakeOIS {
      public:
        MakeOIS(const Period& swapTenor,
                const boost::shared_ptr<OvernightIndex>& overnightIndex,
                Rate fixedRate = Null<Rate>(),
                const Period& fwdStart = 0*Days);

        operator OvernightIndexedSwap() const;
        operator boost::shared_ptr<OvernightIndexedSwap>() const ;

        MakeOIS& receiveFixed(bool flag = true);
        MakeOIS& withType(OvernightIndexedSwap::Type type);
        MakeOIS& withNominal(Real n);

        MakeOIS& withSettlementDays(Natural settlementDays);
        MakeOIS& withEffectiveDate(const Date&);
        MakeOIS& withTerminationDate(const Date&);
        MakeOIS& withRule(DateGeneration::Rule r);

        MakeOIS& withPaymentFrequency(Frequency f);
        MakeOIS& withEndOfMonth(bool flag = true);

        MakeOIS& withFixedLegDayCount(const DayCounter& dc);

        MakeOIS& withOvernightLegSpread(Spread sp);

        MakeOIS& withDiscountingTermStructure(
                  const Handle<YieldTermStructure>& discountingTermStructure);
        MakeOIS& withPricingEngine(
                              const boost::shared_ptr<PricingEngine>& engine);
      private:
        Period swapTenor_;
        boost::shared_ptr<OvernightIndex> overnightIndex_;
        Rate fixedRate_;
        Period forwardStart_;

        Natural settlementDays_;
        Date effectiveDate_, terminationDate_;
        Calendar calendar_;

        Frequency paymentFrequency_;
        DateGeneration::Rule rule_;
        bool endOfMonth_, isDefaultEOM_;

        OvernightIndexedSwap::Type type_;
        Real nominal_;

        Spread overnightSpread_;
        DayCounter fixedDayCount_;

        boost::shared_ptr<PricingEngine> engine_;
    };

}

#endif
