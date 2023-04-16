/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008, 2009 StatPro Italia srl

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

/*! \file integralcdsengine.hpp
    \brief Integral engine for credit default swaps
*/

#ifndef quantlib_integral_cds_engine_hpp
#define quantlib_integral_cds_engine_hpp

#include <ql/instruments/creditdefaultswap.hpp>
#include <optional>

namespace QuantLib {

    class IntegralCdsEngine : public CreditDefaultSwap::engine {
      public:
        IntegralCdsEngine(const Period& integrationStep,
                          Handle<DefaultProbabilityTermStructure>,
                          Real recoveryRate,
                          Handle<YieldTermStructure> discountCurve,
                          const std::optional<bool>& includeSettlementDateFlows = std::nullopt);
        void calculate() const override;

      private:
        Period integrationStep_;
        Handle<DefaultProbabilityTermStructure> probability_;
        Real recoveryRate_;
        Handle<YieldTermStructure> discountCurve_;
        std::optional<bool> includeSettlementDateFlows_;
    };

}


#endif
