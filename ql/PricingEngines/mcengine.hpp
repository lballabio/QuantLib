
/*
 Copyright (C) 2003 Ferdinando Ametrano
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

/*! \file mcengine.hpp
    \brief base class for Monte Carlo engines
*/

// $Id$

#ifndef quantlib_montecarlo_engine_h
#define quantlib_montecarlo_engine_h

#include <ql/dataformatters.hpp>
#include <ql/grid.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>
#include <ql/MonteCarlo/europeanpathpricer.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>
#include <ql/Volatilities/blackconstantvol.hpp>

namespace QuantLib {

    namespace PricingEngines {

        //! base class for Monte Carlo engines
        /*! Eventually this class might offer greeks methods.
            Deriving a class from McEngine gives an easy way to write
            a Monte Carlo engine.
            See McVanillaEngine as example of one factor engine,
            McBasketEngine as example of multi factor engine.
        */

        template <class MC,
                  class RNG = MonteCarlo::PseudoRandom,
                  class S = Math::Statistics>
        class McSimulation {
          public:
            typedef 
            typename MonteCarlo::MonteCarloModel<MC,RNG,S>::path_generator_type
                path_generator_type;
            typedef 
            typename MonteCarlo::MonteCarloModel<MC,RNG,S>::path_pricer_type
                path_pricer_type;
            typedef 
            typename MonteCarlo::MonteCarloModel<MC,RNG,S>::stats_type
                stats_type;

            virtual ~McSimulation() {}
            //! add samples until the required tolerance is reached
            double value(double tolerance,
                         Size maxSample = QL_MAX_INT) const;
            //! simulate a fixed number of samples
            double valueWithSamples(Size samples) const;
            //! error estimated using the samples simulated so far
            double errorEstimate() const;
            //! access to the sample accumulator for richer statistics
            const stats_type& sampleAccumulator(void) const;
          protected:
            McSimulation(bool antitheticVariate,
                         bool controlVariate)
            : antitheticVariate_(antitheticVariate),
              controlVariate_(controlVariate) {}
            virtual Handle<path_pricer_type> pathPricer() const = 0;
            virtual Handle<path_pricer_type> controlPathPricer() const { 
                return Handle<path_pricer_type>(); 
            }
            virtual Handle<PricingEngine> controlPricingEngine() const { 
                return Handle<PricingEngine>(); 
            }
            virtual Handle<path_generator_type> pathGenerator() const = 0;
            virtual TimeGrid timeGrid() const = 0;
            mutable Handle<MonteCarlo::MonteCarloModel<MC,RNG,S> > mcModel_;
            static const Size minSample_;
            bool antitheticVariate_, controlVariate_;
        };


        template<class MC, class RNG, class S>
        const Size McSimulation<MC,RNG,S>::minSample_ = 100;

        // inline definitions
        template<class MC, class RNG, class S>
        inline double McSimulation<MC,RNG,S>::value(double tolerance,
                                                    Size maxSamples) const {
            Size sampleNumber =
                mcModel_->sampleAccumulator().samples();
            if (sampleNumber<minSample_) {
                mcModel_->addSamples(minSample_-sampleNumber);
                sampleNumber = mcModel_->sampleAccumulator().samples();
            }

            Size nextBatch;
            double order;
            double result = mcModel_->sampleAccumulator().mean();
            double accuracy = mcModel_->sampleAccumulator().errorEstimate()/
                result;
            while (accuracy > tolerance) {
                // conservative estimate of how many samples are needed 
                order = accuracy*accuracy/tolerance/tolerance;
                nextBatch = Size(
                    QL_MAX(sampleNumber*order*0.8-sampleNumber,
                    double(minSample_)));

                // do not exceed maxSamples
                nextBatch = QL_MIN(nextBatch, maxSamples-sampleNumber);
                QL_REQUIRE(nextBatch>0,
                    "McSimulation::value : "
                    "max number of samples exceeded");

                sampleNumber += nextBatch;
                mcModel_->addSamples(nextBatch);
                result = mcModel_->sampleAccumulator().mean();
                accuracy = mcModel_->sampleAccumulator().errorEstimate()/
                    result;
            }

            return result;
        }


