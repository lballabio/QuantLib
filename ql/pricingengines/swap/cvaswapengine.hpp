/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Jose Aparicio

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

#ifndef quantlib_cva_swap_engine_hpp
#define quantlib_cva_swap_engine_hpp

#include <ql/handle.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>

namespace QuantLib {

    class YieldTermStructure;
    class Quote;

  /*! Bilateral (CVA and DVA) default adjusted vanilla swap pricing
    engine. Collateral is not considered. No wrong way risk is 
    considered (rates and counterparty default are uncorrelated).
    Based on:
    Sorensen,  E.H.  and  Bollier,  T.F.,  Pricing  swap  default 
    risk. Financial Analysts Journal, 1994, 50, 23–33
    Also see sect. II-5 in: Risk Neutral Pricing of Counterparty Risk
    D. Brigo, M. Masetti, 2004
    or in sections 3 and 4 of "A Formula for Interest Rate Swaps 
      Valuation under Counterparty Risk in presence of Netting Agreements"
    D. Brigo and M. Masetti; May 4, 2005

    to do: Compute fair rate through iteration instead of the 
    current approximation .
    to do: write Issuer based constructors (event type)
    to do: Check consistency between option engine discount and the one given
   */
  class CounterpartyAdjSwapEngine : public VanillaSwap::engine {
    public:
      //! \name Constructors
      //@{
      //! 
      /*! Creates the engine from an arbitrary swaption engine.
        If the investor default model is not given a default 
        free one is assumed.
        @param discountCurve Used in pricing.
        @param swaptionEngine Determines the volatility and thus the 
        exposure model.
        @param ctptyDTS Counterparty default curve.
        @param ctptyRecoveryRate Counterparty recovey rate.
        @param invstDTS Investor (swap holder) default curve.
        @param invstRecoveryRate Investor recovery rate.
       */
      CounterpartyAdjSwapEngine(
          const Handle<YieldTermStructure>& discountCurve,
          const Handle<PricingEngine>& swaptionEngine,
          const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
          Real ctptyRecoveryRate,
          const Handle<DefaultProbabilityTermStructure>& invstDTS =
              Handle<DefaultProbabilityTermStructure>(),
          Real invstRecoveryRate = 0.999);
      /*! Creates an engine with a black volatility model for the 
        exposure.
        If the investor default model is not given a default 
        free one is assumed.
        @param discountCurve Used in pricing.
        @param blackVol Black volatility used in the exposure model.
        @param ctptyDTS Counterparty default curve.
        @param ctptyRecoveryRate Counterparty recovey rate.
        @param invstDTS Investor (swap holder) default curve.
        @param invstRecoveryRate Investor recovery rate.
       */
      CounterpartyAdjSwapEngine(const Handle<YieldTermStructure>& discountCurve,
                                Volatility blackVol,
                                const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
                                Real ctptyRecoveryRate,
                                const Handle<DefaultProbabilityTermStructure>& invstDTS =
                                    Handle<DefaultProbabilityTermStructure>(),
                                Real invstRecoveryRate = 0.999);
      /*! Creates an engine with a black volatility model for the 
        exposure. The volatility is given as a quote.
        If the investor default model is not given a default 
        free one is assumed.
        @param discountCurve Used in pricing.
        @param blackVol Black volatility used in the exposure model.
        @param ctptyDTS Counterparty default curve.
        @param ctptyRecoveryRate Counterparty recovey rate.
        @param invstDTS Investor (swap holder) default curve.
        @param invstRecoveryRate Investor recovery rate.
      */
      CounterpartyAdjSwapEngine(
          const Handle<YieldTermStructure>& discountCurve,
          const Handle<Quote>& blackVol,
          const Handle<DefaultProbabilityTermStructure>& ctptyDTS,
          Real ctptyRecoveryRate,
          const Handle<DefaultProbabilityTermStructure>& invstDTS =
              Handle<DefaultProbabilityTermStructure>(),
          Real invstRecoveryRate = 0.999);
      //@}
      void calculate() const override;

    private:
      Handle<PricingEngine> baseSwapEngine_;
      Handle<PricingEngine> swaptionletEngine_;
      Handle<YieldTermStructure> discountCurve_;
      Handle<DefaultProbabilityTermStructure> defaultTS_;	  
      Real ctptyRecoveryRate_;
      Handle<DefaultProbabilityTermStructure> invstDTS_;	  
      Real invstRecoveryRate_;
  };

}

#endif


#ifndef id_d7a5671b4caf58d2d674defff6d29e77
#define id_d7a5671b4caf58d2d674defff6d29e77
inline bool test_d7a5671b4caf58d2d674defff6d29e77(int* i) { return i != 0; }
#endif
