/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mchestonhullwhiteengine.hpp
    \brief Monte Carlo vanilla option engine for stochastic interest rates
*/

#ifndef quantlib_mc_heston_hull_white_engine_hpp
#define quantlib_mc_heston_hull_white_engine_hpp

#include <ql/pricingengines/vanilla/analytichestonhullwhiteengine.hpp>
#include <ql/pricingengines/vanilla/mcvanillaengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>
#include <utility>

namespace QuantLib {

    template <class RNG = PseudoRandom, class S = Statistics>
    class MCHestonHullWhiteEngine
        : public MCVanillaEngine<MultiVariate, RNG, S> {
      public:
        typedef MCVanillaEngine<MultiVariate, RNG,S> base_type;
        typedef typename base_type::path_generator_type path_generator_type;
        typedef typename base_type::path_pricer_type path_pricer_type;
        typedef typename base_type::stats_type stats_type;
        typedef typename base_type::result_type result_type;

        MCHestonHullWhiteEngine(
               const ext::shared_ptr<HybridHestonHullWhiteProcess>& process,
               Size timeSteps,
               Size timeStepsPerYear,
               bool antitheticVariate,
               bool controlVariate,
               Size requiredSamples,
               Real requiredTolerance,
               Size maxSamples,
               BigNatural seed);

        void calculate() const override;

      protected:
        // just to avoid upcasting
        ext::shared_ptr<HybridHestonHullWhiteProcess> process_;

        ext::shared_ptr<path_pricer_type> pathPricer() const override;

        ext::shared_ptr<path_pricer_type> controlPathPricer() const override;
        ext::shared_ptr<PricingEngine> controlPricingEngine() const override;
        ext::shared_ptr<path_generator_type> controlPathGenerator() const override;
    };