        template<class MC, class RNG, class S>
        inline double McSimulation<MC,RNG,S>::valueWithSamples(Size samples)
            const {

            QL_REQUIRE(samples>=minSample_,
                "McSimulation::valueWithSamples : "
                "number of requested samples ("
                + IntegerFormatter::toString(samples) +
                ") lower than minSample_ ("
                + IntegerFormatter::toString(minSample_) +
                ")");

            Size sampleNumber =
                mcModel_->sampleAccumulator().samples();

            QL_REQUIRE(samples>=sampleNumber,
                "McSimulation::valueWithSamples : "
                "number of already simulated samples ("
                + IntegerFormatter::toString(sampleNumber) +
                ") greater than"
                "requested samples ("
                + IntegerFormatter::toString(samples) +
                ")");

            mcModel_->addSamples(samples-sampleNumber);

            return mcModel_->sampleAccumulator().mean();
        }


        template<class MC, class RNG, class S>
        inline double McSimulation<MC,RNG,S>::errorEstimate() const {

            Size sampleNumber =
                mcModel_->sampleAccumulator().samples();

            QL_REQUIRE(sampleNumber>=minSample_,
                "McSimulation::errorEstimate : "
                "number of simulated samples lower than minSample_");

            return mcModel_->sampleAccumulator().errorEstimate();
        }

        template<class MC, class RNG, class S>
        inline const typename McSimulation<MC,RNG,S>::stats_type& 
        McSimulation<MC,RNG,S>::sampleAccumulator() const {
            return mcModel_->sampleAccumulator();
        }

    

        //! Base class for Monte Carlo vanilla option engines
        template<class RNG = MonteCarlo::PseudoRandom, 
                 class S = Math::Statistics>
        class MCVanillaEngine : public VanillaEngine,
                                public McSimulation<MonteCarlo::SingleAsset, 
                                                    RNG, S> {
          public:
            void calculate() const;
          protected:
            typedef typename
            McSimulation<MonteCarlo::SingleAsset,RNG,S>::path_generator_type
                path_generator_type;
            typedef typename
            McSimulation<MonteCarlo::SingleAsset,RNG,S>::path_pricer_type
                path_pricer_type;
            typedef typename
            McSimulation<MonteCarlo::SingleAsset,RNG,S>::stats_type
                stats_type;
            // constructor
            MCVanillaEngine(Size maxTimeStepsPerYear,
                            bool antitheticVariate = false,
                            bool controlVariate = false,
                            Size requiredSamples = Null<int>(),
                            double requiredTolerance = Null<double>(),
                            Size maxSamples = Null<int>(),
                            long seed = 0);
            // McSimulation implementation
            Handle<path_generator_type> pathGenerator() const;
            // data members
            Size maxTimeStepsPerYear_;
            Size requiredSamples_, maxSamples_;
            double requiredTolerance_;
            long seed_;
        };


        // inline definitions

        template<class RNG, class S>        
        inline
        MCVanillaEngine<RNG,S>::MCVanillaEngine(Size maxTimeStepsPerYear,
                                                bool antitheticVariate,
                                                bool controlVariate,
                                                Size requiredSamples,
                                                double requiredTolerance,
                                                Size maxSamples,
                                                long seed)
        : McSimulation<MonteCarlo::SingleAsset,RNG,S>(antitheticVariate,
                                                      controlVariate),
          maxTimeStepsPerYear_(maxTimeStepsPerYear), 
          requiredSamples_(requiredSamples),
          maxSamples_(maxSamples),
          requiredTolerance_(requiredTolerance),
          seed_(seed) {}

        // template definitions

        template<class RNG, class S>
        inline Handle<typename MCVanillaEngine<RNG,S>::path_generator_type> 
        MCVanillaEngine<RNG,S>::pathGenerator() const
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


