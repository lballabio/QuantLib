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


#ifndef quantlib_forward_rate_euler_constrained_evolver_hpp
#define quantlib_forward_rate_euler_constrained_evolver_hpp

#include <ql/models/marketmodels/constrainedevolver.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>
#include <ql/models/marketmodels/driftcomputation/lmmdriftcalculator.hpp>
#include <valarray>

namespace QuantLib {

    class MarketModel;
    class BrownianGenerator;
    class BrownianGeneratorFactory;

    //! euler stepping
    class LogNormalFwdRateEulerConstrained : public ConstrainedEvolver
        {
      public:
        LogNormalFwdRateEulerConstrained(const ext::shared_ptr<MarketModel>&,
                                         const BrownianGeneratorFactory&,
                                         const std::vector<Size>& numeraires,
                                         Size initialStep = 0);
        //! \name MarketModelConstrained interface
        //@{
        void setConstraintType(const std::vector<Size>& startIndexOfSwapRate,
                               const std::vector<Size>& endIndexOfSwapRate) override;
        void setThisConstraint(const std::vector<Rate>& rateConstraints,
                               const std::valarray<bool>& isConstraintActive) override;
        //@}
        //! \name MarketModel interface
        //@{
        const std::vector<Size>& numeraires() const override;
        Real startNewPath() override;
        Real advanceStep() override;
        Size currentStep() const override;
        const CurveState& currentState() const override;
        void setInitialState(const CurveState&) override;
        //@}
      private:
        void setForwards(const std::vector<Real>& forwards);
        // inputs
        ext::shared_ptr<MarketModel> marketModel_;
        std::vector<Size> numeraires_;
        Size initialStep_;
        ext::shared_ptr<BrownianGenerator> generator_;

        std::vector<Size> startIndexOfSwapRate_;
        std::vector<Size> endIndexOfSwapRate_;

        //often changing inputs
        std::vector<Rate> rateConstraints_;
        std::valarray<bool> isConstraintActive_;

        // fixed variables
        std::vector<std::vector<Real> > fixedDrifts_;
        std::vector<std::vector<Real> > variances_;

        // working variables
        std::vector<std::vector<Real> > covariances_; // covariance of constrained rate with other rates on same step
                                                                                          // step first index
        Size numberOfRates_, numberOfFactors_;
        LMMCurveState curveState_;
        Size currentStep_;
        std::vector<Rate> forwards_, displacements_, logForwards_, initialLogForwards_;
        std::vector<Real> drifts1_, initialDrifts_;
        std::vector<Real> brownians_, correlatedBrownians_;
        std::vector<Size> alive_;
         // helper classes
        std::vector<LMMDriftCalculator> calculators_;
    };

}

#endif
