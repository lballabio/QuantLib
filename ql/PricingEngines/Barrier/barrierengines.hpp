
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

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

/*! \file barrierengines.hpp
    \brief Barrier option engines
*/

#ifndef quantlib_barrier_engines_h
#define quantlib_barrier_engines_h

#include <ql/Instruments/barrieroption.hpp>
#include <ql/PricingEngines/mcsimulation.hpp>
#include <ql/MonteCarlo/barrierpathpricer.hpp>
#include <ql/MonteCarlo/biasedbarrierpathpricer.hpp>

namespace QuantLib {

    //! Barrier engine base class
    class BarrierEngine : public GenericEngine<BarrierOption::arguments,
                                               BarrierOption::results> {};

    //! Pricing engine for Barrier options using analytical formulae
    /*! The formulas are taken from "Option pricing formulas",
         E.G. Haug, McGraw-Hill, p.69 and following.
    */
    class AnalyticBarrierEngine : public BarrierEngine {
      public:
        void calculate() const;
      private:
        CumulativeNormalDistribution f_;
        // helper methods
        double underlying() const;
        double strike() const;
        Time residualTime() const;
        double volatility() const;
        double barrier() const;
        double rebate() const;
        double stdDeviation() const;
        Rate riskFreeRate() const;
        DiscountFactor riskFreeDiscount() const;
        Rate dividendYield() const;
        DiscountFactor dividendDiscount() const;
        double mu() const;
        double muSigma() const;
        double A(double phi) const;
        double B(double phi) const;
        double C(double eta, double phi) const;
        double D(double eta, double phi) const;
        double E(double eta) const;
        double F(double eta) const;
    };

    //! Pricing engine for Barrier options using Monte Carlo
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCBarrierEngine : public BarrierEngine,
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
        Handle<path_generator_type> pathGenerator() const;
        TimeGrid timeGrid() const;
        Handle<path_pricer_type> pathPricer() const;

        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        double requiredTolerance_;
        bool isBiased_;
        long seed_;
    };


    // template definitions

    template<class RNG, class S>
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


    template<class RNG, class S>
    inline
    Handle<QL_TYPENAME MCBarrierEngine<RNG,S>::path_generator_type>
    MCBarrierEngine<RNG,S>::pathGenerator() const
    {
        Handle<DiffusionProcess> bs(new
            BlackScholesProcess(
                arguments_.blackScholesProcess->riskFreeTS,
                arguments_.blackScholesProcess->dividendTS,
                arguments_.blackScholesProcess->volTS,
                arguments_.blackScholesProcess->stateVariable->value()));

        TimeGrid grid = timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1,seed_);
        return Handle<path_generator_type>(
                                      new path_generator_type(bs, grid, gen));
    }


    template <class RNG, class S>
    inline Handle<QL_TYPENAME MCBarrierEngine<RNG,S>::path_pricer_type>
    MCBarrierEngine<RNG,S>::pathPricer() const {
        #if defined(HAVE_BOOST)
        Handle<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff,
                   "MCBarrierEngine: non-plain payoff given");
        #else
        Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
        #endif

        // do this with Template Parameters?
        if (isBiased_) {
            return Handle<MCBarrierEngine<RNG,S>::path_pricer_type>(
                new BiasedBarrierPathPricer(
                    arguments_.barrierType,
                    arguments_.barrier,
                    arguments_.rebate,
                    payoff->optionType(),
                    payoff->strike(),
                    arguments_.blackScholesProcess->stateVariable->value(),
                    arguments_.blackScholesProcess->riskFreeTS));
        } else {
            TimeGrid grid = timeGrid();
            UniformRandomSequenceGenerator
                sequenceGen(grid.size()-1, UniformRandomGenerator(5));

            return Handle<MCBarrierEngine<RNG,S>::path_pricer_type>(
                new BarrierPathPricer(
                    arguments_.barrierType,
                    arguments_.barrier,
                    arguments_.rebate,
                    payoff->optionType(),
                    payoff->strike(),
                    arguments_.blackScholesProcess->stateVariable->value(),
                    arguments_.blackScholesProcess->riskFreeTS,
                    Handle<DiffusionProcess>(new
                        BlackScholesProcess(
                            arguments_.blackScholesProcess->riskFreeTS,
                            arguments_.blackScholesProcess->dividendTS,
                            arguments_.blackScholesProcess->volTS,
                            arguments_.blackScholesProcess->stateVariable->value())),
                    sequenceGen));
        }
    }


    template <class RNG, class S>
    inline TimeGrid MCBarrierEngine<RNG,S>::timeGrid() const {

        Time t = arguments_.blackScholesProcess->riskFreeTS->dayCounter().yearFraction(
            arguments_.blackScholesProcess->riskFreeTS->referenceDate(),
            arguments_.exercise->lastDate());

        return TimeGrid(t, Size(t * maxTimeStepsPerYear_));
    }


    template<class RNG, class S>
    void MCBarrierEngine<RNG,S>::calculate() const {

        QL_REQUIRE(requiredTolerance_ != Null<double>() ||
                   int(requiredSamples_) != Null<int>(),
                   "MCBarrierEngine::calculate: "
                   "neither tolerance nor number of samples set");

        // what exercise type is a barrier option?
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "MCBarrierEngine::calculate() : "
                   "not an European Option");

        //! Initialize the one-factor Monte Carlo
        if (controlVariate_) {

            Handle<path_pricer_type> controlPP = controlPathPricer();
            QL_REQUIRE(!IsNull(controlPP),
                       "MCBarrierEngine::calculate() : "
                       "engine does not provide "
                       "control variation path pricer");

            Handle<PricingEngine> controlPE = controlPricingEngine();

            QL_REQUIRE(!IsNull(controlPE),
                       "MCBarrierEngine::calculate() : "
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
                Handle<MonteCarloModel<SingleAsset<RNG>, S> >(
                    new MonteCarloModel<SingleAsset<RNG>, S>(
                        pathGenerator(), pathPricer(), stats_type(),
                        antitheticVariate_, controlPP,
                        controlVariateValue));
            */
        } else {
            mcModel_ =
                Handle<MonteCarloModel<SingleAsset<RNG>, S> >(
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
