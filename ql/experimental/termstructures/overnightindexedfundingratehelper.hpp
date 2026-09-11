/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Kyrylo Protsenko

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file overnightindexedfundingratehelper.hpp
    \brief rate helper for funding margins over compounded overnight rates
*/

#ifndef quantlib_overnight_indexed_funding_rate_helper_hpp
#define quantlib_overnight_indexed_funding_rate_helper_hpp

#include <ql/instruments/swap.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/dategenerationrule.hpp>

namespace QuantLib {

    class OvernightIndex;

    //! Rate helper for funding margins over a compounded overnight rate
    /*! The underlying instrument exchanges principal at the start and end
        and pays the overnight rate plus the quoted margin periodically. All
        payments, including the principal exchanges, use the same payment lag.

        The overnight rate is projected from an exogenous curve. The curve
        being bootstrapped discounts all cash flows.
    */
    class OvernightIndexedFundingRateHelper : public RelativeDateRateHelper {
      public:
        OvernightIndexedFundingRateHelper(const std::variant<Spread, Handle<Quote>>& margin,
                                          const Period& tenor,
                                          Natural settlementDays,
                                          Calendar calendar,
                                          BusinessDayConvention convention,
                                          bool endOfMonth,
                                          const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                          const Period& paymentTenor,
                                          DayCounter paymentDayCounter,
                                          Integer paymentLag = 0,
                                          bool telescopicValueDates = false,
                                          DateGeneration::Rule rule = DateGeneration::Backward,
                                          Pillar::Choice pillar = Pillar::LastRelevantDate,
                                          Date customPillarDate = Date());

        OvernightIndexedFundingRateHelper(const std::variant<Spread, Handle<Quote>>& margin,
                                          const Date& startDate,
                                          const Date& endDate,
                                          Calendar calendar,
                                          BusinessDayConvention convention,
                                          bool endOfMonth,
                                          const ext::shared_ptr<OvernightIndex>& overnightIndex,
                                          const Period& paymentTenor,
                                          DayCounter paymentDayCounter,
                                          Integer paymentLag = 0,
                                          bool telescopicValueDates = false,
                                          DateGeneration::Rule rule = DateGeneration::Backward,
                                          Pillar::Choice pillar = Pillar::LastRelevantDate,
                                          Date customPillarDate = Date());

        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        void accept(AcyclicVisitor&) override;

        // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
        ext::shared_ptr<Swap> swap() const { return swap_; }

      private:
        void initialize(const ext::shared_ptr<OvernightIndex>& overnightIndex,
                        Date customPillarDate);
        void initializeDates() override;

        Period tenor_;
        Natural settlementDays_;
        Date startDate_, endDate_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Period paymentTenor_;
        DayCounter paymentDayCounter_;
        Integer paymentLag_;
        bool telescopicValueDates_;
        DateGeneration::Rule rule_;
        Pillar::Choice pillarChoice_;

        ext::shared_ptr<Swap> swap_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;
    };

}

#endif
