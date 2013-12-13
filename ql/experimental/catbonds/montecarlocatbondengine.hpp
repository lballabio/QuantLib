/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012, 2013 Grzegorz Andruszkiewicz

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

/*! \file montecarlocatbondengine.hpp
    \brief Monte Carlo pricing engine for cat bonds
*/

#ifndef quantlib_montecarlo_catbond_engine_hpp
#define quantlib_montecarlo_catbond_engine_hpp

#include <ql/experimental/catbonds/catbond.hpp>

namespace QuantLib {

    class MonteCarloCatBondEngine :
        public CatBond::engine
    {
    public:
        MonteCarloCatBondEngine(
              const boost::shared_ptr<CatRisk> catRisk,
              const Handle<YieldTermStructure>& discountCurve =
                                                Handle<YieldTermStructure>(),
              boost::optional<bool> includeSettlementDateFlows = boost::none);
        void calculate() const;
        Handle<YieldTermStructure> discountCurve() const {
            return discountCurve_;
        }
    protected:
        Real cashFlowRiskyValue(const boost::shared_ptr<CashFlow> cf, 
                                const NotionalPath& notionalPath) const;

        Real npv(bool includeSettlementDateFlows, 
                 Date settlementDate, 
                 Date npvDate, 
                 Real& lossProbability, 
                 Real& exhaustionProbability, 
                 Real& expectedLoss) const;

        Real pathNpv(bool includeSettlementDateFlows, 
                     Date settlementDate, 
                     const NotionalPath& notionalPath) const;
      private:
        boost::shared_ptr<CatRisk> catRisk_;
        Handle<YieldTermStructure> discountCurve_;
        boost::optional<bool> includeSettlementDateFlows_;
    };

}

#endif

