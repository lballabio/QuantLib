/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl
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

/*! \file basisswapratehelpers.hpp
    \brief ibor-ibor and ois-ibor basis swap rate helpers
*/

#ifndef quantlib_basisswapratehelpers_hpp
#define quantlib_basisswapratehelpers_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/dategenerationrule.hpp>
#include <optional>

namespace QuantLib {

    //! Rate helper for bootstrapping over ibor-ibor basis swaps
    /*! The swap is assumed to pay baseIndex + basis and receive
        otherIndex.  The helper can be used to bootstrap the forecast
        curve for baseIndex (in which case you'll have to pass
        bootstrapBaseCurve = true and provide otherIndex with a
        forecast curve) or the forecast curve for otherIndex (in which
        case bootstrapBaseCurve = false and baseIndex will need a
        forecast curve).
        In both cases, an exogenous discount curve is required.
    */
    class IborIborBasisSwapRateHelper : public RelativeDateRateHelper {
      public:
        IborIborBasisSwapRateHelper(const Handle<Quote>& basis,
                                    const Period& tenor,
                                    Natural settlementDays,
                                    Calendar calendar,
                                    BusinessDayConvention convention,
                                    bool endOfMonth,
                                    const ext::shared_ptr<IborIndex>& baseIndex,
                                    const ext::shared_ptr<IborIndex>& otherIndex,
                                    Handle<YieldTermStructure> discountHandle,
                                    bool bootstrapBaseCurve,
                                    std::optional<bool> useIndexedCoupons = std::nullopt,
                                    DateGeneration::Rule rule = DateGeneration::Backward);

        Real impliedQuote() const override;
        void accept(AcyclicVisitor&) override;
        // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
        ext::shared_ptr<Swap> swap() const { return swap_; }
      private:
        void initializeDates() override;
        void setTermStructure(YieldTermStructure*) override;

        Period tenor_;
        Natural settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        ext::shared_ptr<IborIndex> baseIndex_;
        ext::shared_ptr<IborIndex> otherIndex_;
        Handle<YieldTermStructure> discountHandle_;
        bool bootstrapBaseCurve_;
        std::optional<bool> useIndexedCoupons_;
        DateGeneration::Rule rule_;

        ext::shared_ptr<Swap> swap_;

        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };


    //! Rate helper for bootstrapping over overnight-ibor basis swaps
    /*! The swap is assumed to pay overnight + basis and receive ibor.
        As a default, the helper is used to bootstrap the forecast
        curve for the ibor index; the overnight index will need an
        existing forecast curve. If bootstrapBaseCurve is set to true,
        instead, the helper will be used to bootstrap the forecast
        curve for the overnight index; in this case, the ibor index
        will need to be given a forecast curve.

        An exogenous discount curve can be passed; if not, the
        overnight-index curve will be used.  Note that when
        bootstrapBaseCurve = true and no discount curve is passed, the
        curve being bootstrapped is also used for discounting.

        A payment lag can also be passed; it is applied to both legs.

        The payment frequency of each leg can be overridden
        independently; both default to the tenor of the ibor index.
        Once pays a single coupon at maturity, whatever the rule.
    */
    class OvernightIborBasisSwapRateHelper : public RelativeDateRateHelper {
      public:
        OvernightIborBasisSwapRateHelper(const Handle<Quote>& basis,
                                         const Period& tenor,
                                         Natural settlementDays,
                                         Calendar calendar,
                                         BusinessDayConvention convention,
                                         bool endOfMonth,
                                         const ext::shared_ptr<OvernightIndex>& baseIndex,
                                         const ext::shared_ptr<IborIndex>& otherIndex,
                                         Handle<YieldTermStructure> discountHandle = Handle<YieldTermStructure>(),
                                         bool bootstrapBaseCurve = false,
                                         Integer paymentLag = 0,
                                         std::optional<Frequency> overnightPaymentFrequency = std::nullopt,
                                         std::optional<Frequency> iborPaymentFrequency = std::nullopt,
                                         std::optional<bool> useIndexedCoupons = std::nullopt,
                                         DateGeneration::Rule rule = DateGeneration::Backward);

        Real impliedQuote() const override;
        void accept(AcyclicVisitor&) override;
        // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
        ext::shared_ptr<Swap> swap() const { return swap_; }
      private:
        void initializeDates() override;
        void setTermStructure(YieldTermStructure*) override;

        Period tenor_;
        Natural settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        ext::shared_ptr<OvernightIndex> baseIndex_;
        ext::shared_ptr<IborIndex> otherIndex_;
        Handle<YieldTermStructure> discountHandle_;
        bool bootstrapBaseCurve_;
        Integer paymentLag_;
        std::optional<Frequency> overnightPaymentFrequency_;
        std::optional<Frequency> iborPaymentFrequency_;
        std::optional<bool> useIndexedCoupons_;
        DateGeneration::Rule rule_;

        ext::shared_ptr<Swap> swap_;

        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

}

#endif
