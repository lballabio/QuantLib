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

#ifndef quantlib_multistep_ratchet_hpp
#define quantlib_multistep_ratchet_hpp

#include <ql/models/marketmodels/products/multiproductmultistep.hpp>

namespace QuantLib {

    class MultiStepRatchet : public MultiProductMultiStep {
      public:
        MultiStepRatchet(const std::vector<Time>& rateTimes,
                         std::vector<Real> accruals,
                         const std::vector<Time>& paymentTimes,
                         Real gearingOfFloor,
                         Real gearingOfFixing,
                         Rate spreadOfFloor,
                         Rate spreadOfFixing,
                         Real initialFloor,
                         bool payer = true);
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
        Real gearingOfFloor_, gearingOfFixing_;
        Rate spreadOfFloor_, spreadOfFixing_;
        Real multiplier_;
        Size lastIndex_;
        Real initialFloor_;
        // things that vary in a path
        Real floor_;
        Size currentIndex_;
    };


    // inline definitions

    inline std::vector<Time>
    MultiStepRatchet::possibleCashFlowTimes() const {
        return paymentTimes_;
    }

    inline Size MultiStepRatchet::numberOfProducts() const {
        return 1;
    }

    inline Size
    MultiStepRatchet::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void MultiStepRatchet::reset() 
    {
       currentIndex_=0;
       floor_ = initialFloor_;
    }

}

#endif
