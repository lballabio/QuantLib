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


#ifndef quantlib_onestep_forwards_hpp
#define quantlib_onestep_forwards_hpp

#include <ql/models/marketmodels/products/multiproductonestep.hpp>

namespace QuantLib {

    class OneStepForwards : public MultiProductOneStep {
      public:
        OneStepForwards(const std::vector<Time>& rateTimes,
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
    };

    // inline definitions

    inline std::vector<Time>
    OneStepForwards::possibleCashFlowTimes() const {
      return paymentTimes_;
    }

    inline Size
    OneStepForwards::numberOfProducts() const {
        return strikes_.size();
    }

    inline Size
    OneStepForwards::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void
    OneStepForwards::reset() {
        // nothing to do
    }

}


#endif



#ifndef id_78be59b24fa7626b64262c65638c7253
#define id_78be59b24fa7626b64262c65638c7253
inline bool test_78be59b24fa7626b64262c65638c7253(int* i) { return i != 0; }
#endif
