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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_upper_bound_engine_hpp
#define quantlib_upper_bound_engine_hpp

#include <ql/MarketModels/marketmodelproduct.hpp>
#include <ql/MarketModels/Products/multiproductcomposite.hpp>
#include <ql/MarketModels/marketmodeldiscounter.hpp>
#include <ql/MarketModels/marketmodelevolver.hpp>
#include <ql/Math/sequencestatistics.hpp>
#include <ql/Utilities/clone.hpp>
#include <utility>

namespace QuantLib {

    class UpperBoundEngine {
      public:
        UpperBoundEngine(const boost::shared_ptr<MarketModelEvolver>& evolver,
                         const CallSpecifiedMultiProduct& product,
                         const MarketModelMultiProduct& hedge,
                         double initialNumeraireValue);

        std::pair<Real,Real> singlePathValue();
        void multiplePathValues(Statistics& stats,
                                Size numberOfPaths);
      private:
        boost::shared_ptr<MarketModelEvolver> evolver_;
        MultiProductComposite composite_;

        double initialNumeraireValue_;
        Size underlyingSize_, rebateSize_;
        Size numberOfProducts_;
        Size numberOfSteps_;

        // workspace
        std::vector<Size> numberCashFlowsThisStep_;
        std::vector<std::vector<MarketModelMultiProduct::CashFlow> >
                                                         cashFlowsGenerated_;
        std::vector<MarketModelDiscounter> discounters_;

    };

}

#endif
