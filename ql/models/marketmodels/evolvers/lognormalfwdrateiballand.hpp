/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Sun Xiuxin

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

#ifndef quantlib_forward_rate_iballand_evolver_hpp
#define quantlib_forward_rate_iballand_evolver_hpp

#include <ql/models/marketmodels/evolver.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>

namespace QuantLib {

    class MarketModel;
    class BrownianGenerator;
    class BrownianGeneratorFactory;
    class LMMDriftCalculator;

    //! Iterative Predictor-Corrector
    class LogNormalFwdRateiBalland : public MarketModelEvolver {
      public:
        LogNormalFwdRateiBalland(const ext::shared_ptr<MarketModel>&,
                            const BrownianGeneratorFactory&,
                            const std::vector<Size>& numeraires,
                            Size initialStep = 0);
        //! \name MarketModel interface
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
        ext::shared_ptr<MarketModel> marketModel_;
        std::vector<Size> numeraires_;
        Size initialStep_;
        ext::shared_ptr<BrownianGenerator> generator_;
        // fixed variables
        std::vector<std::vector<Real> > fixedDrifts_;
         // working variables
        Size numberOfRates_, numberOfFactors_;
        LMMCurveState curveState_;
        Size currentStep_;
        std::vector<Rate> forwards_, displacements_, logForwards_, initialLogForwards_;
        std::vector<Real> initialDrifts_;
        std::vector<Real> brownians_, correlatedBrownians_;
        std::vector<Time> rateTaus_;
        std::vector<Size> alive_;
        //std::vector<Matrix> C_;
        // helper classes
        std::vector<LMMDriftCalculator> calculators_;
    };

}

#endif
