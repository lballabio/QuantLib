/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters
 Copyright (C) 2008, 2009 StatPro Italia srl
 Copyright (C) 2009, 2014 Jose Aparicio
 Copyright (C) 2014 Peter Caspers

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

/*! \file isdacdsengine.hpp
    \brief ISDA engine for credit default swaps
*/

#ifndef quantlib_isda_cds_engine_hpp
#define quantlib_isda_cds_engine_hpp

#include <ql/instruments/creditdefaultswap.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/termstructures/credit/defaultprobabilityhelpers.hpp>

namespace QuantLib {

    class IsdaCdsEngine : public CreditDefaultSwap::engine {
      public:
        IsdaCdsEngine(
            const Handle<DefaultProbabilityTermStructure> &probability,
            Real recoveryRate, const Handle<YieldTermStructure> &discountCurve,
            boost::optional<bool> includeSettlementDateFlows = boost::none);
        void calculate() const;

        static Handle<YieldTermStructure> IsdaYieldCurve(std::vector<
            boost::shared_ptr<RelativeDateRateHelper> > &rateHelper);
        static Handle<DefaultProbabilityTermStructure> IsdaProbabilityCurve(
            std::vector<boost::shared_ptr<CdsHelper> > &probabilityHelper);

      private:
        Handle<DefaultProbabilityTermStructure> probability_;
        Real recoveryRate_;
        Handle<YieldTermStructure> discountCurve_;
        boost::optional<bool> includeSettlementDateFlows_;
    };
}


#endif
