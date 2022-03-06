/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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


#ifndef quantlib_market_model_cash_rebate_hpp
#define quantlib_market_model_cash_rebate_hpp

#include <ql/models/marketmodels/multiproduct.hpp>
#include <ql/models/marketmodels/evolutiondescription.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib 
{
    /*!
    Class to model receipt of a fixed cash amount once. Product terminates immediately. 
    Mainly useful as rebate received when another product is cancelled. 

    */

    class MarketModelCashRebate : public MarketModelMultiProduct 
    {
      public:
        MarketModelCashRebate(EvolutionDescription evolution,
                              const std::vector<Time>& paymentTimes,
                              Matrix amounts,
                              Size numberOfProducts);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Size> suggestedNumeraires() const override;
        const EvolutionDescription& evolution() const override;
        std::vector<Time> possibleCashFlowTimes() const override;
        Size numberOfProducts() const override;
        Size maxNumberOfCashFlowsPerProductPerStep() const override;
        void reset() override;
        bool nextTimeStep(const CurveState& currentState,
                          std::vector<Size>& numberCashFlowsThisStep,
                          std::vector<std::vector<CashFlow> >& cashFlowsGenerated) override;
#if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<MarketModelMultiProduct> clone() const override;
#else
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        #endif
        //@}
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


#ifndef id_189e0c33a7caff310576e2d7dc97d35d
#define id_189e0c33a7caff310576e2d7dc97d35d
inline bool test_189e0c33a7caff310576e2d7dc97d35d(const int* i) {
    return i != nullptr;
}
#endif
