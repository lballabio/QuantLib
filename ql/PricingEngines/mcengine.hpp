
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

    \fullpath
    ql/PricingEngines/%mcengine.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_engine_h
#define quantlib_montecarlo_engine_h

#include <ql/dataformatters.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>
#include <ql/PricingEngines/vanillaengines.hpp>

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
        class McEngine {
          public:
            virtual ~McEngine() {}
            //! add samples until the required tolerance is reached
            double value(double tolerance,
                         Size maxSample = QL_MAX_INT) const;
            //! simulate a fixed number of samples
            double valueWithSamples(Size samples) const;
            //! error estimated using the samples simulated so far
            double errorEstimate() const;
            //! access to the sample accumulator for richer statistics
            const S& sampleAccumulator(void) const;
          protected:
            McEngine() {}
            virtual Handle<PP> pathPricer() const = 0;
            virtual Handle<PP> controlPathPricer() const { return Handle<PP>(); }
            virtual Handle<PricingEngine> controlPricingEngine() const { return Handle<PricingEngine>(); }
            virtual Handle<PG> pathGenerator() const = 0;
            mutable Handle<MonteCarlo::MonteCarloModel<S, PG, PP> > mcModel_;
            static const Size minSample_;
        };


        template<class S, class PG, class PP>
        const Size McEngine<S, PG, PP>::minSample_ = 100;

        // inline definitions
        template<class S, class PG, class PP>
        inline double McEngine<S, PG, PP>::value(double tolerance,
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
                    "McEngine::value : "
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
        inline double McEngine<S, PG, PP>::valueWithSamples(Size samples)
            const {

            QL_REQUIRE(samples>=minSample_,
                "McEngine::valueWithSamples : "
                "number of requested samples ("
                + IntegerFormatter::toString(samples) +
                ") lower than minSample_ ("
                + IntegerFormatter::toString(minSample_) +
                ")");

            Size sampleNumber =
                mcModel_->sampleAccumulator().samples();

            QL_REQUIRE(samples>=sampleNumber,
                "McEngine::valueWithSamples : "
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
        inline double McEngine<S, PG, PP>::errorEstimate() const {

            Size sampleNumber =
                mcModel_->sampleAccumulator().samples();

            QL_REQUIRE(sampleNumber>=minSample_,
                "McEngine::errorEstimate : "
                "number of simulated samples lower than minSample_");

            return mcModel_->sampleAccumulator().errorEstimate();
        }

        template<class S, class PG, class PP>
        inline const S& McEngine<S, PG, PP>::sampleAccumulator() const {

            return mcModel_->sampleAccumulator();
        }

    


        //! Base class for Vanilla option Pricing engines using Monte Carlo simulation
        template<class S, class PG, class PP>
        class MCVanillaEngine : public VanillaEngine,
                                public McEngine<S, PG, PP> {
          public:
            void calculate() const;
          protected:
            MCVanillaEngine(bool antitheticVariance,
                            bool controlVariate,
                            long seed=0) 
            : antitheticVariance_(antitheticVariance),
              controlVariate_(controlVariate), seed_(seed) {}
            Handle<PG> pathGenerator() const;
            bool antitheticVariance_, controlVariate_;
          private:
            long seed_;
        };



        template<class S, class PG, class PP>
        inline Handle<PG> MCVanillaEngine<S, PG, PP>::pathGenerator() const {
            Date referenceDate = arguments_.riskFreeTS->referenceDate();
            Date exerciseDate = arguments_.exercise.date();
            Time residualTime = arguments_.riskFreeTS->dayCounter().yearFraction(
                referenceDate, exerciseDate);

            //! Initialize the path generator
            Handle<TermStructure> drift(new
                TermStructures::DriftTermStructure(arguments_.riskFreeTS,
                                                   arguments_.dividendTS,
                                                   arguments_.volTS));
            double mu = drift->zeroYield(exerciseDate);
            double volatility = arguments_.volTS->blackVol(
                arguments_.exercise.date(), arguments_.underlying);

            return Handle<MonteCarlo::GaussianPathGenerator>(
                new MonteCarlo::GaussianPathGenerator(mu,
                    volatility*volatility, residualTime, 1, seed_));

        }


        template<class S, class PG, class PP>
        inline void MCVanillaEngine<S, PG, PP>::calculate() const {

            QL_REQUIRE(arguments_.exercise.type() == Exercise::European,
                "MCVanillaEngine::calculate() : "
                "not an European Option");

            //! Initialize the one-factor Monte Carlo
            if (controlVariate_) {
/*
                Handle<PP> controlPP = controlPathPricer();
                QL_REQUIRE(!controlPP.isNull(),
                    "MCVanillaEngine::calculate() : "
                    "control variation path pricer not provided by the engine");

                Handle<PricingEngine> controlPE = controlPricingEngine();

                QL_REQUIRE(!controlPE.isNull(),
                    "MCVanillaEngine::calculate() : "
                    "control variation pricing engine not provided by the MC engine");


                VanillaOptionArguments* controlArguments =
                    dynamic_cast<VanillaOptionArguments>(
                        controlPE->arguments());
                controlArguments = arguments_;
                controlPE->calculate();
                VanillaOptionResults* controlResults =
                    dynamic_cast<VanillaOptionResults>(
                        controlPE->results());
                double controlVariateValue = controlResults.value;

                mcModel_ = Handle<MonteCarlo::MonteCarloModel<S, PG, PP> >(
                    new MonteCarlo::MonteCarloModel<S, PG, PP>(
                        pathGenerator(), pathPricer(), S(),
                        controlPP, controlVariateValue));
*/
                mcModel_ = Handle<MonteCarlo::MonteCarloModel<S, PG, PP> >(
                    new MonteCarlo::MonteCarloModel<S, PG, PP>(
                        pathGenerator(), pathPricer(), S()));

            
            } else {
                mcModel_ = Handle<MonteCarlo::MonteCarloModel<S, PG, PP> >(
                    new MonteCarlo::MonteCarloModel<S, PG, PP>(
                        pathGenerator(), pathPricer(), S()));
            }


            value(0.005);

            results_.value       = mcModel_->sampleAccumulator().mean();
            results_.delta       = 0.0;
            results_.gamma       = 0.0;
            results_.theta       = 0.0;
            results_.rho         = 0.0;
            results_.dividendRho = 0.0;
            results_.vega        = 0.0;
//            results_.errorEstimate = mcModel_->sampleAccumulator().errorEstimate()

        }


        //! European Vanilla option pricing engine using Monte Carlo simulation
        template<class S, class PG, class PP>
        class MCEuropeanVanillaEngine : public MCVanillaEngine<S, PG, PP> {
          public:
            MCEuropeanVanillaEngine(bool antitheticVariance,
                                    bool controlVariate,
                                    long seed=0) 
            : MCVanillaEngine<S, PG, PP>(antitheticVariance, controlVariate,
              seed) {}
          protected:
            Handle<PP> pathPricer() const;
        };

        template<class S, class PG, class PP>
        inline Handle<PP> MCEuropeanVanillaEngine<S, PG, PP>::pathPricer() const {
            //! Initialize the path pricer
            return Handle<MonteCarlo::PathPricer<MonteCarlo::Path> >(
                new MonteCarlo::EuropeanPathPricer(arguments_.type,
                arguments_.underlying, arguments_.strike,
                arguments_.riskFreeTS->discount(arguments_.exercise.date()),
                antitheticVariance_));
        }


    }

}

#endif