    //! Monte Carlo Heston/Hull-White engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCHestonHullWhiteEngine {
      public:
        explicit MakeMCHestonHullWhiteEngine(ext::shared_ptr<HybridHestonHullWhiteProcess>);
        // named parameters
        MakeMCHestonHullWhiteEngine& withSteps(Size steps);
        MakeMCHestonHullWhiteEngine& withStepsPerYear(Size steps);
        MakeMCHestonHullWhiteEngine& withAntitheticVariate(bool b = true);
        MakeMCHestonHullWhiteEngine& withControlVariate(bool b = true);
        MakeMCHestonHullWhiteEngine& withSamples(Size samples);
        MakeMCHestonHullWhiteEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCHestonHullWhiteEngine& withMaxSamples(Size samples);
        MakeMCHestonHullWhiteEngine& withSeed(BigNatural seed);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<HybridHestonHullWhiteProcess> process_;
        Size steps_, stepsPerYear_, samples_, maxSamples_;
        bool antithetic_ = false, controlVariate_ = false;
        Real tolerance_;
        BigNatural seed_ = 0;
    };


    class HestonHullWhitePathPricer : public PathPricer<MultiPath> {
      public:
        HestonHullWhitePathPricer(Time exerciseTime,
                                  ext::shared_ptr<Payoff> payoff,
                                  ext::shared_ptr<HybridHestonHullWhiteProcess> process);

        Real operator()(const MultiPath& path) const override;

      private:
        Time exerciseTime_;
        ext::shared_ptr<Payoff> payoff_;
        ext::shared_ptr<HybridHestonHullWhiteProcess> process_;
    };


    template<class RNG,class S>
    inline MCHestonHullWhiteEngine<RNG,S>::MCHestonHullWhiteEngine(
              const ext::shared_ptr<HybridHestonHullWhiteProcess> & process,
              Size timeSteps,
              Size timeStepsPerYear,
              bool antitheticVariate,
              bool controlVariate,
              Size requiredSamples,
              Real requiredTolerance,
              Size maxSamples,
              BigNatural seed)
    : base_type(process, timeSteps, timeStepsPerYear,
                false, antitheticVariate,
                controlVariate, requiredSamples,
                requiredTolerance, maxSamples, seed),
      process_(process) {}

    template<class RNG,class S>
    inline void MCHestonHullWhiteEngine<RNG,S>::calculate() const {
        MCVanillaEngine<MultiVariate, RNG, S>::calculate();
        
        if (this->controlVariate_) {
            // control variate might lead to small negative
            // option values for deep OTM options
            this->results_.value = std::max(0.0, this->results_.value);
        }
    }
                  
    template <class RNG,class S> inline
    ext::shared_ptr<typename MCHestonHullWhiteEngine<RNG,S>::path_pricer_type>
    MCHestonHullWhiteEngine<RNG,S>::pathPricer() const {

        ext::shared_ptr<Exercise> exercise = this->arguments_.exercise;

        QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");

        const Time exerciseTime = process_->time(exercise->lastDate());

        return ext::shared_ptr<path_pricer_type>(
             new HestonHullWhitePathPricer(exerciseTime,
                                           this->arguments_.payoff,
                                           process_));
    }

    template <class RNG, class S> inline
    ext::shared_ptr<
        typename MCHestonHullWhiteEngine<RNG,S>::path_pricer_type>
    MCHestonHullWhiteEngine<RNG,S>::controlPathPricer() const {

        ext::shared_ptr<HestonProcess> hestonProcess =
            process_->hestonProcess();

        QL_REQUIRE(hestonProcess, "first constituent of the joint stochastic "
                                  "process need to be of type HestonProcess");

        ext::shared_ptr<Exercise> exercise = this->arguments_.exercise;

        QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");

        const Time exerciseTime = process_->time(exercise->lastDate());

        return ext::shared_ptr<path_pricer_type>(
             new HestonHullWhitePathPricer(
                  exerciseTime,
                  this->arguments_.payoff,
                  process_) );
    }

    template <class RNG, class S> inline
    ext::shared_ptr<PricingEngine>
    MCHestonHullWhiteEngine<RNG,S>::controlPricingEngine() const {

        ext::shared_ptr<HestonProcess> hestonProcess =
            process_->hestonProcess();

        ext::shared_ptr<HullWhiteForwardProcess> hullWhiteProcess =
            process_->hullWhiteProcess();

        ext::shared_ptr<HestonModel> hestonModel(
                                              new HestonModel(hestonProcess));
        ext::shared_ptr<HullWhite> hwModel(
                              new HullWhite(hestonProcess->riskFreeRate(),
                                            hullWhiteProcess->a(),
                                            hullWhiteProcess->sigma()));

        return ext::shared_ptr<PricingEngine>(
                new AnalyticHestonHullWhiteEngine(hestonModel, hwModel, 144));
    }

    template <class RNG, class S> inline
    ext::shared_ptr<
        typename MCHestonHullWhiteEngine<RNG,S>::path_generator_type>
    MCHestonHullWhiteEngine<RNG,S>::controlPathGenerator() const {

        Size dimensions = process_->factors();
        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type generator =
            RNG::make_sequence_generator(dimensions*(grid.size()-1),
                                         this->seed_);

        ext::shared_ptr<HybridHestonHullWhiteProcess> cvProcess(
            new HybridHestonHullWhiteProcess(process_->hestonProcess(),
                                             process_->hullWhiteProcess(),
                                             0.0,
                                             process_->discretization()));

        return ext::shared_ptr<path_generator_type>(
                  new path_generator_type(cvProcess, grid, generator, false));
    }


    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG, S>::MakeMCHestonHullWhiteEngine(
        ext::shared_ptr<HybridHestonHullWhiteProcess> process)
    : process_(std::move(process)), steps_(Null<Size>()), stepsPerYear_(Null<Size>()),
      samples_(Null<Size>()), maxSamples_(Null<Size>()), tolerance_(Null<Real>()) {}

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withSteps(Size steps) {
        steps_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withStepsPerYear(Size steps) {
        stepsPerYear_ = steps;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withControlVariate(bool b) {
        controlVariate_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withAbsoluteTolerance(Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHestonHullWhiteEngine<RNG,S>&
    MakeMCHestonHullWhiteEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline
    MakeMCHestonHullWhiteEngine<RNG,S>::operator
    ext::shared_ptr<PricingEngine>() const {
        QL_REQUIRE(steps_ != Null<Size>() || stepsPerYear_ != Null<Size>(),
                   "number of steps not given");
        QL_REQUIRE(steps_ == Null<Size>() || stepsPerYear_ == Null<Size>(),
                   "number of steps overspecified");
        return ext::shared_ptr<PricingEngine>(new
            MCHestonHullWhiteEngine<RNG,S>(process_,
                                           steps_,
                                           stepsPerYear_,
                                           antithetic_,
                                           controlVariate_,
                                           samples_,
                                           tolerance_,
                                           maxSamples_,
                                           seed_));
    }

}

#endif
