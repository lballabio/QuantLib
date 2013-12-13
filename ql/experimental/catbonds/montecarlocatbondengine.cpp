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

#include <ql/experimental/catbonds/montecarlocatbondengine.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <algorithm>

namespace QuantLib {

    MonteCarloCatBondEngine::MonteCarloCatBondEngine(
                             const boost::shared_ptr<CatRisk> catRisk,
                             const Handle<YieldTermStructure>& discountCurve,
                             boost::optional<bool> includeSettlementDateFlows)
    : catRisk_(catRisk), discountCurve_(discountCurve),
      includeSettlementDateFlows_(includeSettlementDateFlows) {
        registerWith(discountCurve_);
    }

    void MonteCarloCatBondEngine::calculate() const {
        QL_REQUIRE(!discountCurve_.empty(),
                   "discounting term structure handle is empty");

        results_.valuationDate = (*discountCurve_)->referenceDate();

        bool includeRefDateFlows =
            includeSettlementDateFlows_ ?
            *includeSettlementDateFlows_ :
            Settings::instance().includeReferenceDateEvents();

        Real lossProbability;
        Real exhaustionProbability;
        Real expectedLoss;

        results_.value = npv(includeRefDateFlows,
                             results_.valuationDate,
                             results_.valuationDate,
                             lossProbability,
                             exhaustionProbability,
                             expectedLoss);

        results_.lossProbability = lossProbability;
        results_.exhaustionProbability = exhaustionProbability;
        results_.expectedLoss = expectedLoss;

        // a bond's cashflow on settlement date is never taken into
        // account, so we might have to play it safe and recalculate
        if (!includeRefDateFlows
                     && results_.valuationDate == arguments_.settlementDate) {
            // same parameters as above, we can avoid another call
            results_.settlementValue = results_.value;
        } else {
            // no such luck
            results_.settlementValue =
                npv(includeRefDateFlows, arguments_.settlementDate, arguments_.settlementDate, lossProbability, exhaustionProbability, expectedLoss);
        }
    }

    Real MonteCarloCatBondEngine::npv(bool includeSettlementDateFlows, Date settlementDate, Date npvDate, Real& lossProbability, Real &exhaustionProbability, Real& expectedLoss) const
    {
	    const size_t MAX_PATHS = 10000; //TODO
        lossProbability =  0.0;
        exhaustionProbability = 0.0;
        expectedLoss = 0.0;
        if (arguments_.cashflows.empty())
            return 0.0;

        if (settlementDate == Date())
            settlementDate = Settings::instance().evaluationDate();

        if (npvDate == Date())
            npvDate = settlementDate;

        Real totalNPV = 0.0;
        Date effectiveDate = std::max(arguments_.startDate, settlementDate);
        Date maturityDate = (*arguments_.cashflows.rbegin())->date();
        boost::shared_ptr<CatSimulation> catSimulation = catRisk_->newSimulation(effectiveDate, maturityDate);
        std::vector<std::pair<Date, Real> > eventsPath;
        NotionalPath notionalPath;
        Real riskFreeNPV = pathNpv(includeSettlementDateFlows, settlementDate, notionalPath);
        size_t pathCount=0;
        while(catSimulation->nextPath(eventsPath) && pathCount<MAX_PATHS)
        {
            arguments_.notionalRisk->updatePath(eventsPath, notionalPath);
            if(notionalPath.loss()>0) { //optimization, most paths will not include any loss
                totalNPV += pathNpv(includeSettlementDateFlows, settlementDate, notionalPath);
                lossProbability+=1;
                if (notionalPath.loss()==1) 
                    exhaustionProbability+=1;
                expectedLoss+=notionalPath.loss();
            } else {
                totalNPV += riskFreeNPV;
            }
            pathCount++;
        }
        lossProbability/=pathCount;
        exhaustionProbability/=pathCount;
        expectedLoss/=pathCount;
        return totalNPV/(pathCount*discountCurve_->discount(npvDate));
    }

    Real MonteCarloCatBondEngine::pathNpv(bool includeSettlementDateFlows, 
                                          Date settlementDate, 
                                          const NotionalPath& notionalPath) const {
        Real totalNPV = 0.0;
        for (Size i=0; i<arguments_.cashflows.size(); ++i) {
            if (!arguments_.cashflows[i]->hasOccurred(settlementDate, 
                                        includeSettlementDateFlows)) {
                Real amount = cashFlowRiskyValue(arguments_.cashflows[i], notionalPath);
                totalNPV += amount * discountCurve_->discount(arguments_.cashflows[i]->date());
            }
        }
        return totalNPV;
    }

    Real MonteCarloCatBondEngine::cashFlowRiskyValue(const boost::shared_ptr<CashFlow> cf, 
                                                     const NotionalPath& notionalPath) const {
        return cf->amount()*notionalPath.notionalRate(cf->date()); //TODO: fix for more complicated cashflows
    }

}
