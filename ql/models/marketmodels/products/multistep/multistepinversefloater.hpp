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

#ifndef quantlib_multistep_inverse_floater_hpp
#define quantlib_multistep_inverse_floater_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>

namespace QuantLib {

    /*
    Tested in MarketModels::testInverseFloater()

    */

    class MultiStepInverseFloater : public MultiProductMultiStep {
      public:
        MultiStepInverseFloater(const std::vector<Time>& rateTimes,
                      const std::vector<Real>& fixedAccruals,
                      const std::vector<Real>& floatingAccruals,
                      const std::vector<Real>& fixedStrikes,
                      const std::vector<Real>& fixedMultipliers, 
                      const std::vector<Real>& floatingSpreads,
                      const std::vector<Time>& paymentTimes,
                      bool payer = true);
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
        std::auto_ptr<MarketModelMultiProduct> clone() const;
        //@}
      private:
        std::vector<Real> fixedAccruals_, floatingAccruals_,fixedStrikes_, fixedMultipliers_, floatingSpreads_;
        std::vector<Time> paymentTimes_;
    
        bool payer_;
        Real multiplier_;
        Size lastIndex_;
        // things that vary in a path
        Size currentIndex_;
    };


    // inline definitions

    inline std::vector<Time>
    MultiStepInverseFloater::possibleCashFlowTimes() const {
        return paymentTimes_;
    }

    inline Size MultiStepInverseFloater::numberOfProducts() const {
        return 1;
    }

    inline Size
    MultiStepInverseFloater::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepInverseFloater::reset() {
       currentIndex_=0;
    }

}

#endif
