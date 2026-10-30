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
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_onestep_optionlets_hpp
#define quantlib_onestep_optionlets_hpp

#include <ql/models/marketmodels/products/multiproductonestep.hpp>
#include <ql/shared_ptr.hpp>

namespace QuantLib {

    class Payoff;

    class OneStepOptionlets : public MultiProductOneStep {
      public:
        OneStepOptionlets(const std::vector<Time>& rateTimes,
                          std::vector<Real> accruals,
                          const std::vector<Time>& paymentTimes,
                          std::vector<ext::shared_ptr<Payoff> >);
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
        std::vector<Real> accruals_;
        std::vector<Time> paymentTimes_;
        std::vector<ext::shared_ptr<Payoff> > payoffs_;
    };

    // inline definitions

    inline std::vector<Time>
    OneStepOptionlets::possibleCashFlowTimes() const {
      return paymentTimes_;
    }

    inline Size OneStepOptionlets::numberOfProducts() const {
        return payoffs_.size();
    }

    inline Size
    OneStepOptionlets::maxNumberOfCashFlowsPerProductPerStep() const {
        return 1;
    }

    inline void OneStepOptionlets::reset() {
        // nothing to do
    }

}

#endif
