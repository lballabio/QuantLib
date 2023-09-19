/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano
 Copyright (C) 2006 Mark Joshi

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


#ifndef quantlib_accounting_engine_hpp
#define quantlib_accounting_engine_hpp

// to be removed using forward declaration
#include <ql/models/marketmodels/multiproduct.hpp>
#include <ql/models/marketmodels/discounter.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>

#include <ql/shared_ptr.hpp>
#include <ql/utilities/clone.hpp>
#include <ql/types.hpp>
#include <vector>

namespace QuantLib {

    class MarketModelEvolver;

    //class MarketModelDiscounter;
    //class SequenceStatistics;
    //class MarketModelMultiProduct;
    //struct MarketModelMultiProduct::CashFlow;

    //! Engine collecting cash flows along a market-model simulation
    class AccountingEngine {
      public:
        AccountingEngine(ext::shared_ptr<MarketModelEvolver> evolver,
                         const Clone<MarketModelMultiProduct>& product,
                         Real initialNumeraireValue);
        void multiplePathValues(SequenceStatisticsInc& stats,
                                Size numberOfPaths);
      private:
        Real singlePathValues(std::vector<Real>& values);

        ext::shared_ptr<MarketModelEvolver> evolver_;
        Clone<MarketModelMultiProduct> product_;

        Real initialNumeraireValue_;
        Size numberProducts_;

        // workspace
        std::vector<Real> numerairesHeld_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelDiscounter> discounters_;

    };

}

#endif
