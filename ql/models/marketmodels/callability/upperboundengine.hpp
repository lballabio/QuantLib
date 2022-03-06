/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Mark Joshi
 Copyright (C) 2006 StatPro Italia srl

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


#ifndef quantlib_upper_bound_engine_hpp
#define quantlib_upper_bound_engine_hpp

#include <ql/models/marketmodels/products/multiproductcomposite.hpp>
#include <ql/methods/montecarlo/exercisestrategy.hpp>
#include <ql/math/statistics/sequencestatistics.hpp>
#include <ql/utilities/clone.hpp>
#include <utility>
#include <valarray>

namespace QuantLib {

    class MarketModelEvolver;
    class MarketModelDiscounter;
    class MarketModelMultiProduct;
    class MarketModelExerciseValue;

    //! Market-model %engine for upper-bound estimation
    /*! \pre product and hedge must have the same rate times
             and exercise times
    */
    class UpperBoundEngine {
      public:
        UpperBoundEngine(ext::shared_ptr<MarketModelEvolver> evolver,
                         std::vector<ext::shared_ptr<MarketModelEvolver> > innerEvolvers,
                         const MarketModelMultiProduct& underlying,
                         const MarketModelExerciseValue& rebate,
                         const MarketModelMultiProduct& hedge,
                         const MarketModelExerciseValue& hedgeRebate,
                         const ExerciseStrategy<CurveState>& hedgeStrategy,
                         Real initialNumeraireValue);
        void multiplePathValues(Statistics& stats,
                                Size outerPaths,
                                Size innerPaths);
        std::pair<Real,Real> singlePathValue(Size innerPaths);
      private:
        Real collectCashFlows(Size currentStep,
                              Real principalInNumerairePortfolio,
                              Size beginProduct,
                              Size endProduct) const;

        ext::shared_ptr<MarketModelEvolver> evolver_;
        std::vector<ext::shared_ptr<MarketModelEvolver> > innerEvolvers_;
        MultiProductComposite composite_;

        Real initialNumeraireValue_;
        Size underlyingSize_, rebateSize_, hedgeSize_, hedgeRebateSize_;
        Size underlyingOffset_, rebateOffset_, hedgeOffset_, hedgeRebateOffset_;
        Size numberOfProducts_;
        Size numberOfSteps_;
        std::valarray<bool> isExerciseTime_;

        // workspace
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelDiscounter> discounters_;
    };

}

#endif


#ifndef id_27ab06d0bb36d547f974ccd97bccbfcd
#define id_27ab06d0bb36d547f974ccd97bccbfcd
inline bool test_27ab06d0bb36d547f974ccd97bccbfcd(const int* i) {
    return i != nullptr;
}
#endif
