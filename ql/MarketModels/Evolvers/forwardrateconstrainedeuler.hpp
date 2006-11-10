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
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/


#ifndef quantlib_forward_rate_euler_constrained_evolver_hpp
#define quantlib_forward_rate_euler_constrained_evolver_hpp

#include <ql/MarketModels/marketmodelconstrainedevolver.hpp>
#include <ql/MarketModels/marketmodel.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/MarketModels/browniangenerator.hpp>
#include <ql/MarketModels/driftcalculator.hpp>

namespace QuantLib {

  //! euler stepping
    class ForwardRateConstrainedEuler : public ConstrainedEvolver
        {
      public:
        ForwardRateConstrainedEuler(const boost::shared_ptr<MarketModel>&,
                                    const BrownianGeneratorFactory&,
                                    const std::vector<Size>& numeraires,
                                    Size initialStep = 0);
        //! \name MarketModelConstrainedEvolver interface
        //@{
        virtual void setConstraintType(
            const std::vector<Size>& startIndexOfSwapRate,
            const std::vector<Size>& endIndexOfSwapRate);
        virtual void setThisConstraint(
            const std::vector<Rate>& rateConstraints,
            const std::vector<bool>& isConstraintActive);
        //@}
        //! \name MarketModelEvolver interface
        //@{
        const std::vector<Size>& numeraires() const;
        Real startNewPath();
        Real advanceStep();
        Size currentStep() const;
        const CurveState& currentState() const;
        void setInitialState(const CurveState&);
        //@}
      private:
        void setForwards(const std::vector<Real>& forwards);
        // inputs
        boost::shared_ptr<MarketModel> marketModel_;
        std::vector<Size> numeraires_;
        Size initialStep_;
        boost::shared_ptr<BrownianGenerator> generator_;

        std::vector<Size> startIndexOfSwapRate_;
        std::vector<Size> endIndexOfSwapRate_;

        //often changing inputs
        std::vector<Rate> rateConstraints_;
        std::vector<bool> isConstraintActive_;

        // fixed variables
        std::vector<std::vector<Real> > fixedDrifts_;
        std::vector<std::vector<Real> > variances_;

        // working variables
        std::vector<std::vector<Real> > covariances_; // covariance of constrained rate with other rates on same step
                                                                                          // step first index
        Size n_, F_;
        CurveState curveState_;
        Size currentStep_;
        std::vector<Rate> forwards_, displacements_, logForwards_, initialLogForwards_;
        std::vector<Real> drifts1_, initialDrifts_;
        Array brownians_, correlatedBrownians_;
        std::vector<Size> alive_;
         // helper classes
        std::vector<DriftCalculator> calculators_;
    };

}

#endif
