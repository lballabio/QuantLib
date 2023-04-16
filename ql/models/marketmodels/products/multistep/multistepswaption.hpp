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


#ifndef quantlib_multistep_multistep_swaption_hpp
#define quantlib_multistep_multistep_swaption_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>
#include <memory>

namespace QuantLib {

    class StrikedTypePayoff;

    /*!
    Price a swaption associated to a contiguous subset of rates. Useful only for
    testing purposes. Steps through all rate times up to start of swap. 
    */

    class MultiStepSwaption : public MultiProductMultiStep {
      public:
        MultiStepSwaption(const std::vector<Time>& rateTimes,
                                     Size startIndex,
                                     Size endIndex,
                                     std::shared_ptr<StrikedTypePayoff> &);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Time> possibleCashFlowTimes() const override;
        Size numberOfProducts() const override;
        Size maxNumberOfCashFlowsPerProductPerStep() const override;
        void reset() override;
        bool nextTimeStep(const CurveState& currentState,
                          std::vector<Size>& numberCashFlowsThisStep,
                          std::vector<std::vector<CashFlow> >& cashFlowsGenerated) override;
        std::unique_ptr<MarketModelMultiProduct> clone() const override;
         //@}

      private:
    
        Size startIndex_;
        Size endIndex_;
        std::shared_ptr<StrikedTypePayoff> payoff_;
        std::vector<Time> paymentTimes_;
        // things that vary in a path
        Size currentIndex_;
    };

    // Inline definitions

    inline std::vector<Time>
    MultiStepSwaption::possibleCashFlowTimes() const 
    {
        return paymentTimes_;
    }

    inline Size MultiStepSwaption::numberOfProducts() const {
        return 1UL;
    }

    inline Size
    MultiStepSwaption::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepSwaption::reset()
    {
       currentIndex_=0;
    }

}

#endif
