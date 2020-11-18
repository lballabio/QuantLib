/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file mcdiscreteasianengine.hpp
    \brief Monte Carlo pricing engine for discrete average Asians
*/

#ifndef quantlib_mcdiscreteasian_engine_hpp
#define quantlib_mcdiscreteasian_engine_hpp

#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/pricingengines/asian/mcdiscreteasianenginebase.hpp>

namespace QuantLib {

    //! Pricing engine for discrete average Asians using Monte Carlo simulation
    /*! \deprecated Use MCDiscreteAveragingAsianEngineBase instead.
                    Deprecated in version 1.21.

        \ingroup asianengines
    */
    template<class RNG = PseudoRandom, class S = Statistics>
    class QL_DEPRECATED MCDiscreteAveragingAsianEngine :
                                public MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S> {
      public:
        typedef typename McSimulation<SingleVariate,RNG,S>::path_generator_type path_generator_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::path_pricer_type path_pricer_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::stats_type stats_type;

        MCDiscreteAveragingAsianEngine(
             const ext::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             bool controlVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
        : MCDiscreteAveragingAsianEngineBase<SingleVariate,RNG,S>(
            process, brownianBridge, antitheticVariate, controlVariate,
            requiredSamples, requiredTolerance, maxSamples, seed),
          process_(process) {}
      protected:
        // This hides the one in the base class and gives it the
        // "correct" type for existing client code.
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif
