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

/*! \file mcsimulation.hpp
    \brief framework for Monte Carlo engines
*/

#ifndef quantlib_montecarlo_engine_h
#define quantlib_montecarlo_engine_h

#include <ql/grid.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>

namespace QuantLib {

    //! base class for Monte Carlo engines
    /*! Eventually this class might offer greeks methods.  Deriving a
        class from McSimulation gives an easy way to write a Monte
        Carlo engine.

        See McVanillaEngine as an example.
    */

    template <class MC, class S = Statistics>
    class McSimulation {
      public:
        typedef typename MonteCarloModel<MC,S>::path_generator_type
            path_generator_type;
        typedef typename MonteCarloModel<MC,S>::path_pricer_type
            path_pricer_type;
        typedef typename MonteCarloModel<MC,S>::stats_type
            stats_type;

        virtual ~McSimulation() {}
        //! add samples until the required absolute tolerance is reached
        Real value(Real tolerance,
                   Size maxSample = QL_MAX_INTEGER) const;
        //! simulate a fixed number of samples
        Real valueWithSamples(Size samples) const;
        //! error estimated using the samples simulated so far
        Real errorEstimate() const;
        //! access to the sample accumulator for richer statistics
        const stats_type& sampleAccumulator(void) const;
        //! basic calculate method provided to inherited pricing engines
        void calculate(Real requiredTolerance,
                       Size requiredSamples,
                       Size maxSamples) const;
      protected:
        McSimulation(bool antitheticVariate,
                     bool controlVariate)
        : antitheticVariate_(antitheticVariate),
          controlVariate_(controlVariate) {}
        virtual boost::shared_ptr<path_pricer_type> pathPricer() const = 0;
        virtual boost::shared_ptr<path_generator_type> pathGenerator()
                                                                   const = 0;
        virtual TimeGrid timeGrid() const = 0;
        virtual boost::shared_ptr<path_pricer_type> controlPathPricer() const {
            return boost::shared_ptr<path_pricer_type>();
        }
        virtual boost::shared_ptr<PricingEngine> controlPricingEngine() const {
            return boost::shared_ptr<PricingEngine>();
        }
        virtual Real controlVariateValue() const {
            return Null<Real>();
        }
        mutable boost::shared_ptr<MonteCarloModel<MC,S> > mcModel_;
        static const Size minSample_;
        bool antitheticVariate_, controlVariate_;
    };


    template<class MC, class S>
    const Size McSimulation<MC,S>::minSample_ = 1023; // (2^10 - 1)

    // inline definitions
    template<class MC, class S>
    inline Real McSimulation<MC,S>::value(Real tolerance,
                                          Size maxSamples) const {
        Size sampleNumber =
            mcModel_->sampleAccumulator().samples();
        if (sampleNumber<minSample_) {
            mcModel_->addSamples(minSample_-sampleNumber);
            sampleNumber = mcModel_->sampleAccumulator().samples();
        }

        Size nextBatch;
        Real order;
        Real error = mcModel_->sampleAccumulator().errorEstimate();
        while (error > tolerance) {
            QL_REQUIRE(sampleNumber<maxSamples,
                       "max number of samples (" << maxSamples
                       << ") reached, while error (" << error
                       << ") is still above tolerance (" << tolerance << ")");

            // conservative estimate of how many samples are needed
            order = error*error/tolerance/tolerance;
            nextBatch =
                Size(std::max<Real>(sampleNumber*order*0.8-sampleNumber,
                                    minSample_));

            // do not exceed maxSamples
            nextBatch = std::min(nextBatch, maxSamples-sampleNumber);
            sampleNumber += nextBatch;
            mcModel_->addSamples(nextBatch);
            error = mcModel_->sampleAccumulator().errorEstimate();
        }

        return mcModel_->sampleAccumulator().mean();
    }


    template<class MC, class S>
    inline Real McSimulation<MC,S>::valueWithSamples(Size samples) const {

        QL_REQUIRE(samples>=minSample_,
                   "number of requested samples (" << samples
                   << ") lower than minSample_ (" << minSample_ << ")");

        Size sampleNumber = mcModel_->sampleAccumulator().samples();

        QL_REQUIRE(samples>=sampleNumber,
                   "number of already simulated samples (" << sampleNumber
                   << ") greater than requested samples (" << samples << ")");

        mcModel_->addSamples(samples-sampleNumber);

        return mcModel_->sampleAccumulator().mean();
    }


    template<class MC, class S>
    inline void McSimulation<MC,S>::calculate(Real requiredTolerance,
                                              Size requiredSamples,
                                              Size maxSamples) const {

        QL_REQUIRE(requiredTolerance != Null<Real>() ||
                   requiredSamples != Null<Size>(),
                   "neither tolerance nor number of samples set");

        //! Initialize the one-factor Monte Carlo
        if (this->controlVariate_) {

            Real controlVariateValue = this->controlVariateValue();
            QL_REQUIRE(controlVariateValue != Null<Real>(),
                       "engine does not provide "
                       "control-variation price");

            boost::shared_ptr<path_pricer_type> controlPP =
                this->controlPathPricer();
            QL_REQUIRE(controlPP,
                       "engine does not provide "
                       "control-variation path pricer");

            this->mcModel_ =
                boost::shared_ptr<MonteCarloModel<MC, S> >(
                    new MonteCarloModel<MC, S>(
                           pathGenerator(), this->pathPricer(), stats_type(),
                           this->antitheticVariate_, controlPP,
                           controlVariateValue));

        } else {
            this->mcModel_ =
                boost::shared_ptr<MonteCarloModel<MC, S> >(
                    new MonteCarloModel<MC, S>(
                           pathGenerator(), this->pathPricer(), S(),
                           this->antitheticVariate_));
        }

        if (requiredTolerance != Null<Real>()) {
            if (maxSamples != Null<Size>())
                this->value(requiredTolerance, maxSamples);
            else
                this->value(requiredTolerance);
        } else {
            this->valueWithSamples(requiredSamples);
        }

    }

    template<class MC, class S>
    inline Real McSimulation<MC,S>::errorEstimate() const {

        Size sampleNumber = mcModel_->sampleAccumulator().samples();

        QL_REQUIRE(sampleNumber>=minSample_,
                   "number of simulated samples lower than minSample_");

        return mcModel_->sampleAccumulator().errorEstimate();
    }

    template<class MC, class S>
    inline const typename McSimulation<MC,S>::stats_type&
    McSimulation<MC,S>::sampleAccumulator() const {
        return mcModel_->sampleAccumulator();
    }

}


#endif
