/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_proxy_greek_engine_hpp
#define quantlib_proxy_greek_engine_hpp

// to be removed using forward declaration
#include <ql/models/marketmodels/multiproduct.hpp>

#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/utilities/clone.hpp>
#include <valarray>

namespace QuantLib {

    class MarketModelEvolver;
    class ConstrainedEvolver;
    class MarketModelDiscounter;

    class ProxyGreekEngine {
      public:
        ProxyGreekEngine(
            ext::shared_ptr<MarketModelEvolver> evolver,
            std::vector<std::vector<ext::shared_ptr<ConstrainedEvolver> > > constrainedEvolvers,
            std::vector<std::vector<std::vector<Real> > > diffWeights,
            std::vector<Size> startIndexOfConstraint,
            std::vector<Size> endIndexOfConstraint,
            const Clone<MarketModelMultiProduct>& product,
            Real initialNumeraireValue);
        void multiplePathValues(
                  SequenceStatisticsInc& stats,
                  std::vector<std::vector<SequenceStatisticsInc> >& modifiedStats,
                  Size numberOfPaths);
        void singlePathValues(
                std::vector<Real>& values,
                std::vector<std::vector<std::vector<Real> > >& modifiedValues);
      private:
        void singleEvolverValues(MarketModelEvolver& evolver,
                                 std::vector<Real>& values,
                                 bool storeRates = false);
        ext::shared_ptr<MarketModelEvolver> originalEvolver_;
        std::vector<std::vector<ext::shared_ptr<ConstrainedEvolver> > >
            constrainedEvolvers_;
        std::vector<std::vector<std::vector<Real> > > diffWeights_;
        std::vector<Size> startIndexOfConstraint_;
        std::vector<Size> endIndexOfConstraint_;
        Clone<MarketModelMultiProduct> product_;

        Real initialNumeraireValue_;
        Size numberProducts_;

        // workspace
        std::vector<Rate> constraints_;
        std::valarray<bool> constraintsActive_;
        std::vector<Real> numerairesHeld_;
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelDiscounter> discounters_;

    };

}

#endif
