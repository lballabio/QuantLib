
/*
 Copyright (C) 2003, 2004 Neil Firth
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2003, 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mcbarrierengine.hpp
    \brief Monte Carlo barrier option engines
*/

#ifndef quantlib_mc_barrier_engines_hpp
#define quantlib_mc_barrier_engines_hpp

#include <ql/Instruments/barrieroption.hpp>
#include <ql/PricingEngines/mcsimulation.hpp>

namespace QuantLib {

    //! Pricing engine for barrier options using Monte Carlo simulation
    /*! Uses the Brownian-bridge correction for the barrier found in
        <i>
        Going to Extremes: Correcting Simulation Bias in Exotic
        Option Valuation - D.R. Beaglehole, P.H. Dybvig and G. Zhou
        Financial Analysts Journal; Jan/Feb 1997; 53, 1. pg. 62-68
        </i>
        and
        <i>
        Simulating path-dependent options: A new approach -
        M. El Babsiri and G. Noel
        Journal of Derivatives; Winter 1998; 6, 2; pg. 65-83
        </i>

        \ingroup barrierengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCBarrierEngine : public BarrierOption::engine,
                            public McSimulation<SingleAsset<RNG>, S> {
      public:
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::stats_type
            stats_type;
        // constructor
        MCBarrierEngine(Size maxTimeStepsPerYear,
                        bool brownianBridge,
                        bool antitheticVariate,
                        bool controlVariate,
                        Size requiredSamples,
                        Real requiredTolerance,
                        Size maxSamples,
                        bool isBiased,
                        BigNatural seed);
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
        // McSimulation implementation
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;
        // Real controlVariateValue() const;
        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool isBiased_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    class BarrierPathPricer : public PathPricer<Path> {
      public:
        BarrierPathPricer(
                      Barrier::Type barrierType,
                      Real barrier,
                      Real rebate,
                      Option::Type type,
                      Real underlying,
                      Real strike,
                      DiscountFactor discount,
                      const boost::shared_ptr<StochasticProcess>& diffProcess,
                      const PseudoRandom::ursg_type& sequenceGen);
        Real operator()(const Path& path) const;
      private:
        Real underlying_;
        Barrier::Type barrierType_;
        Real barrier_;
        Real rebate_;
        boost::shared_ptr<StochasticProcess> diffProcess_;
        PseudoRandom::ursg_type sequenceGen_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };


    class BiasedBarrierPathPricer : public PathPricer<Path> {
      public:
        BiasedBarrierPathPricer(Barrier::Type barrierType,
                                Real barrier,
                                Real rebate,
                                Option::Type type,
                                Real underlying,
                                Real strike,
                                DiscountFactor discount);
        Real operator()(const Path& path) const;
      private:
        Real underlying_;
        Barrier::Type barrierType_;
        Real barrier_;
        Real rebate_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };


    // template definitions

    template <class RNG, class S>
    inline MCBarrierEngine<RNG,S>::MCBarrierEngine(Size maxTimeStepsPerYear,
                                                   bool brownianBridge,
                                                   bool antitheticVariate,
                                                   bool controlVariate,
                                                   Size requiredSamples,
                                                   Real requiredTolerance,
                                                   Size maxSamples,
                                                   bool isBiased,
                                                   BigNatural seed)
    : McSimulation<SingleAsset<RNG>,S>(antitheticVariate, controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      isBiased_(isBiased),
      brownianBridge_(brownianBridge), seed_(seed) {}

    template <class RNG, class S>
    inline TimeGrid MCBarrierEngine<RNG,S>::timeGrid() const {

        Time t = arguments_.blackScholesProcess->riskFreeRate()
            ->dayCounter().yearFraction(
              arguments_.blackScholesProcess->riskFreeRate()->referenceDate(),
              arguments_.exercise->lastDate());

        return TimeGrid(t, Size(QL_MAX<Real>(t * maxTimeStepsPerYear_, 1.0)));
    }

    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCBarrierEngine<RNG,S>::path_generator_type>
    MCBarrierEngine<RNG,S>::pathGenerator() const
    {
        TimeGrid grid = timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1,seed_);
        return boost::shared_ptr<path_generator_type>(new
            path_generator_type(arguments_.blackScholesProcess,
                                grid, gen, brownianBridge_));
    }


    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCBarrierEngine<RNG,S>::path_pricer_type>
    MCBarrierEngine<RNG,S>::pathPricer() const {
        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<BlackScholesProcess> process =
            arguments_.blackScholesProcess;

        // do this with template parameters?
        if (isBiased_) {
            return boost::shared_ptr<
                        QL_TYPENAME MCBarrierEngine<RNG,S>::path_pricer_type>(
                new BiasedBarrierPathPricer(
                       arguments_.barrierType,
                       arguments_.barrier,
                       arguments_.rebate,
                       payoff->optionType(),
                       payoff->strike(),
                       process->stateVariable()->value(),
                       process->riskFreeRate()->discount(timeGrid().back())));
        } else {
            TimeGrid grid = timeGrid();
            PseudoRandom::ursg_type sequenceGen(grid.size()-1,
                                                PseudoRandom::urng_type(5));

            return boost::shared_ptr<
                        QL_TYPENAME MCBarrierEngine<RNG,S>::path_pricer_type>(
                new BarrierPathPricer(
                    arguments_.barrierType,
                    arguments_.barrier,
                    arguments_.rebate,
                    payoff->optionType(),
                    payoff->strike(),
                    process->stateVariable()->value(),
                    process->riskFreeRate()->discount(grid.back()),
                    process,
                    sequenceGen));
        }
    }

    /*
    template<class RNG, class S>
    inline
    Real MCBarrierEngine<RNG,S>::controlVariateValue() const {

        boost::shared_ptr<PricingEngine> controlPE =
                this->controlPricingEngine();
            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control-variation pricing engine");

            BarrierOption::arguments* controlArguments =
                dynamic_cast<BarrierOption::arguments*>(
                    controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const BarrierOption::results* controlResults =
                dynamic_cast<const BarrierOption::results*>(
                    controlPE->results());

            return controlResults->value;
    }
    */

}


#endif
