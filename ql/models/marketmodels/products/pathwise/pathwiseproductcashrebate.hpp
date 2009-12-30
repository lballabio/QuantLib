/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Mark Joshi

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


#ifndef quantlib_market_model_pathwise_swaption_hpp
#define quantlib_market_model_pathwise_swaption_hpp

#include <ql/types.hpp>
#include <ql/models/marketmodels/pathwisemultiproduct.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/math/matrix.hpp>
#include <vector>
#include <memory>

namespace QuantLib {

    class EvolutionDescription;
    class CurveState;

    /*!
    Swap for doing simple cash rebate. Fairly useless when used directly, but if we want to look a breakable swap
    it becomes useful.

    */
    class MarketModelPathwiseCashRebate : public MarketModelPathwiseMultiProduct

     {
      public:
        MarketModelPathwiseCashRebate(const EvolutionDescription& evolution,
                              const std::vector<Time>& paymentTimes,
                              const Matrix& amounts,
                              Size numberOfProducts);

        virtual std::vector<Size> suggestedNumeraires() const;
        virtual const EvolutionDescription& evolution() const;
        virtual std::vector<Time> possibleCashFlowTimes() const;
        virtual Size numberOfProducts() const;
        virtual Size maxNumberOfCashFlowsPerProductPerStep() const;
        virtual void reset();


        virtual bool alreadyDeflated() const;

        virtual bool nextTimeStep(const CurveState& currentState,
                                  std::vector<Size>& numberCashFlowsThisStep,
                                  std::vector<std::vector<MarketModelPathwiseMultiProduct::CashFlow> >& cashFlowsGenerated);

        virtual std::auto_ptr<MarketModelPathwiseMultiProduct> clone() const;

      private:
        EvolutionDescription evolution_;
        std::vector<Time> paymentTimes_;
        Matrix amounts_;
        Size numberOfProducts_;
        // things that vary in a path
        Size currentIndex_;
    };
}

#endif
