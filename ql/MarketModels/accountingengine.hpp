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


#ifndef quantlib_accounting_engine_hpp
#define quantlib_accounting_engine_hpp

#ifndef QL_EXTRA_SAFETY_CHECKS
#   define QL_EXTRA_SAFETY_CHECKS
#endif

#include <ql/MarketModels/marketmodelproduct.hpp>
#include <ql/MarketModels/marketmodelevolver.hpp>
#include <ql/Math/sequencestatistics.hpp>

namespace QuantLib {

    class AccountingEngine
    {
    public:
        class Discounter;

        AccountingEngine(const boost::shared_ptr<MarketModelEvolver>& evolver,
                         const boost::shared_ptr<MarketModelProduct>& product,
                         const EvolutionDescription& evolution,
                         double initialNumeraireValue
                );

        Real singlePathValues(Array& values);
        void multiplePathValues(SequenceStatistics<>& stats,
                                Size numberOfPaths);

    private:
        boost::shared_ptr<MarketModelEvolver> evolver_;
        boost::shared_ptr<MarketModelProduct> product_;
        EvolutionDescription evolution_;
        
        double initialNumeraireValue_;
        Size numberProducts_;
        
        // workspace
        std::vector<Real> numerairesHeld_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<Discounter> discounters_;
        
    };

    class AccountingEngine::Discounter {
    public:
        Discounter(Time paymentTime,
                   const Array& rateTimes);
        Real numeraireBonds(const CurveState&,
                            Size numeraire) const;
    private:
        Size before_;
        Real beforeWeight_;
    };

}


#endif
