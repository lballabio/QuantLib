
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
#include <ql/MonteCarlo/mctypedefs.hpp>
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

        template<class S, class PG, class PP>
        class McSimulation {
          public:
            virtual ~McSimulation() {}
            //! add samples until the required tolerance is reached
            double value(double tolerance,
                         Size maxSample = QL_MAX_INT) const;
            //! simulate a fixed number of samples
            double valueWithSamples(Size samples) const;
            //! error estimated using the samples simulated so far
            double errorEstimate() const;
            //! access to the sample accumulator for richer statisticss
            const S& sampleAccumulator(void) const;
          protected:
            McSimulation(bool antitheticVariate,
                         bool controlVariate)
            : antitheticVariate_(antitheticVariate),
              controlVariate_(controlVariate) {}
            virtual Handle<PP> pathPricer() const = 0;
            virtual Handle<PP> controlPathPricer() const { 
                return Handle<PP>(); 
            }
            virtual Handle<PricingEngine> controlPricingEngine() const { 
                return Handle<PricingEngine>(); 
            }
            virtual Handle<PG> pathGenerator() const = 0;
            virtual TimeGrid timeGrid() const = 0;
            mutable Handle<MonteCarlo::MonteCarloModel<S, PG, PP> > mcModel_;
            static const Size minSample_;
            bool antitheticVariate_, controlVariate_;
        };


        template<class S, class PG, class PP>
        const Size McSimulation<S, PG, PP>::minSample_ = 100;

        // inline definitions
        template<class S, class PG, class PP>
        inline double McSimulation<S, PG, PP>::value(double tolerance,
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


        template<class S, class PG, class PP>
        inline double McSimulation<S, PG, PP>::valueWithSamples(Size samples)
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


        template<class S, class PG, class PP>
        inline double McSimulation<S, PG, PP>::errorEstimate() const {

            Size sampleNumber =
                mcModel_->sampleAccumulator().samples();

            QL_REQUIRE(sampleNumber>=minSample_,
                "McSimulation::errorEstimate : "
                "number of simulated samples lower than minSample_");

            return mcModel_->sampleAccumulator().errorEstimate();
        }

        template<class S, class PG, class PP>
        inline const S& McSimulation<S, PG, PP>::sampleAccumulator() const {

            return mcModel_->sampleAccumulator();
        }

    

        //! Base class for Monte Carlo vanilla option engines
        template<class S, class SG, class PG, class PP>
        class MCVanillaEngine : public VanillaEngine,
                                public McSimulation<S, PG, PP> {
          public:
            void calculate() const;
          protected:
            MCVanillaEngine(bool antitheticVariate,
                            bool controlVariate,
                            Size maxTimeStepsPerYear,
                            SG sequenceGenerator) 
            : McSimulation<S, PG, PP>(antitheticVariate,controlVariate),
              maxTimeStepsPerYear_(maxTimeStepsPerYear),
              sequenceGenerator_(sequenceGenerator) {}
            Handle<PG> pathGenerator() const;
            Size maxTimeStepsPerYear_;
            SG sequenceGenerator_;
        };


        // template definitions

        template<class S, class SG, class PG, class PP>
        inline
        Handle<PG> MCVanillaEngine<S, SG, PG, PP>::pathGenerator() const
        {
            Handle<DiffusionProcess> bs(new
                BlackScholesProcess(arguments_.riskFreeTS, 
                                    arguments_.dividendTS,
                                    arguments_.volTS, 
                                    arguments_.underlying));

            return Handle<PG>(new PG(bs, timeGrid(), sequenceGenerator_));

        }


        template<class S, class SG, class PG, class PP>
        inline void MCVanillaEngine<S, SG, PG, PP>::calculate() const {

            QL_REQUIRE(arguments_.exerciseType == Exercise::European,
                "MCVanillaEngine::calculate() : "
                "not an European Option");

            //! Initialize the one-factor Monte Carlo
            if (controlVariate_) {

                Handle<PP> controlPP = controlPathPricer();
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

                mcModel_ = Handle<MonteCarlo::MonteCarloModel<S, PG, PP> >(
                    new MonteCarlo::MonteCarloModel<S, PG, PP>(
                        pathGenerator(), pathPricer(), S(), antitheticVariate_,
                        controlPP, controlVariateValue));
           
            } else {
                mcModel_ = Handle<MonteCarlo::MonteCarloModel<S, PG, PP> >(
                    new MonteCarlo::MonteCarloModel<S, PG, PP>(
                        pathGenerator(), pathPricer(), S(), 
                        antitheticVariate_));
            }

            value(0.01);

            results_.value = mcModel_->sampleAccumulator().mean();
            results_.errorEstimate = 
                mcModel_->sampleAccumulator().errorEstimate();
        }


        //! European Vanilla option pricing engine using Monte Carlo simulation
        template<class S, class SG, class PG>
        class MCEuropeanEngine
            : public MCVanillaEngine<S, SG, PG,
                                     MonteCarlo::PathPricer<
                                        MonteCarlo::Path> > {
          public:
            MCEuropeanEngine(bool antitheticVariate,
                             bool controlVariate,
                             Size maxTimeStepPerYear,
                             SG sequenceGenerator) 
            : MCVanillaEngine<S, SG,PG, MonteCarlo::PathPricer<
                                        MonteCarlo::Path> >(antitheticVariate,
              controlVariate, maxTimeStepPerYear, sequenceGenerator) {}
          protected:
            TimeGrid timeGrid() const;
            Handle<MonteCarlo::PathPricer<MonteCarlo::Path> > 
            pathPricer() const;
        };

        template<class S, class SG, class PG>
        inline 
        Handle<MonteCarlo::PathPricer<MonteCarlo::Path> >
        MCEuropeanEngine<S, SG, PG>::pathPricer() const {
            //! Initialize the path pricer
            return Handle<MonteCarlo::PathPricer<MonteCarlo::Path> >(
                new MonteCarlo::EuropeanPathPricer(arguments_.type,
                arguments_.underlying, arguments_.strike,
                arguments_.riskFreeTS));
        }

        template<class S, class SG, class PG>
        inline TimeGrid MCEuropeanEngine<S, SG, PG>::timeGrid() const {
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

    }

}

#endif
