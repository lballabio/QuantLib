/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/MarketModels/accountingengine.hpp>
#include <algorithm>

namespace QuantLib {

    AccountingEngine::AccountingEngine(
        const boost::shared_ptr<MarketModelEvolver>& evolver,
        const boost::shared_ptr<MarketModelMultiProduct>& product,
        const EvolutionDescription& evolution,
        double initialNumeraireValue)
        :
        evolver_(evolver),
        product_(product),
        evolution_(evolution),
        initialNumeraireValue_(initialNumeraireValue),
        numberProducts_(product->numberOfProducts()),      
        numerairesHeld_(product->numberOfProducts()),
        numberCashFlowsThisStep_(product->numberOfProducts()),
        cashFlowsGenerated_(product->numberOfProducts())
    {
        for (Size i = 0; i <numberProducts_; ++i )
            cashFlowsGenerated_[i].resize(
                       product_->maxNumberOfCashFlowsPerProductPerStep());
        
        const std::vector<Time>& cashFlowTimes = product_->possibleCashFlowTimes();
        const std::vector<Rate>& rateTimes = evolution_.rateTimes();
        for (Size j = 0; j < cashFlowTimes.size(); ++j)
            discounters_.push_back(Discounter(cashFlowTimes[j], rateTimes));

    }

    Real AccountingEngine::singlePathValues(std::vector<Real>& values)
    {
        std::fill(numerairesHeld_.begin(),numerairesHeld_.end(),0.);
        Real weight = evolver_->startNewPath();
        product_->reset();     
        Real principalInNumerairePortfolio = 1.0;
    
        bool done = false;
        do {
            Size thisStep = evolver_->currentStep();
            weight *= evolver_->advanceStep();
            done = product_->nextTimeStep(evolver_->currentState(), 
                                          numberCashFlowsThisStep_, 
                                          cashFlowsGenerated_);
            Size numeraire =
                evolution_.numeraires()[thisStep];

            // for each product...
            for (Size i=0; i<numberProducts_; ++i) {
                // ...and each cash flow...
                const std::vector<MarketModelMultiProduct::CashFlow>& cashflows =
                    cashFlowsGenerated_[i];
                for (Size j=0; j<numberCashFlowsThisStep_[i]; ++j) {
                    // ...convert the cash flow to numeraires.
                    // This is done by calculating the number of
                    // numeraire bonds corresponding to such cash flow...
                    const Discounter& discounter =
                        discounters_[cashflows[j].timeIndex];

                    Real bonds = 
                        cashflows[j].amount *
                        discounter.numeraireBonds(evolver_->currentState(),
                                                  numeraire);

                    // ...and adding the newly bought bonds to the number
                    // of numeraires held.
                    numerairesHeld_[i] += bonds/principalInNumerairePortfolio;
                }
            }

            if (!done) {

                // The numeraire might change between steps. This implies
                // that we might have to convert the numeraire bonds for
                // this step into a corresponding amount of numeraire
                // bonds for the next step. This can be done by changing
                // the principal of the numeraire and updating the number
                // of bonds in the numeraire portfolio accordingly.

                Size nextNumeraire =
                    evolution_.numeraires()[thisStep+1];

                principalInNumerairePortfolio *=
                    evolver_->currentState().discountRatio(numeraire, nextNumeraire);
            }
            
        }
        while (!done);

        for (Size i=0; i < numerairesHeld_.size(); ++i)
            values[i] = numerairesHeld_[i] * initialNumeraireValue_;

        return weight;
    }

    void AccountingEngine::multiplePathValues(SequenceStatistics& stats,
                                              Size numberOfPaths)
    {
        std::vector<Real> values(product_->numberOfProducts());
        for (Size i=0; i<numberOfPaths; ++i) {
            Real weight = singlePathValues(values);
            stats.add(values,weight);
        }
    }




    AccountingEngine::Discounter::Discounter(Time paymentTime,
                                             const std::vector<Time>& rateTimes)
	{
		before_ = std::lower_bound(rateTimes.begin(), rateTimes.end(),paymentTime)-
			rateTimes.begin();

        // handle the case where the payment is in the last
        // period or after the last period
		if (before_ > rateTimes.size()-2)
			before_ =  rateTimes.size()-2;

		beforeWeight_=1.0-(paymentTime-rateTimes[before_])/
            (rateTimes[before_+1]-rateTimes[before_]);
    }

    Real AccountingEngine::Discounter::numeraireBonds(
                                          const CurveState& curveState,
                                          Size numeraire) const {
        double preDF = curveState.discountRatio(before_,numeraire);
		if (beforeWeight_==1.0)
			return preDF;
		
		double postDF = curveState.discountRatio(before_+1,numeraire);
		if (beforeWeight_==0.0)
			return postDF;
	
		return std::pow(preDF,beforeWeight_)*std::pow(postDF,1.-beforeWeight_);
    }

}
