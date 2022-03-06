/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Giorgio Facchinetti

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


#ifndef quantlib_multistep_coterminal_swaptions_hpp
#define quantlib_multistep_coterminal_swaptions_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    class StrikedTypePayoff;

    class MultiStepCoterminalSwaptions : public MultiProductMultiStep {
      public:
        MultiStepCoterminalSwaptions(const std::vector<Time>& rateTimes,
                                     const std::vector<Time>& paymentTimes,
                                     std::vector<ext::shared_ptr<StrikedTypePayoff> >);
        //! \name MarketModelMultiProduct interface
        //@{
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
        std::vector<Time> paymentTimes_;
        std::vector<ext::shared_ptr<StrikedTypePayoff> > payoffs_;
        Size lastIndex_;
        // things that vary in a path
        Size currentIndex_;
    };

    // Inline definitions

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


#ifndef id_2a4bb7d52f8e760a9206c89b0f6d90c7
#define id_2a4bb7d52f8e760a9206c89b0f6d90c7
inline bool test_2a4bb7d52f8e760a9206c89b0f6d90c7(int* i) { return i != 0; }
#endif
