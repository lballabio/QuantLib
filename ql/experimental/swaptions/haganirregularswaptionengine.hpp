/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011, 2012 Andre Miemiec
 Copyright (C) 2012 Samuel Tebege

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

/*! \file haganirregularswaptionengine.hpp
    \brief engine for pricing irregular swaptions via super-replication
*/

#ifndef quantlib_hagan_irregular_swaption_engine_hpp
#define quantlib_hagan_irregular_swaption_engine_hpp

#include <ql/experimental/swaptions/irregularswaption.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolstructure.hpp>
#include <ql/math/optimization/costfunction.hpp>
#include <ql/instruments/makevanillaswap.hpp>

namespace QuantLib {

    //! Pricing engine for irregular swaptions 
    /*! References:

        1. P.S. Hagan: "Methodology for Callable Swaps and Bermudan
           'Exercise into Swaptions'"
        2. P.J. Hunt, J.E. Kennedy: "Implied interest rate pricing
           models", Finance Stochast. 2, 275-293 (1998)
    
        \warning Currently a spread is not handled correctly; it
                 should be a minor exercise to account for this
                 feature as well;
    */
    class HaganIrregularSwaptionEngine
        : public GenericEngine<IrregularSwaption::arguments,
        IrregularSwaption::results> {
    public:
        //@{
      explicit HaganIrregularSwaptionEngine(
          Handle<SwaptionVolatilityStructure>,
          Handle<YieldTermStructure> termStructure = Handle<YieldTermStructure>());
      //@}
      void calculate() const override;

      // helper class
      class Basket {
        public:
          Basket(ext::shared_ptr<IrregularSwap> swap,
                 Handle<YieldTermStructure> termStructure,
                 Handle<SwaptionVolatilityStructure> volatilityStructure);
          Array compute(Rate lambda = 0.0) const;
          Real operator()(Rate x) const;
          ext::shared_ptr<VanillaSwap> component(Size i) const;
          Array weights() const { return compute(lambda_); };
          Real& lambda() const { return lambda_; };
          // NOLINTNEXTLINE(cppcoreguidelines-noexcept-swap,performance-noexcept-swap)
          ext::shared_ptr<IrregularSwap> swap() const { return swap_; };
        private:
            ext::shared_ptr<IrregularSwap> swap_;
            Handle<YieldTermStructure>          termStructure_;
            Handle<SwaptionVolatilityStructure> volatilityStructure_;

            Real targetNPV_ = 0.0;

            ext::shared_ptr<PricingEngine> engine_;

            std::vector<Real> fairRates_;
            std::vector<Real> annuities_;
            std::vector<Date> expiries_;

            mutable Real lambda_ = 0.0;
        };

        Real HKPrice(Basket& basket,ext::shared_ptr<Exercise>& exercise)  const;
        Real LGMPrice(Basket& basket,ext::shared_ptr<Exercise>& exercise) const;

    private:
        Handle<YieldTermStructure>          termStructure_;
        Handle<SwaptionVolatilityStructure> volatilityStructure_;
        class rStarFinder;
    };

}

#endif