        template<class RNG, class S>
        inline void MCVanillaEngine<RNG,S>::calculate() const {

            QL_REQUIRE(requiredTolerance_ != Null<double>() ||
                       requiredSamples_ != Null<int>(),
                       "MCVanillaEngine::calculate: "
                       "neither tolerance nor number of samples set");

            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "MCVanillaEngine::calculate() : "
                "not an European Option");

            //! Initialize the one-factor Monte Carlo
            if (controlVariate_) {

                Handle<path_pricer_type> controlPP = controlPathPricer();
                QL_REQUIRE(!controlPP.isNull(),
                           "MCVanillaEngine::calculate() : "
                           "engine does not provide "
                           "control variation path pricer");

                Handle<PricingEngine> controlPE = controlPricingEngine();

                QL_REQUIRE(!controlPE.isNull(),
                           "MCVanillaEngine::calculate() : "
                           "engine does not provide "
                           "control variation pricing engine");

                VanillaOptionArguments* controlArguments =
                    dynamic_cast<VanillaOptionArguments*>(
                        controlPE->arguments());
                *controlArguments = arguments_;
                controlPE->calculate();

                const VanillaOptionResults* controlResults =
                    dynamic_cast<const VanillaOptionResults*>(
                        controlPE->results());
                double controlVariateValue = controlResults->value;

                mcModel_ = 
                    Handle<MonteCarlo::MonteCarloModel<MonteCarlo::SingleAsset,
                                                       RNG, S> >(
                       new MonteCarlo::MonteCarloModel<MonteCarlo::SingleAsset,
                                                       RNG, S>(
                           pathGenerator(), pathPricer(), stats_type(), 
                           antitheticVariate_, controlPP, 
                           controlVariateValue));
           
            } else {
                mcModel_ = 
                    Handle<MonteCarlo::MonteCarloModel<MonteCarlo::SingleAsset,
                                                       RNG, S> >(
                       new MonteCarlo::MonteCarloModel<MonteCarlo::SingleAsset,
                                                       RNG, S>(
                           pathGenerator(), pathPricer(), S(), 
                           antitheticVariate_));
            }

