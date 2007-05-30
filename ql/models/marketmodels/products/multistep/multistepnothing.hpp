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


#ifndef quantlib_multistep_nothing_hpp
#define quantlib_multistep_nothing_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>

namespace QuantLib {

    class MultiStepNothing : public MultiProductMultiStep {
      public:
        MultiStepNothing(const EvolutionDescription& evolution,
                         Size numberOfProducts = 1,
                         Size doneIndex = 0);
        //! \name MarketModelMultiProduct interface
        //@{
        std::vector<Time> possibleCashFlowTimes() const;
        Size numberOfProducts() const;
        Size maxNumberOfCashFlowsPerProductPerStep() const;
        void reset();
        bool nextTimeStep(const CurveState&,
                          std::vector<Size>&,
                          std::vector<std::vector<CashFlow> >&);
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        //@}
      private:
        Size numberOfProducts_, doneIndex_;
        // things that vary in a path
        Size currentIndex_;
    };

    // inline definitions

    inline std::vector<Time>
    MultiStepNothing::possibleCashFlowTimes() const {
        return std::vector<Time>();
    }

    inline Size MultiStepNothing::numberOfProducts() const {
        return numberOfProducts_;
    }

    inline Size
    MultiStepNothing::maxNumberOfCashFlowsPerProductPerStep() const {
        return 0;
    }

    inline void MultiStepNothing::reset() {
       currentIndex_=0;
    }

}

#endif
