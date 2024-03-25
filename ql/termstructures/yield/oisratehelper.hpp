/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009, 2012 Roland Lichters
 Copyright (C) 2009, 2012 Ferdinando Ametrano

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

/*! \file oisratehelper.hpp
    \brief Overnight Indexed Swap (aka OIS) rate helpers
*/

#ifndef quantlib_oisratehelper_hpp
#define quantlib_oisratehelper_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/instruments/overnightindexedswap.hpp>
#include <ql/optional.hpp>

namespace QuantLib {
    namespace detail {
        class OISRateHelperBase : public SwapRateHelperBase {
          public:
            OISRateHelperBase(const Handle<Quote>& fixedRate,
                              const ext::shared_ptr<OvernightIndex>& overnightIndex,
                              Handle<YieldTermStructure> discountingCurve = {},
                              Pillar::Choice pillar = Pillar::LastRelevantDate,
                              Date customPillarDate = Date());
            //! \name RateHelper interface
            //@{
            Real impliedQuote() const override;
            //@}
            //! \name OISRateHelperBase inspectors
            //@{
            // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
            ext::shared_ptr<OvernightIndexedSwap> swap() const {
                return ext::static_pointer_cast<OvernightIndexedSwap>(swap_);
            }
            //@}
          protected:
            ext::shared_ptr<OvernightIndex> overnightIndex() const {
                return ext::static_pointer_cast<OvernightIndex>(iborIndex_);
            }
        };
    }

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class OISRateHelper : public RelativeDateBootstrapHelper<YieldTermStructure,
                                                             detail::OISRateHelperBase> {
        typedef RelativeDateBootstrapHelper<YieldTermStructure, detail::OISRateHelperBase>
            base_type;
      public:
        OISRateHelper(Natural settlementDays,
                      const Period& tenor, // swap maturity
                      const Handle<Quote>& fixedRate,
                      const ext::shared_ptr<OvernightIndex>& overnightIndex,
                      // exogenous discounting curve
                      Handle<YieldTermStructure> discountingCurve = {},
                      bool telescopicValueDates = false,
                      Integer paymentLag = 0,
                      BusinessDayConvention paymentConvention = Following,
                      Frequency paymentFrequency = Annual,
                      Calendar paymentCalendar = Calendar(),
                      const Period& forwardStart = 0 * Days,
                      Spread overnightSpread = 0.0,
                      Pillar::Choice pillar = Pillar::LastRelevantDate,
                      Date customPillarDate = Date(),
                      RateAveraging::Type averagingMethod = RateAveraging::Compound,
                      ext::optional<bool> endOfMonth = ext::nullopt,
                      ext::optional<Frequency> fixedPaymentFrequency = ext::nullopt,
                      Calendar fixedCalendar = Calendar());
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    protected:
      void initializeDates() override;

      Natural settlementDays_;
      Period tenor_;
      bool telescopicValueDates_;
      Integer paymentLag_;
      BusinessDayConvention paymentConvention_;
      Frequency paymentFrequency_;
      Calendar paymentCalendar_;
      Period forwardStart_;
      Spread overnightSpread_;
      RateAveraging::Type averagingMethod_;
      ext::optional<bool> endOfMonth_;
      ext::optional<Frequency> fixedPaymentFrequency_;
      Calendar fixedCalendar_;
    };

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class DatedOISRateHelper : public detail::OISRateHelperBase {
        typedef detail::OISRateHelperBase base_type;
      public:
        DatedOISRateHelper(const Date& startDate,
                           const Date& endDate,
                           const Handle<Quote>& fixedRate,
                           const ext::shared_ptr<OvernightIndex>& overnightIndex,
                           // exogenous discounting curve
                           Handle<YieldTermStructure> discountingCurve = {},
                           bool telescopicValueDates = false,
                           RateAveraging::Type averagingMethod = RateAveraging::Compound,
                           Integer paymentLag = 0,
                           BusinessDayConvention paymentConvention = Following,
                           Frequency paymentFrequency = Annual,
                           const Calendar& paymentCalendar = Calendar(),
                           const Period& forwardStart = 0 * Days,
                           Spread overnightSpread = 0.0,
                           ext::optional<bool> endOfMonth = ext::nullopt,
                           ext::optional<Frequency> fixedPaymentFrequency = ext::nullopt,
                           const Calendar& fixedCalendar = Calendar(),
                           Pillar::Choice pillar = Pillar::LastRelevantDate,
                           Date customPillarDate = Date());
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
    };

}

#endif