            if (requiredTolerance_ != Null<double>()) {
                if (maxSamples_ != Null<int>())
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


        //! European Vanilla option pricing engine using Monte Carlo simulation
        template <class RNG = MonteCarlo::PseudoRandom, 
                  class S = Math::Statistics>
        class MCEuropeanEngine : public MCVanillaEngine<RNG,S> {
          public:
            typedef typename MCVanillaEngine<RNG,S>::path_generator_type
                path_generator_type;
            typedef typename MCVanillaEngine<RNG,S>::path_pricer_type
                path_pricer_type;
            typedef typename MCVanillaEngine<RNG,S>::stats_type
                stats_type;
            MCEuropeanEngine(Size maxTimeStepPerYear,
                             bool antitheticVariate = false,
                             bool controlVariate = false,
                             Size requiredSamples = Null<int>(),
                             double requiredTolerance = Null<double>(),
                             Size maxSamples = Null<int>(),
                             long seed = 0);
          protected:
            TimeGrid timeGrid() const;
            Handle<path_pricer_type> pathPricer() const;
        };

        // helper class for easier instantiation
        template <class RNG = MonteCarlo::PseudoRandom, 
                  class S = Math::Statistics>
        class MakeMCEuropeanEngine {
          public:
            MakeMCEuropeanEngine();
            // named parameters
            MakeMCEuropeanEngine& withStepsPerYear(Size maxSteps);
            MakeMCEuropeanEngine& withSamples(Size samples);
            MakeMCEuropeanEngine& withTolerance(double tolerance);
            MakeMCEuropeanEngine& withMaxSamples(Size samples);
            MakeMCEuropeanEngine& withSeed(long seed);
            MakeMCEuropeanEngine& withAntitheticVariate();
            MakeMCEuropeanEngine& withControlVariate();
            // conversion to pricing engine
            operator Handle<PricingEngine>() const;
          private:
            bool antithetic_, controlVariate_;
            Size steps_, samples_, maxSamples_;
            double tolerance_;
            long seed_;
        };


        // inline definitions

        template <class RNG, class S>
        inline 
        MCEuropeanEngine<RNG,S>::MCEuropeanEngine(Size maxTimeStepPerYear,
                                                  bool antitheticVariate,
                                                  bool controlVariate,
                                                  Size requiredSamples,
                                                  double requiredTolerance,
                                                  Size maxSamples,
                                                  long seed)
        : MCVanillaEngine<RNG,S>(maxTimeStepPerYear,
                                 antitheticVariate,
                                 controlVariate, 
                                 requiredSamples,
                                 requiredTolerance,
                                 maxSamples,
                                 seed) {}

        template <class RNG, class S>
        inline 
        Handle<typename MCEuropeanEngine<RNG,S>::path_pricer_type>
        MCEuropeanEngine<RNG,S>::pathPricer() const {
            //! Initialize the path pricer
            return Handle<MCEuropeanEngine<RNG,S>::path_pricer_type>(
                new MonteCarlo::EuropeanPathPricer(arguments_.type,
                                                   arguments_.underlying, 
                                                   arguments_.strike,
                                                   arguments_.riskFreeTS));
        }

        template <class RNG, class S>
        inline TimeGrid MCEuropeanEngine<RNG,S>::timeGrid() const {
            try {
                Handle<VolTermStructures::BlackConstantVol> constVolTS = 
                    (*(arguments_.volTS)).currentLink();
                return TimeGrid(arguments_.maturity, 1);
            } catch (...) {
                return TimeGrid(arguments_.maturity, 
                                Size(arguments_.maturity * 
                                     maxTimeStepsPerYear_));
            }
        }


        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>::MakeMCEuropeanEngine()
        : antithetic_(false), controlVariate_(false),
          steps_(Null<int>()), samples_(Null<int>()), maxSamples_(Null<int>()),
          tolerance_(Null<double>()), seed_(0) {}

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withStepsPerYear(Size maxSteps) {
            steps_ = maxSteps;
            return *this;
        }

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withSamples(Size samples) {
            QL_REQUIRE(tolerance_ == Null<double>(),
                       "MakeMCEuropeanEngine::withSamples: "
                       "tolerance already set");
            samples_ = samples;
            return *this;
        }

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withTolerance(double tolerance) {
            QL_REQUIRE(samples_ == Size(Null<int>()),
                       "MakeMCEuropeanEngine::withTolerance: "
                       "number of samples already set");
            QL_REQUIRE(RNG::allowsErrorEstimate,
                       "MakeMCEuropeanEngine::withTolerance: "
                       "chosen random generator policy "
                       "does not allow an error estimate");
            tolerance_ = tolerance;
            return *this;
        }

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withMaxSamples(Size samples) {
            maxSamples_ = samples;
            return *this;
        }

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withSeed(long seed) {
            seed_ = seed;
            return *this;
        }

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withAntitheticVariate() {
            antithetic_ = true;
            return *this;
        }

        template <class RNG, class S>
        inline MakeMCEuropeanEngine<RNG,S>&
        MakeMCEuropeanEngine<RNG,S>::withControlVariate() {
            controlVariate_ = true;
            return *this;
        }

        template <class RNG, class S>
        inline 
        MakeMCEuropeanEngine<RNG,S>::operator Handle<PricingEngine>() const {
            QL_REQUIRE(steps_ != Size(Null<int>()),
                       "MakeMCEuropeanEngine<RNG,S>: "
                       "max number of steps per year not given");
            return Handle<PricingEngine>(
                new MCEuropeanEngine<RNG,S>(steps_, antithetic_, 
                                            controlVariate_, 
                                            samples_, tolerance_, 
                                            maxSamples_, seed_));
        }

    }

}


#endif
