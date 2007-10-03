/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Warren Chou

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

/*! \file mcvarianceswapengine.hpp
    \brief Monte Carlo variance-swap engine
*/

#ifndef quantlib_mc_varianceswap_engine_hpp
#define quantlib_mc_varianceswap_engine_hpp

#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/instruments/varianceswap.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Variance-swap pricing engine using Monte Carlo simulation,
    /*! as described in Demeterfi, Derman, Kamal & Zou,
        "A Guide to Volatility and Variance Swaps", 1999

        \ingroup forwardengines

        \todo define tolerance of numerical integral and incorporate it
              in errorEstimate

        \test returned fair variances checked for consistency with
              implied volatility curve.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCVarianceSwapEngine : public VarianceSwap::engine,
                                 public McSimulation<SingleVariate,RNG,S> {
      public:
        typedef
        typename McSimulation<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef
        typename McSimulation<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCVarianceSwapEngine(Size timeSteps,
                             Size timeStepsPerYear,
                             bool brownianBridge,
                             bool antitheticVariate,
                             Size requiredSamples,
                             Real requiredTolerance,
                             Size maxSamples,
                             BigNatural seed);
        // calculate fair variance via Monte Carlo
        void calculate() const {
            McSimulation<SingleVariate,RNG,S>::calculate(requiredTolerance_,
                                                         requiredSamples_,
                                                         maxSamples_);
            results_.fairVariance =
                     this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
      protected:
        // McSimulation implementation
        boost::shared_ptr<path_pricer_type> pathPricer() const;
        TimeGrid timeGrid() const;

        boost::shared_ptr<path_generator_type> pathGenerator() const {

            Size dimensions = arguments_.stochasticProcess->factors();

            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(dimensions*(grid.size()-1),seed_);

            return boost::shared_ptr<path_generator_type>(
                         new path_generator_type(arguments_.stochasticProcess,
                                                 grid, gen, brownianBridge_));
        }
        // data members
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    //! Monte Carlo variance-swap engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCVarianceSwapEngine {
      public:
        MakeMCVarianceSwapEngine();
        // named parameters
        MakeMCVarianceSwapEngine& withSteps(Size steps);
        MakeMCVarianceSwapEngine& withStepsPerYear(Size steps);
        MakeMCVarianceSwapEngine& withBrownianBridge(bool b = true);
        MakeMCVarianceSwapEngine& withSamples(Size samples);
        MakeMCVarianceSwapEngine& withTolerance(Real tolerance);
        MakeMCVarianceSwapEngine& withMaxSamples(Size samples);
        MakeMCVarianceSwapEngine& withSeed(BigNatural seed);
        MakeMCVarianceSwapEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator boost::shared_ptr<PricingEngine>() const;
      private:
        bool antithetic_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    class FairVariancePathPricer : public PathPricer<Path> {
      public:
        FairVariancePathPricer(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process)
        : process_(process) {}
        Real operator()(const Path& path) const;
      private:
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

    // inline definitions

    template<class RNG, class S>
    inline
    MCVarianceSwapEngine<RNG,S>::MCVarianceSwapEngine(Size timeSteps,
                                                      Size timeStepsPerYear,
                                                      bool brownianBridge,
                                                      bool antitheticVariate,
                                                      Size requiredSamples,
                                                      Real requiredTolerance,
                                                      Size maxSamples,
                                                      BigNatural seed)
    : McSimulation<SingleVariate,RNG,S>(antitheticVariate, false),
      timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {}


    template <class RNG, class S>
    inline TimeGrid MCVarianceSwapEngine<RNG,S>::timeGrid() const {

        Time t = this->arguments_.stochasticProcess->time(
                                               this->arguments_.maturityDate);

        if (timeSteps_ != Null<Size>()) {
            return TimeGrid(t, this->timeSteps_);
        } else if (timeStepsPerYear_ != Null<Size>()) {
            Size steps = static_cast<Size>(timeStepsPerYear_*t);
            return TimeGrid(t, std::max<Size>(steps, 1));
        } else {
            QL_FAIL("time steps not specified");
        }
    }


    template <class RNG, class S>
    inline
    boost::shared_ptr<
        typename MCVarianceSwapEngine<RNG,S>::path_pricer_type>
    MCVarianceSwapEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<GeneralizedBlackScholesProcess> process =
            boost::dynamic_pointer_cast<GeneralizedBlackScholesProcess>(
                this->arguments_.stochasticProcess);
        QL_REQUIRE(process, "Black-Scholes process required");

        return boost::shared_ptr<
            typename MCVarianceSwapEngine<RNG,S>::path_pricer_type>(
              new FairVariancePathPricer(this->arguments_.stochasticProcess));
    }


    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>::MakeMCVarianceSwapEngine()
    : antithetic_(false), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()), brownianBridge_(false), seed_(0) {}

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withBrownianBridge(bool brownianBridge) {
        brownianBridge_ = brownianBridge;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>&
    MakeMCVarianceSwapEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG,S>::
    operator boost::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return boost::shared_ptr<PricingEngine>(new
            MCVarianceSwapEngine<RNG,S>(steps_,
                                        stepsPerYear_,
                                        brownianBridge_,
                                        antithetic_,
                                        samples_, tolerance_,
                                        maxSamples_,
                                        seed_));
    }


    namespace detail {

        class Integrand : std::unary_function<Real,Real> {
          public:
            Integrand(const Path& path,
                      const boost::shared_ptr<GeneralizedBlackScholesProcess>&
                                                                      process)
            : path_(path), process_(process) {}
            Real operator()(Time t) const {
                Size i =  static_cast<Size>(t/path_.timeGrid().dt(0));
                Real sigma = process_->diffusion(t,path_[i]);
                return sigma*sigma;
            }
          private:
            Path path_;
            boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        };

    }


    inline Real FairVariancePathPricer::operator()(const Path& path) const {
        QL_REQUIRE(path.length() > 0, "the path cannot be empty");
        Time t0 = path.timeGrid().front();
        Time t = path.timeGrid().back();
        Time dt = path.timeGrid().dt(0);
        SegmentIntegral integrator(static_cast<Size>(t/dt));
        detail::Integrand f(path, process_);
        return integrator(f,t0,t)/t;
    }

}


#endif
