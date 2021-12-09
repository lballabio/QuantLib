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


#ifndef quantlib_multistep_forwards_hpp
#define quantlib_multistep_forwards_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>

namespace QuantLib {

    class MultiStepForwards : public MultiProductMultiStep {
      public:
        MultiStepForwards(const std::vector<Time>& rateTimes,
                          std::vector<Real> accruals,
                          const std::vector<Time>& paymentTimes,
                          std::vector<Rate> strikes);
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
        std::vector<Real> accruals_;
        std::vector<Time> paymentTimes_;
        std::vector<Rate> strikes_;
        // things that vary in a path
        Size currentIndex_;
    };

    // inline

    inline std::vector<Time>
    MultiStepForwards::possibleCashFlowTimes() const {
        return paymentTimes_;
    }

    inline Size MultiStepForwards::numberOfProducts() const {
        return strikes_.size();
    }

    inline Size
    MultiStepForwards::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepForwards::reset() {
       currentIndex_=0;
    }

}

#endif
