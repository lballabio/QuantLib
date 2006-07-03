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
    AccountingEngine::AccountingEngine(MarketModelEvolver& evolver,
        MarketModelProduct& product,
        const EvolutionDescription& evolution,
        double initialNumeraireValue)
        :
    evolver_(&evolver),
        product_(&product),
       evolution_(evolution),
        initialNumeraireValue_(initialNumeraireValue),
        numberProducts_(product.numberOfProducts()),      
        numerairesHeld_(product.numberOfProducts()),
        curveState_(evolution.rateTimes()),
        numberCashFlowsThisStep_(product.numberOfProducts()),
        cashFlowsGenerated_(product.numberOfProducts()) 
    {
        for (Size i = 0; i <numberProducts_; ++i )
            cashFlowsGenerated_[i].resize(product_->maxNumberOfCashFlowsPerProductPerStep());


    }

 void AccountingEngine::SinglePathValues(Array& values)
 {
     std::fill(numerairesHeld_.begin(),numerairesHeld_.end(),0.);
     Real weight = evolver_->startNewPath();
     product_->reset();     
     Real principalInNumerairePortfolio =1.0;
    
     bool done = false;
     do
     {
        weight*= evolver_->advanceStep();
        done = product_->nextTimeStep(
            evolver_->currentState(), 
            numberCashFlowsThisStep_, 
            cashFlowsGenerated_);
// do the hard bit!
        //loop throguht products
        // each product, for each cash flow
        // convert it to numeraires
        // add on to number numeraires held\
        // update the number of bonds in the numeraire portfolio
     }
     while (!done);

     for (Size i=0; i < numerairesHeld_.size(); ++i)
         values[i]=numerairesHeld_[i] * initialNumeraireValue_;

//         std::transform(numerairesHeld_.begin(),numerairesHeld_.end(),numerairesHeld_.begin(),
  //       boost::bind(std::multiplies<Real>    
         
 }

}