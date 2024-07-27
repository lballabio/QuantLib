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

#include <ql/instruments/varianceswap.hpp>
#include <ql/math/integrals/segmentintegral.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <utility>

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
        MCVarianceSwapEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process,
                             Size timeSteps,
                             Size timeStepsPerYear,
                             bool brownianBridge,
                             bool antitheticVariate,
                             Size requiredSamples,
                             Real requiredTolerance,
                             Size maxSamples,
                             BigNatural seed);
        // calculate variance via Monte Carlo
        void calculate() const override {
            McSimulation<SingleVariate,RNG,S>::calculate(requiredTolerance_,
                                                         requiredSamples_,
                                                         maxSamples_);
            results_.variance =
                     this->mcModel_->sampleAccumulator().mean();

            DiscountFactor riskFreeDiscount =
                process_->riskFreeRate()->discount(arguments_.maturityDate);
            Real multiplier;
            switch (arguments_.position) {
              case Position::Long:
                multiplier = 1.0;
                break;
              case Position::Short:
                multiplier = -1.0;
                break;
              default:
                QL_FAIL("Unknown position");
            }
            multiplier *= riskFreeDiscount * arguments_.notional;

            results_.value =
                multiplier * (results_.variance - arguments_.strike);

            if constexpr (RNG::allowsErrorEstimate) {
                Real varianceError =
                    this->mcModel_->sampleAccumulator().errorEstimate();
                results_.errorEstimate = multiplier * varianceError;
            }
        }

      protected:
        // McSimulation implementation
        ext::shared_ptr<path_pricer_type> pathPricer() const override;
        TimeGrid timeGrid() const override;

        ext::shared_ptr<path_generator_type> pathGenerator() const override {

            Size dimensions = process_->factors();

            TimeGrid grid = timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(dimensions*(grid.size()-1),seed_);

            return ext::shared_ptr<path_generator_type>(
                         new path_generator_type(process_, grid, gen,
                                                 brownianBridge_));
        }
        // data members
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
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
        MakeMCVarianceSwapEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process);
        // named parameters
        MakeMCVarianceSwapEngine& withSteps(Size steps);
        MakeMCVarianceSwapEngine& withStepsPerYear(Size steps);
        MakeMCVarianceSwapEngine& withBrownianBridge(bool b = true);
        MakeMCVarianceSwapEngine& withSamples(Size samples);
        MakeMCVarianceSwapEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCVarianceSwapEngine& withMaxSamples(Size samples);
        MakeMCVarianceSwapEngine& withSeed(BigNatural seed);
        MakeMCVarianceSwapEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        bool antithetic_ = false;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_ = false;
        BigNatural seed_ = 0;
    };

    class VariancePathPricer : public PathPricer<Path> {
      public:
        VariancePathPricer(ext::shared_ptr<GeneralizedBlackScholesProcess> process)
        : process_(std::move(process)) {}
        Real operator()(const Path& path) const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

    // inline definitions

    template <class RNG, class S>
    inline MCVarianceSwapEngine<RNG, S>::MCVarianceSwapEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process,
        Size timeSteps,
        Size timeStepsPerYear,
        bool brownianBridge,
        bool antitheticVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : McSimulation<SingleVariate, RNG, S>(antitheticVariate, false), process_(std::move(process)),
      timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear), requiredSamples_(requiredSamples),
      maxSamples_(maxSamples), requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {
        QL_REQUIRE(timeSteps != Null<Size>() ||
                   timeStepsPerYear != Null<Size>(),
                   "no time steps provided");
        QL_REQUIRE(timeSteps == Null<Size>() ||
                   timeStepsPerYear == Null<Size>(),
                   "both time steps and time steps per year were provided");
        QL_REQUIRE(timeSteps != 0,
                   "timeSteps must be positive, " << timeSteps <<
                   " not allowed");
        QL_REQUIRE(timeStepsPerYear != 0,
                   "timeStepsPerYear must be positive, " << timeStepsPerYear <<
                   " not allowed");
    }


    template <class RNG, class S>
    inline TimeGrid MCVarianceSwapEngine<RNG,S>::timeGrid() const {

        Time t = this->process_->time(this->arguments_.maturityDate);

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
    ext::shared_ptr<
        typename MCVarianceSwapEngine<RNG,S>::path_pricer_type>
    MCVarianceSwapEngine<RNG,S>::pathPricer() const {

        return ext::shared_ptr<
            typename MCVarianceSwapEngine<RNG,S>::path_pricer_type>(
                                            new VariancePathPricer(process_));
    }


    template <class RNG, class S>
    inline MakeMCVarianceSwapEngine<RNG, S>::MakeMCVarianceSwapEngine(
        ext::shared_ptr<GeneralizedBlackScholesProcess> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()) {}

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
    MakeMCVarianceSwapEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
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
    operator ext::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return ext::shared_ptr<PricingEngine>(
                         new MCVarianceSwapEngine<RNG,S>(process_,
                                                         steps_,
                                                         stepsPerYear_,
                                                         brownianBridge_,
                                                         antithetic_,
                                                         samples_, tolerance_,
                                                         maxSamples_,
                                                         seed_));
    }


    namespace detail {

        class Integrand {
          public:
            Integrand(Path path, ext::shared_ptr<GeneralizedBlackScholesProcess> process)
            : path_(std::move(path)), process_(std::move(process)) {}
            Real operator()(Time t) const {
                Size i =  static_cast<Size>(t/path_.timeGrid().dt(0));
                Real sigma = process_->diffusion(t,path_[i]);
                return sigma*sigma;
            }
          private:
            Path path_;
            ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        };

    }


    inline Real VariancePathPricer::operator()(const Path& path) const {
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
