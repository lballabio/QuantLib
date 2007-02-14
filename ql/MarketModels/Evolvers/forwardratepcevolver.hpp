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


#ifndef quantlib_forward_rate_pc_evolver_hpp
#define quantlib_forward_rate_pc_evolver_hpp

#include <ql/MarketModels/marketmodelevolver.hpp>
#include <ql/MarketModels/marketmodel.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/MarketModels/browniangenerator.hpp>
#include <ql/MarketModels/DriftComputation/lmmdriftcalculator.hpp>
#include <ql/MarketModels/CurveStates/lmmcurvestate.hpp>

namespace QuantLib {

    //! Predictor-Corrector
    class ForwardRatePcEvolver : public MarketModelEvolver {
      public:
        ForwardRatePcEvolver(const boost::shared_ptr<MarketModel>&,
                             const BrownianGeneratorFactory&,
                             const std::vector<Size>& numeraires,
                             Size initialStep = 0);
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
        // fixed variables
        std::vector<std::vector<Real> > fixedDrifts_;
         // working variables
        Size n_, F_;
        LMMCurveState curveState_;
        Size currentStep_;
        std::vector<Rate> forwards_, displacements_, logForwards_, initialLogForwards_;
        std::vector<Real> drifts1_, drifts2_, initialDrifts_;
        Array brownians_, correlatedBrownians_;
        std::vector<Size> alive_;
        // helper classes
        std::vector<LMMDriftCalculator> calculators_;
    };

}

#endif
