/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Stefano Fondi

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

/*! \file arithmeticoisratehelper.hpp
    \brief Arithmetic Average Overnight Indexed Swap rate helpers
*/

#ifndef quantlib_arithmeticoisratehelper_hpp
#define quantlib_arithmeticoisratehelper_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/experimental/averageois/arithmeticaverageois.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over Overnight Indexed Swap rates
    class ArithmeticOISRateHelper : public RelativeDateRateHelper {
      public:
        ArithmeticOISRateHelper(
            Natural settlementDays,
            const Period& tenor, // swap maturity
            Frequency fixedLegPaymentFrequency,
            const Handle<Quote>& fixedRate,
            ext::shared_ptr<OvernightIndex> overnightIndex,
            Frequency overnightLegPaymentFrequency,
            Handle<Quote> spread,
            Real meanReversionSpeed = 0.03,
            Real volatility = 0.00, // NO convexity adjustment by default
            bool byApprox = false,  // TRUE to use Katsumi Takada approximation
            // exogenous discounting curve
            Handle<YieldTermStructure> discountingCurve = Handle<YieldTermStructure>());
        //! \name RateHelper interface
        //@{
          Real impliedQuote() const override;
          void setTermStructure(YieldTermStructure*) override;
          //@}
          //! \name inspectors
          //@{
          // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
          ext::shared_ptr<ArithmeticAverageOIS> swap() const { return swap_; }
          //@}
          //! \name Visitability
          //@{
          void accept(AcyclicVisitor&) override;
          //@}
        protected:
          void initializeDates() override;

          Natural settlementDays_;
          Period tenor_;
          ext::shared_ptr<OvernightIndex> overnightIndex_;

          ext::shared_ptr<ArithmeticAverageOIS> swap_;
          RelinkableHandle<YieldTermStructure> termStructureHandle_;

          Handle<YieldTermStructure> discountHandle_;
          RelinkableHandle<YieldTermStructure> discountRelinkableHandle_;

          Frequency fixedLegPaymentFrequency_;
          Frequency overnightLegPaymentFrequency_;
          Handle<Quote> spread_;

          Real mrs_;
          Real vol_;
          bool byApprox_;
 
    };

}

#endif
