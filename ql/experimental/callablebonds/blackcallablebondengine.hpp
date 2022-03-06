/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Allen Kuo

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

/*! \file blackcallablebondengine.hpp
    \brief Black-formula callable bond engines
*/

#ifndef quantlib_black_callable_bond_engine_hpp
#define quantlib_black_callable_bond_engine_hpp

#include <ql/experimental/callablebonds/callablebond.hpp>
#include <ql/experimental/callablebonds/callablebondvolstructure.hpp>

namespace QuantLib {

    //! Black-formula callable fixed rate bond engine
    /*! Callable fixed rate bond Black engine. The embedded (European)
        option follows the Black "European bond option" treatment in
        Hull, Fourth Edition, Chapter 20.

        \todo set additionalResults (e.g. vega, fairStrike, etc.)

        \warning This class has yet to be tested

        \ingroup callablebondengines
    */
    class BlackCallableFixedRateBondEngine
        : public CallableFixedRateBond::engine {
      public:
        //! volatility is the quoted fwd yield volatility, not price vol
        BlackCallableFixedRateBondEngine(const Handle<Quote>& fwdYieldVol,
                                         Handle<YieldTermStructure> discountCurve);
        //! volatility is the quoted fwd yield volatility, not price vol
        BlackCallableFixedRateBondEngine(Handle<CallableBondVolatilityStructure> yieldVolStructure,
                                         Handle<YieldTermStructure> discountCurve);
        void calculate() const override;

      private:
        Handle<CallableBondVolatilityStructure> volatility_;
        Handle<YieldTermStructure> discountCurve_;
        // present value of all coupons paid during the life of option
        Real spotIncome() const;
        // converts the yield volatility into a forward price volatility
        Volatility forwardPriceVolatility() const;
    };


    //! Black-formula callable zero coupon bond engine
    /*! Callable zero coupon bond, where the embedded (European)
        option price is assumed to obey the Black formula. Follows
        "European bond option" treatment in Hull, Fourth Edition,
        Chapter 20.

        \warning This class has yet to be tested.

        \ingroup callablebondengines
    */
    class BlackCallableZeroCouponBondEngine :
        public BlackCallableFixedRateBondEngine {
      public:
        //! volatility is the quoted fwd yield volatility, not price vol
        BlackCallableZeroCouponBondEngine(
                              const Handle<Quote>& fwdYieldVol,
                              const Handle<YieldTermStructure>& discountCurve)
        : BlackCallableFixedRateBondEngine(fwdYieldVol, discountCurve) {}

        //! volatility is the quoted fwd yield volatility, not price vol
        BlackCallableZeroCouponBondEngine(
             const Handle<CallableBondVolatilityStructure>& yieldVolStructure,
             const Handle<YieldTermStructure>& discountCurve)
        : BlackCallableFixedRateBondEngine(yieldVolStructure, discountCurve) {}
    };

}


#endif


#ifndef id_1254604ee67da9ef0038d85e29f8be89
#define id_1254604ee67da9ef0038d85e29f8be89
inline bool test_1254604ee67da9ef0038d85e29f8be89(const int* i) {
    return i != nullptr;
}
#endif
