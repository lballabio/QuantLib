/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2009 StatPro Italia srl
 Copyright (C) 2011 Ferdinando Ametrano

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

/*! \file discountingswapengine.hpp
    \brief discounting swap engine
*/

#ifndef quantlib_discounting_swap_engine_hpp
#define quantlib_discounting_swap_engine_hpp

#include <ql/instruments/swap.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class DiscountingSwapEngine : public Swap::engine {
      public:
        DiscountingSwapEngine(
            Handle<YieldTermStructure> discountCurve = Handle<YieldTermStructure>(),
            const boost::optional<bool>& includeSettlementDateFlows = boost::none,
            Date settlementDate = Date(),
            Date npvDate = Date());
        void calculate() const override;
        Handle<YieldTermStructure> discountCurve() const {
            return discountCurve_;
        }
      private:
        Handle<YieldTermStructure> discountCurve_;
        boost::optional<bool> includeSettlementDateFlows_;
        Date settlementDate_, npvDate_;
    };

}

#endif


#ifndef id_620b252c510d5be019a0d4342f694a74
#define id_620b252c510d5be019a0d4342f694a74
inline bool test_620b252c510d5be019a0d4342f694a74(int* i) { return i != 0; }
#endif
