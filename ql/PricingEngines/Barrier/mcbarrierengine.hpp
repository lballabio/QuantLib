
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

    //! Pricing engine for barrier options using Monte Carlo
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
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCBarrierEngine : public BarrierOption::engine,
                            public McSimulation<SingleAsset<RNG>, S> {
      public:
        MCBarrierEngine(Size maxTimeStepsPerYear,
                        bool antitheticVariate = false,
                        bool controlVariate = false,
                        Size requiredSamples = Null<int>(),
                        double requiredTolerance = Null<double>(),
                        Size maxSamples = Null<int>(),
                        bool isBiased = false,
                        long seed = 0);

        void calculate() const;
      protected:
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::stats_type
            stats_type;

        // McSimulation implementation
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;

        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        double requiredTolerance_;
        bool isBiased_;
        long seed_;
    };


    class BarrierPathPricer : public PathPricer<Path> {
      public:
        BarrierPathPricer(
                      Barrier::Type barrierType, 
                      double barrier, 
                      double rebate, 
                      Option::Type type,
                      double underlying,
                      double strike,
                      const RelinkableHandle<TermStructure>& discountTS,
                      const boost::shared_ptr<DiffusionProcess>& diffProcess,
                      const PseudoRandom::ursg_type& sequenceGen);
        double operator()(const Path& path) const;
      private:
        double underlying_;
        Barrier::Type barrierType_;
        double barrier_;
        double rebate_;
        boost::shared_ptr<DiffusionProcess> diffProcess_;
        PseudoRandom::ursg_type sequenceGen_;
        PlainVanillaPayoff payoff_;
    };


    class BiasedBarrierPathPricer : public PathPricer<Path> {
      public:
        BiasedBarrierPathPricer(
                           Barrier::Type barrierType, 
                           double barrier, 
                           double rebate, 
                           Option::Type type,
                           double underlying,
                           double strike,
                           const RelinkableHandle<TermStructure>& discountTS);
        double operator()(const Path& path) const;
      private:
        double underlying_;
        Barrier::Type barrierType_;
        double barrier_;
        double rebate_;
        PlainVanillaPayoff payoff_;
    };


    // template definitions

    template <class RNG, class S>
    inline MCBarrierEngine<RNG,S>::MCBarrierEngine(Size maxTimeStepsPerYear,
                                                   bool antitheticVariate,
                                                   bool controlVariate,
                                                   Size requiredSamples,
                                                   double requiredTolerance,
                                                   Size maxSamples,
                                                   bool isBiased,
                                                   long seed)
    : McSimulation<SingleAsset<RNG>,S>(antitheticVariate,
                                       controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples),
      maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      isBiased_(isBiased),
      seed_(seed) {}


    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCBarrierEngine<RNG,S>::path_generator_type>
    MCBarrierEngine<RNG,S>::pathGenerator() const
    {
        boost::shared_ptr<DiffusionProcess> bs(new BlackScholesProcess(
                RelinkableHandle<TermStructure>(
                             arguments_.blackScholesProcess->riskFreeRate()),
                RelinkableHandle<TermStructure>(
                             arguments_.blackScholesProcess->dividendYield()),
                RelinkableHandle<BlackVolTermStructure>(
                             arguments_.blackScholesProcess->volatility()),
                arguments_.blackScholesProcess->stateVariable()->value()));

        TimeGrid grid = timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1,seed_);
        // BB here
        return boost::shared_ptr<path_generator_type>(new
            path_generator_type(bs, grid, gen, true));
    }


    template <class RNG, class S>
    inline 
    boost::shared_ptr<QL_TYPENAME MCBarrierEngine<RNG,S>::path_pricer_type>
    MCBarrierEngine<RNG,S>::pathPricer() const {
        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        boost::shared_ptr<BlackScholesStochasticProcess> process =
            arguments_.blackScholesProcess;

        // do this with template parameters?
        if (isBiased_) {
            return boost::shared_ptr<MCBarrierEngine<RNG,S>::path_pricer_type>(
                new BiasedBarrierPathPricer(
                    arguments_.barrierType,
                    arguments_.barrier,
                    arguments_.rebate,
                    payoff->optionType(),
                    payoff->strike(),
                    process->stateVariable()->value(),
                    RelinkableHandle<TermStructure>(process->riskFreeRate())));
        } else {
            TimeGrid grid = timeGrid();
            PseudoRandom::ursg_type sequenceGen(grid.size()-1, 
                                                PseudoRandom::urng_type(5));

            RelinkableHandle<TermStructure> riskFree(process->riskFreeRate());
            RelinkableHandle<TermStructure> dividend(process->dividendYield());
            RelinkableHandle<BlackVolTermStructure> volatility(
                                                       process->volatility());
            return boost::shared_ptr<MCBarrierEngine<RNG,S>::path_pricer_type>(
                new BarrierPathPricer(
                    arguments_.barrierType,
                    arguments_.barrier,
                    arguments_.rebate,
                    payoff->optionType(),
                    payoff->strike(),
                    process->stateVariable()->value(),
                    riskFree,
                    boost::shared_ptr<DiffusionProcess>(
                        new BlackScholesProcess(
                                          riskFree,
                                          dividend,
                                          volatility,
                                          process->stateVariable()->value())),
                    sequenceGen));
        }
    }


    template <class RNG, class S>
    inline TimeGrid MCBarrierEngine<RNG,S>::timeGrid() const {

        Time t = arguments_.blackScholesProcess->riskFreeRate()
            ->dayCounter().yearFraction(
              arguments_.blackScholesProcess->riskFreeRate()->referenceDate(),
              arguments_.exercise->lastDate());

        return TimeGrid(t, Size(QL_MAX(t * maxTimeStepsPerYear_, 1.0)));
    }


    template <class RNG, class S>
    void MCBarrierEngine<RNG,S>::calculate() const {

        QL_REQUIRE(requiredTolerance_ != Null<double>() ||
                   int(requiredSamples_) != Null<int>(),
                   "neither tolerance nor number of samples set");

        // what exercise type is a barrier option?
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");

        //! Initialize the one-factor Monte Carlo
        if (controlVariate_) {

            boost::shared_ptr<path_pricer_type> controlPP = 
                controlPathPricer();
            QL_REQUIRE(controlPP,
                       "engine does not provide "
                       "control variation path pricer");

            boost::shared_ptr<PricingEngine> controlPE = 
                controlPricingEngine();

            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control variation pricing engine");
            /*
            BarrierOption::arguments* controlArguments =
                dynamic_cast<BarrierOption::arguments*>(
                    controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const BarrierOption::results* controlResults =
                dynamic_cast<const BarrierOption::results*>(
                    controlPE->results());
            double controlVariateValue = controlResults->value;

            mcModel_ =
                boost::shared_ptr<MonteCarloModel<SingleAsset<RNG>, S> >(
                    new MonteCarloModel<SingleAsset<RNG>, S>(
                        pathGenerator(), pathPricer(), stats_type(),
                        antitheticVariate_, controlPP,
                        controlVariateValue));
            */
        } else {
            mcModel_ =
                boost::shared_ptr<MonteCarloModel<SingleAsset<RNG>, S> >(
                    new MonteCarloModel<SingleAsset<RNG>, S>(
                        pathGenerator(), pathPricer(), S(),
                        antitheticVariate_));
        }

        if (requiredTolerance_ != Null<double>()) {
            if (int(maxSamples_) != Null<int>())
                value(requiredTolerance_, maxSamples_);
            else
                value(requiredTolerance_);
        } else {
            valueWithSamples(requiredSamples_);
        }

        results_.value = mcModel_->sampleAccumulator().mean();
        if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                mcModel_->sampleAccumulator().errorEstimate();
    }

}


#endif
