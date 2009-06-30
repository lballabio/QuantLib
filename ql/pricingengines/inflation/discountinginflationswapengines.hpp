/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Chris Kenyon
 Copyright (C) 2009 StatPro Italia srl

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

/*! \file discountinginflationswapengines.hpp
    \brief discounting inflation-swap engine
*/

#ifndef quantlib_discounting_inflation_swap_engines_hpp
#define quantlib_discounting_inflation_swap_engines_hpp

#include <ql/instruments/yearonyearinflationswap.hpp>
#include <ql/instruments/zerocouponinflationswap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    //! deterministic year-on-year inflation-swap engine
    class DiscountingYoYInflationSwapEngine
        : public YearOnYearInflationSwap::engine {
      public:
        DiscountingYoYInflationSwapEngine(
              const Handle<YieldTermStructure>& discountCurve,
              const Handle<YoYInflationTermStructure>& inflationCurve,
              boost::optional<bool> includeSettlementDateFlows = boost::none);
        void calculate() const;
      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<YoYInflationTermStructure> inflationCurve_;
        boost::optional<bool> includeSettlementDateFlows_;
    };


    //! deterministic zero-coupon inflation-swap engine
    class DiscountingZeroInflationSwapEngine
        : public ZeroCouponInflationSwap::engine {
      public:
        DiscountingZeroInflationSwapEngine(
              const Handle<YieldTermStructure>& discountCurve,
              const Handle<ZeroInflationTermStructure>& inflationCurve,
              boost::optional<bool> includeSettlementDateFlows = boost::none);
        void calculate() const;
      private:
        Handle<YieldTermStructure> discountCurve_;
        Handle<ZeroInflationTermStructure> inflationCurve_;
        boost::optional<bool> includeSettlementDateFlows_;
    };

}


#endif
