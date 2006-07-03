/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
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


#ifndef quantlib_forward_rate_evolver_hpp
#define quantlib_forward_rate_evolver_hpp

#include <ql/MarketModels/marketmodelevolver.hpp>
#include <ql/MarketModels/pseudoroot.hpp>
#include <ql/MarketModels/evolutiondescription.hpp>
#include <ql/MarketModels/browniangenerator.hpp>
#include <ql/MarketModels/driftcalculator.hpp>

namespace QuantLib {

    /*! This class does the actual gritty work of evolving the forward
        rates from one time to the next.
    */
    class ForwardRateEvolver : public MarketModelEvolver {
    public:
        ForwardRateEvolver(const boost::shared_ptr<PseudoRoot>,
                           const EvolutionDescription&,
                           const BrownianGeneratorFactory&,
                           const Array& initialForwards,
                           const Array& displacements);
		virtual ~ForwardRateEvolver();

        Real startNewPath();
        Real advanceStep();
        const CurveState& currentState() const;

    private:
        // inputs
        boost::shared_ptr<PseudoRoot> pseudoRoot_;
        EvolutionDescription evolution_;
        boost::shared_ptr<BrownianGenerator> generator_;
        Array initialForwards_, displacements_;
        // working variables
        CurveState curveState_;
        Size currentStep_;
        Array forwards_, logForwards_, initialLogForwards_;
        Array drifts_, initialDrifts_;
        std::vector<Size> alive_;
        // helper classes
        std::vector<DriftCalculator*> calculators_;
    };

}


#endif
