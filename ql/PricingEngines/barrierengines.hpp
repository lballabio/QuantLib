
/*
 Copyright (C) 2003 Neil Firth
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2002, 2003 Sad Rejeb
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file barrierengines.hpp
    \brief Barrier option engines

    Based on the Vanilla Engine pattern
*/

#ifndef quantlib_barrier_engines_h
#define quantlib_barrier_engines_h

#include <ql/exercise.hpp>
#include <ql/handle.hpp>
#include <ql/payoff.hpp>
#include <ql/termstructure.hpp>
#include <ql/voltermstructure.hpp>
#include <ql/Instruments/barrieroption.hpp>
#include <ql/Math/normaldistribution.hpp>
#include <ql/MonteCarlo/biasedbarrierpathpricer.hpp>
#include <ql/MonteCarlo/barrierpathpricer.hpp>
#include <ql/Pricers/barrieroption.hpp>
#include <ql/PricingEngines/genericengine.hpp>
#include <ql/PricingEngines/mcengine.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! Barrier engine base class
        class BarrierEngine 
        : public GenericEngine<Instruments::BarrierOptionArguments,
                               Instruments::BarrierOptionResults> {};
        
        //! Pricing engine for Barrier options using analytical formulae
        class AnalyticBarrierEngine : public BarrierEngine {
          public:
            void calculate() const;
          private:            
        };

        inline 
        void AnalyticBarrierEngine::calculate() const {

            //QL_REQUIRE(arguments_.exerciseType == Exercise::European,
            //    "AnalyticBarrierEngine::calculate() : "
            //    "not an European Option");

            Handle<PlainVanillaPayoff> payoff = arguments_.payoff;

            Barrier::Type barrierType = arguments_.barrierType;
            double barrier = arguments_.barrier;
            double rebate = arguments_.rebate;

            double underlying = arguments_.underlying;
            Time maturity = arguments_.maturity;

            double strike = payoff->strike();
            double variance = arguments_.volTS->blackVariance(
                arguments_.maturity, strike);
            double stdDev = QL_SQRT(variance);
            double vol = arguments_.volTS->blackVol(
                arguments_.maturity, strike);

            DiscountFactor dividendDiscount =
                arguments_.dividendTS->discount(arguments_.maturity);
            Rate dividendRate =
                arguments_.dividendTS->zeroYield(arguments_.maturity);

            DiscountFactor riskFreeDiscount =
                arguments_.riskFreeTS->discount(arguments_.maturity);
            Rate riskFreeRate =
                arguments_.riskFreeTS->zeroYield(arguments_.maturity);
            double forwardPrice = arguments_.underlying *
                dividendDiscount / riskFreeDiscount;

            
            Pricers::BarrierOption pricer = Pricers::BarrierOption(barrierType,
                          payoff->optionType(),
                          underlying,
                          strike,
                          dividendRate,
                          riskFreeRate,
                          maturity,
                          vol,
                          barrier,
                          rebate);
            
            results_.value = pricer.value();
            results_.delta = pricer.delta();
            results_.gamma = pricer.gamma();
            results_.theta = pricer.theta();
            results_.rho = pricer.rho();
            results_.dividendRho = pricer.dividendRho();
            results_.vega = pricer.vega();
            
        }

        //! Pricing engine for Barrier options using Monte Carlo
        template<class RNG = MonteCarlo::PseudoRandom, 
                 class S = Math::Statistics>
        class MCBarrierEngine 
        : public BarrierEngine,
          public McSimulation<MonteCarlo::SingleAsset<RNG>, S> {
          public:
            // constructor
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
            typedef typename
            McSimulation<MonteCarlo::SingleAsset<RNG>,S>::path_generator_type
                path_generator_type;
            typedef typename
            McSimulation<MonteCarlo::SingleAsset<RNG>,S>::path_pricer_type
                path_pricer_type;
            typedef typename
            McSimulation<MonteCarlo::SingleAsset<RNG>,S>::stats_type
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

        // Constructor
        template<class RNG, class S>
        inline
        MCBarrierEngine<RNG,S>::MCBarrierEngine(Size maxTimeStepsPerYear,
                                                  bool antitheticVariate,
                                                  bool controlVariate,
                                                  Size requiredSamples,
                                                  double requiredTolerance,
                                                  Size maxSamples,
                                                  bool isBiased,
                                                  long seed)
        : McSimulation<MonteCarlo::SingleAsset<RNG>,S>(antitheticVariate,
                                                       controlVariate),
          maxTimeStepsPerYear_(maxTimeStepsPerYear), 
          requiredSamples_(requiredSamples),
          maxSamples_(maxSamples),
          requiredTolerance_(requiredTolerance),
          isBiased_(isBiased),
          seed_(seed) {}


        // pathGenerator()
        template<class RNG, class S>
        inline
        Handle<QL_TYPENAME MCBarrierEngine<RNG,S>::path_generator_type> 
        MCBarrierEngine<RNG,S>::pathGenerator() const
        {
            Handle<DiffusionProcess> bs(new
                BlackScholesProcess(arguments_.riskFreeTS, 
                                    arguments_.dividendTS,
                                    arguments_.volTS, 
                                    arguments_.underlying));

            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen = 
                RNG::make_sequence_generator(grid.size()-1,seed_);
            return Handle<path_generator_type>(
                new path_generator_type(bs, grid, gen));
        }

        
        //   pathPricer() 
        template <class RNG, class S>
        inline
        Handle<QL_TYPENAME MCBarrierEngine<RNG,S>::path_pricer_type>
        MCBarrierEngine<RNG,S>::pathPricer() const {
            Handle<PlainVanillaPayoff> payoff = arguments_.payoff;
            // do this with Template Parameters?
            if (isBiased_) {
                return Handle<MCBarrierEngine<RNG,S>::path_pricer_type>(
                    new MonteCarlo::BiasedBarrierPathPricer(                
                        arguments_.barrierType, arguments_.barrier, 
                        arguments_.rebate, payoff->optionType(), 
                        payoff->strike(), arguments_.underlying, 
                        arguments_.riskFreeTS));
            } else {                   
                TimeGrid grid = timeGrid();
                UniformRandomSequenceGenerator sequenceGen = 
                    UniformRandomSequenceGenerator(grid.size()-1, UniformRandomGenerator(5));

                return Handle<MCBarrierEngine<RNG,S>::path_pricer_type>(
                    new MonteCarlo::BarrierPathPricer(                
                        arguments_.barrierType, arguments_.barrier, 
                        arguments_.rebate, payoff->optionType(), 
                        payoff->strike(), arguments_.underlying, 
                        arguments_.riskFreeTS,
                        Handle<DiffusionProcess> (new BlackScholesProcess(
                                    arguments_.riskFreeTS, 
                                    arguments_.dividendTS,
                                    arguments_.volTS, 
                                    arguments_.underlying)),
                        sequenceGen));
            }
        }


        // timeGrid() 
        template <class RNG, class S>
        inline
        TimeGrid MCBarrierEngine<RNG,S>::timeGrid() const {
            //try {
             //   Handle<VolTermStructures::BlackConstantVol> constVolTS = 
              //      (*(arguments_.volTS)).currentLink();
               // return TimeGrid(arguments_.maturity, 1);
            //} catch (...) {
                return TimeGrid(arguments_.maturity, 
                                Size(arguments_.maturity * 
                                     maxTimeStepsPerYear_));
            //}
        }

        // calculate()
        template<class RNG, class S>
        inline
        void MCBarrierEngine<RNG,S>::calculate() const {

            QL_REQUIRE(requiredTolerance_ != Null<double>() ||
                    int(requiredSamples_) != Null<int>(),
                    "MCBarrierEngine::calculate: "
                    "neither tolerance nor number of samples set");

            // what exercise type is a barrier option?
            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "MCBarrierEngine::calculate() : "
                "not an European Option");

            //! Initialize the one-factor Monte Carlo
            if (controlVariate_) {

                Handle<path_pricer_type> controlPP = controlPathPricer();
                QL_REQUIRE(!controlPP.isNull(),
                        "MCBarrierEngine::calculate() : "
                        "engine does not provide "
                        "control variation path pricer");

                Handle<PricingEngine> controlPE = controlPricingEngine();

                QL_REQUIRE(!controlPE.isNull(),
                        "MCBarrierEngine::calculate() : "
                        "engine does not provide "
                        "control variation pricing engine");
                /*
                BarrierOptionArguments* controlArguments =
                    dynamic_cast<BarrierOptionArguments*>(
                        controlPE->arguments());
                *controlArguments = arguments_;
                controlPE->calculate();

                const BarrierOptionResults* controlResults =
                    dynamic_cast<const BarrierOptionResults*>(
                        controlPE->results());
                double controlVariateValue = controlResults->value;

                mcModel_ = 
                    Handle<MonteCarlo::MonteCarloModel<
                            MonteCarlo::SingleAsset<RNG>, S> >(
                    new MonteCarlo::MonteCarloModel<
                            MonteCarlo::SingleAsset<RNG>, S>(
                        pathGenerator(), pathPricer(), stats_type(), 
                        antitheticVariate_, controlPP, 
                        controlVariateValue));
                */
            } else {
                mcModel_ = 
                    Handle<MonteCarlo::MonteCarloModel<
                            MonteCarlo::SingleAsset<RNG>, S> >(
                    new MonteCarlo::MonteCarloModel<
                            MonteCarlo::SingleAsset<RNG>, S>(
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
/*
            Statistics stats = mcModel_->sampleAccumulator();
            std::cout << " samples: " << stats.samples()
                << " min: " << stats.min()
                << " max: " << stats.max()
                << " mean: " << stats.mean()
                << " errorEstimate: " << stats.errorEstimate()
                << std::endl;        
*/

            results_.value = mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
                results_.errorEstimate = 
                    mcModel_->sampleAccumulator().errorEstimate();
        }

    }

}


#endif
