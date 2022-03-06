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

#ifndef quantlib_forward_rate_euler_evolver_hpp
#define quantlib_forward_rate_euler_evolver_hpp

#include <ql/models/marketmodels/evolver.hpp>
#include <ql/models/marketmodels/curvestates/lmmcurvestate.hpp>

namespace QuantLib {

    class MarketModel;
    class BrownianGenerator;
    class BrownianGeneratorFactory;
    class LMMDriftCalculator;

    //! Euler
    class LogNormalFwdRateEuler : public MarketModelEvolver {
      public:
        LogNormalFwdRateEuler(const ext::shared_ptr<MarketModel>&,
                              const BrownianGeneratorFactory&,
                              const std::vector<Size>& numeraires,
                              Size initialStep = 0);
        //! \name MarketModel interface
        //@{
        const std::vector<Size>& numeraires() const override;
        Real startNewPath() override;
        Real advanceStep() override;
        Size currentStep() const override;
        const CurveState& currentState() const override;
        void setInitialState(const CurveState&) override;
        //@}

        //! accessor methods useful for doing pathwise vegas
        const std::vector<Real>& browniansThisStep() const
        {
            return brownians_;
        }
     


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
        std::vector<Real> drifts1_, initialDrifts_;
        std::vector<Real> brownians_, correlatedBrownians_;
        std::vector<Size> alive_;
        // helper classes
        std::vector<LMMDriftCalculator> calculators_;
    };

}

#endif


#ifndef id_ba86fd119330b6f4f77f869b2a90dbe9
#define id_ba86fd119330b6f4f77f869b2a90dbe9
inline bool test_ba86fd119330b6f4f77f869b2a90dbe9(int* i) { return i != 0; }
#endif
