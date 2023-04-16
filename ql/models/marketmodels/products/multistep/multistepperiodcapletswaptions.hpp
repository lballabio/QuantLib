/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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


#ifndef quantlib_multistep_period_caplets_swaptions_hpp
#define quantlib_multistep_period_caplets_swaptions_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>
#include <memory>
#include <vector>
namespace QuantLib {

    class StrikedTypePayoff;

    class MultiStepPeriodCapletSwaptions : public MultiProductMultiStep 
    {
      public:
        MultiStepPeriodCapletSwaptions(
            const std::vector<Time>& rateTimes,
            const std::vector<Time>& forwardOptionPaymentTimes,
            const std::vector<Time>& swaptionPaymentTimes,
            std::vector<std::shared_ptr<StrikedTypePayoff> > forwardPayOffs,
            std::vector<std::shared_ptr<StrikedTypePayoff> > swapPayOffs,
            Size period,
            Size offset);
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
        std::vector<Time> paymentTimes_;
        std::vector<Time> forwardOptionPaymentTimes_;
        std::vector<Time> swaptionPaymentTimes_;
        std::vector<std::shared_ptr<StrikedTypePayoff> > forwardPayOffs_;
        std::vector<std::shared_ptr<StrikedTypePayoff> > swapPayOffs_;
        Size lastIndex_;
        Size period_;
        Size offset_;
        Size numberFRAs_;
        Size  numberBigFRAs_; 

        // things that vary in a path
        Size currentIndex_;
        Size productIndex_;
    };

    // Inline definitions

    inline std::vector<Time>
    MultiStepPeriodCapletSwaptions::possibleCashFlowTimes() const {
        return paymentTimes_;
    }

    inline Size MultiStepPeriodCapletSwaptions::numberOfProducts() const {
        return numberBigFRAs_*2;
    }

    inline Size
    MultiStepPeriodCapletSwaptions::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepPeriodCapletSwaptions::reset() {
       currentIndex_=0;
       productIndex_=0;
    }

}

#endif
