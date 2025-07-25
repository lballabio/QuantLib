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
#include <variant>

namespace QuantLib {

    class FloatingRateCouponPricer;

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class OISRateHelper : public RelativeDateRateHelper {
      public:
        OISRateHelper(
          Natural settlementDays,
          const Period& tenor, // swap maturity
          const std::variant<Rate, Handle<Quote>>& fixedRate,
          const ext::shared_ptr<OvernightIndex>& overnightIndex,
          // exogenous discounting curve
          Handle<YieldTermStructure> discountingCurve = {},
          bool telescopicValueDates = false,
          Integer paymentLag = 0,
          BusinessDayConvention paymentConvention = Following,
          Frequency paymentFrequency = Annual,
          Calendar paymentCalendar = Calendar(),
          const Period& forwardStart = 0 * Days,
          const std::variant<Spread, Handle<Quote>>& overnightSpread = Spread(0.0),
          Pillar::Choice pillar = Pillar::LastRelevantDate,
          Date customPillarDate = Date(),
          RateAveraging::Type averagingMethod = RateAveraging::Compound,
          ext::optional<bool> endOfMonth = ext::nullopt,
          ext::optional<Frequency> fixedPaymentFrequency = ext::nullopt,
          Calendar fixedCalendar = Calendar(),
          Natural lookbackDays = Null<Natural>(),
          Natural lockoutDays = 0,
          bool applyObservationShift = false,
          ext::shared_ptr<FloatingRateCouponPricer> pricer = {},
          DateGeneration::Rule rule = DateGeneration::Backward,
          Calendar overnightCalendar = Calendar());

        OISRateHelper(
          const Date& startDate,
          const Date& endDate,
          const std::variant<Rate, Handle<Quote>>& fixedRate,
          const ext::shared_ptr<OvernightIndex>& overnightIndex,
          // exogenous discounting curve
          Handle<YieldTermStructure> discountingCurve = {},
          bool telescopicValueDates = false,
          Integer paymentLag = 0,
          BusinessDayConvention paymentConvention = Following,
          Frequency paymentFrequency = Annual,
          Calendar paymentCalendar = Calendar(),
          const std::variant<Spread, Handle<Quote>>& overnightSpread = Spread(0.0),
          Pillar::Choice pillar = Pillar::LastRelevantDate,
          Date customPillarDate = Date(),
          RateAveraging::Type averagingMethod = RateAveraging::Compound,
          ext::optional<bool> endOfMonth = ext::nullopt,
          ext::optional<Frequency> fixedPaymentFrequency = ext::nullopt,
          Calendar fixedCalendar = Calendar(),
          Natural lookbackDays = Null<Natural>(),
          Natural lockoutDays = 0,
          bool applyObservationShift = false,
          ext::shared_ptr<FloatingRateCouponPricer> pricer = {},
          DateGeneration::Rule rule = DateGeneration::Backward,
          Calendar overnightCalendar = Calendar());

        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name inspectors
        //@{
        // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
        ext::shared_ptr<OvernightIndexedSwap> swap() const { return swap_; }
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        void initialize(const ext::shared_ptr<OvernightIndex>& overnightIndex,
                        Date customPillarDate);
        void initializeDates() override;

        Natural settlementDays_;
        Period tenor_;
        Date startDate_, endDate_;
        ext::shared_ptr<OvernightIndex> overnightIndex_;

        ext::shared_ptr<OvernightIndexedSwap> swap_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;

        Handle<YieldTermStructure> discountHandle_;
        bool telescopicValueDates_;
        RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;

        Integer paymentLag_;
        BusinessDayConvention paymentConvention_;
        Frequency paymentFrequency_;
        Calendar paymentCalendar_;
        Period forwardStart_;
        Handle<Quote> overnightSpread_;
        Pillar::Choice pillarChoice_;
        RateAveraging::Type averagingMethod_;
        ext::optional<bool> endOfMonth_;
        ext::optional<Frequency> fixedPaymentFrequency_;
        Calendar fixedCalendar_;
        Calendar overnightCalendar_;
        Natural lookbackDays_;
        Natural lockoutDays_;
        bool applyObservationShift_;
        ext::shared_ptr<FloatingRateCouponPricer> pricer_;
        DateGeneration::Rule rule_ = DateGeneration::Backward;

    };

    /*! \deprecated Use OISRateHelper instead.
                    Deprecated in version 1.37.
    */
    class [[deprecated("Use OISRateHelper instead")]] DatedOISRateHelper : public OISRateHelper {
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
                           Spread overnightSpread = {},
                           ext::optional<bool> endOfMonth = ext::nullopt,
                           ext::optional<Frequency> fixedPaymentFrequency = ext::nullopt,
                           const Calendar& fixedCalendar = Calendar(),
                           Natural lookbackDays = Null<Natural>(),
                           Natural lockoutDays = 0,
                           bool applyObservationShift = false,
                           const ext::shared_ptr<FloatingRateCouponPricer>& pricer = {});
    };

}

#endif
