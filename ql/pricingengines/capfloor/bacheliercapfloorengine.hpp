/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Michael von den Driesch

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

/*! \file bacheliercapfloorengine.hpp
    \brief Bachelier-Black-formula cap/floor engine
*/

#ifndef quantlib_pricers_bachelier_capfloor_hpp
#define quantlib_pricers_bachelier_capfloor_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/termstructures/volatility/optionlet/optionletvolatilitystructure.hpp>

namespace QuantLib {

    class Quote;

    //! Bachelier-Black-formula cap/floor engine
    /*! \ingroup capfloorengines */
    class BachelierCapFloorEngine : public CapFloor::engine {
      public:
        BachelierCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                Volatility vol,
                                const DayCounter& dc = Actual365Fixed());
        BachelierCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                const Handle<Quote>& vol,
                                const DayCounter& dc = Actual365Fixed());
        BachelierCapFloorEngine(Handle<YieldTermStructure> discountCurve,
                                Handle<OptionletVolatilityStructure> vol);
        void calculate() const override;
        Handle<YieldTermStructure> termStructure() { return discountCurve_; }
        Handle<OptionletVolatilityStructure> volatility() { return vol_; }
      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<OptionletVolatilityStructure> vol_;
    };

}

#endif


#ifndef id_695a7698c8efed582ea7732f56b6986e
#define id_695a7698c8efed582ea7732f56b6986e
inline bool test_695a7698c8efed582ea7732f56b6986e(const int* i) {
    return i != nullptr;
}
#endif
