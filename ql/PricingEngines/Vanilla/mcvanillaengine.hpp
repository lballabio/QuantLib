/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file mcvanillaengine.hpp
    \brief Monte Carlo vanilla option engine
*/

#ifndef quantlib_mcvanilla_engine_h
#define quantlib_mcvanilla_engine_h

#include <ql/PricingEngines/mcsimulation.hpp>
#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! Pricing engine for vanilla options using Monte Carlo simulation
    /*! \ingroup vanillaengines */
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCVanillaEngine : public VanillaOption::engine,
                            public McSimulation<SingleAsset<RNG>, S> {
      public:
        void calculate() const {
            McSimulation<SingleAsset<RNG>,S>::calculate(requiredTolerance_,
                                                        requiredSamples_,
                                                        maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
      protected:
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::stats_type
            stats_type;
        // constructor
        MCVanillaEngine(Size maxTimeStepsPerYear,
                        bool brownianBridge,
                        bool antitheticVariate,
                        bool controlVariate,
                        Size requiredSamples,
                        Real requiredTolerance,
                        Size maxSamples,
                        BigNatural seed);
        // McSimulation implementation
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        Real controlVariateValue() const;
        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    // template definitions

    template<class RNG, class S>
    inline MCVanillaEngine<RNG,S>::MCVanillaEngine(Size maxTimeStepsPerYear,
                                                   bool brownianBridge,
                                                   bool antitheticVariate,
                                                   bool controlVariate,
                                                   Size requiredSamples,
                                                   Real requiredTolerance,
                                                   Size maxSamples,
                                                   BigNatural seed)
    : McSimulation<SingleAsset<RNG>,S>(antitheticVariate, controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {}

    template<class RNG, class S>
    inline
    Real MCVanillaEngine<RNG,S>::controlVariateValue() const {

        boost::shared_ptr<PricingEngine> controlPE =
                this->controlPricingEngine();
            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control variation pricing engine");

            VanillaOption::arguments* controlArguments =
                dynamic_cast<VanillaOption::arguments*>(
                    controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const VanillaOption::results* controlResults =
                dynamic_cast<const VanillaOption::results*>(
                    controlPE->results());

            return controlResults->value;
    }


    template<class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCVanillaEngine<RNG,S>::path_generator_type>
    MCVanillaEngine<RNG,S>::pathGenerator() const {

        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1,seed_);
        return boost::shared_ptr<path_generator_type>(
                         new path_generator_type(arguments_.stochasticProcess,
                                                 grid, gen, brownianBridge_));
    }



}


#endif
