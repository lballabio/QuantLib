/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Giorgio Facchinetti
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

/*! \file discountingbondengine.hpp
    \brief discounting bond engine
*/

#ifndef quantlib_discounting_bond_engine_hpp
#define quantlib_discounting_bond_engine_hpp

#include <ql/instruments/bond.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/handle.hpp>
#include <optional>

namespace QuantLib {

    class DiscountingBondEngine : public Bond::engine {
      public:
        DiscountingBondEngine(
            Handle<YieldTermStructure> discountCurve = Handle<YieldTermStructure>(),
            const std::optional<bool>& includeSettlementDateFlows = std::nullopt);
        void calculate() const override;
        Handle<YieldTermStructure> discountCurve() const {
            return discountCurve_;
        }
      private:
        Handle<YieldTermStructure> discountCurve_;
        std::optional<bool> includeSettlementDateFlows_;
    };

}

#endif
