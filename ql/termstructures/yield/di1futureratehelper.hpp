/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

/*! \file di1futureratehelper.hpp
    \brief DI1 future bootstrap helper
*/

#ifndef quantlib_di1futureratehelper_hpp
#define quantlib_di1futureratehelper_hpp

#include <ql/instruments/di1future.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <variant>

namespace QuantLib {

    //! RateHelper for bootstrapping over B3 DI1 futures
    /*! The quote is the DI1 futures rate as a decimal (e.g., 0.1325
        for a quoted rate of 13.250%), that is, the annual effective
        rate with a 252-business-day basis from the evaluation date
        to the maturity of the contract; see Di1Future for the
        details of the convention.

        The helper is meant to be used with the Cdi index, whose
        forwarding curve is the one being bootstrapped; the business
        days of the accrual period are counted with the day counter
        of the index.  Since the accrual period starts on the
        evaluation date, the earliest date of the helper moves with
        it.
    */
    class Di1FutureRateHelper : public RelativeDateRateHelper {
      public:
        Di1FutureRateHelper(const std::variant<Rate, Handle<Quote>>& rate,
                            const Date& maturityDate,
                            const ext::shared_ptr<OvernightIndex>& overnightIndex,
                            const std::variant<Rate, Handle<Quote>>& convexityAdjustment = 0.0,
                            Pillar::Choice pillar = Pillar::LastRelevantDate,
                            const Date& customPillarDate = Date());
        /*! The maturity is the first trading session of the given
            month on B3, i.e., the first business day of the month
            according to the Brazil exchange calendar.
        */
        Di1FutureRateHelper(const std::variant<Rate, Handle<Quote>>& rate,
                            Month maturityMonth,
                            Year maturityYear,
                            const ext::shared_ptr<OvernightIndex>& overnightIndex,
                            const std::variant<Rate, Handle<Quote>>& convexityAdjustment = 0.0,
                            Pillar::Choice pillar = Pillar::LastRelevantDate,
                            const Date& customPillarDate = Date());

        //! \name RateHelper interface
        //@{
        Real impliedQuote() const override;
        ImpliedQuoteSensitivities impliedQuoteSensitivitiesByCurve() const override;
        void setTermStructure(YieldTermStructure*) override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        Real convexityAdjustment() const;
        ext::shared_ptr<Di1Future> future() const { return future_; }

      private:
        void initializeDates() override;
        ext::shared_ptr<Di1Future> future_;
        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

}

#endif
