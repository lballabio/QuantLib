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


#ifndef quantlib_template_hpp
#define quantlib_template_hpp

#include <ql/MarketModels/marketmodelproduct.hpp>
#include <ql/MarketModels/marketmodelevolver.hpp>

namespace QuantLib {

    class AccountingEngine
    {
    public:
        AccountingEngine(MarketModelEvolver& evolver,
                         MarketModelProduct& product,
                         const EvolutionDescription& evolution,
                         double initialNumeraireValue
                );

        void SinglePathValues(Array& values);

    private:
        boost::shared_ptr<MarketModelEvolver> evolver_;
        boost::shared_ptr<MarketModelProduct> product_;
        EvolutionDescription evolution_;
        
        double initialNumeraireValue_;
        Size numberProducts_;
        
        // workspace
        std::vector<Real> numerairesHeld_;
        CurveState curveState_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelProduct::CashFlow> > cashFlowsGenerated_; 
        
    };

}


#endif
