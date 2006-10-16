/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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


#ifndef quantlib_market_model_coterminal_swaptions_hpp
#define quantlib_market_model_coterminal_swaptions_hpp

#include <ql/MarketModels/Products/multiproductmultistep.hpp>

namespace QuantLib {

    class MultiStepCoterminalSwaptions : public MultiProductMultiStep {
      public:
        MultiStepCoterminalSwaptions(
                const std::vector<Time>& rateTimes,
                const std::vector<Real>& accruals,
                const std::vector<Time>& paymentTimes,
                const std::vector<Rate>& strikes);

        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset();
        bool nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated);
        std::auto_ptr<MarketModelMultiProduct> clone() const;
 
      private:
        std::vector<Real> accruals_;
        std::vector<Time> paymentTimes_;
        std::vector<Rate> strikes_;
        Size lastIndex_;
        // things that vary in a path
        Size currentIndex_;
    };

    // inline definitions

    inline std::vector<Time>
    MultiStepCoterminalSwaptions::possibleCashFlowTimes() const {
        return paymentTimes_;
    }

    inline Size MultiStepCoterminalSwaptions::numberOfProducts() const {
        return lastIndex_;
    }

    inline Size
    MultiStepCoterminalSwaptions::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepCoterminalSwaptions::reset() {
       currentIndex_=0;
    }

}

#endif
