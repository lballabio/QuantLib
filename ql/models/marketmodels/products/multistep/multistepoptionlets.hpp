/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
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


#ifndef quantlib_multistep_optionlets_hpp
#define quantlib_multistep_optionlets_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    class Payoff;

    class MultiStepOptionlets : public MultiProductMultiStep {
      public:
        MultiStepOptionlets(const std::vector<Time>& rateTimes,
                            const std::vector<Real>& accruals,
                            const std::vector<Time>& paymentTimes,
                            const std::vector<ext::shared_ptr<Payoff> >&);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset();
        bool nextTimeStep(
                     const CurveState& currentState,
                     std::vector<Size>& numberCashFlowsThisStep,
                     std::vector<std::vector<CashFlow> >& cashFlowsGenerated);
        #if defined(QL_USE_STD_UNIQUE_PTR)
        std::unique_ptr<MarketModelMultiProduct> clone() const;
        #else
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        #endif
        //@}
      private:
        std::vector<Real> accruals_;
        std::vector<Time> paymentTimes_;
        std::vector<ext::shared_ptr<Payoff> > payoffs_;
        // things that vary in a path
        Size currentIndex_;
    };

    // inline definitions

    inline std::vector<Time>
    MultiStepOptionlets::possibleCashFlowTimes() const {
      return paymentTimes_;
    }

    inline Size MultiStepOptionlets::numberOfProducts() const {
        return payoffs_.size();
    }

    inline Size
    MultiStepOptionlets::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepOptionlets::reset() {
       currentIndex_=0;
    }

}

#endif
