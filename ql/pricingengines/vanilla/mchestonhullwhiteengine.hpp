/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

/*! \file mchestonhullwhiteengine.hpp
    \brief Monte Carlo vanilla option engine for stochastic interest rates
*/

#ifndef quantlib_mc_heston_hull_white_engine_hpp
#define quantlib_mc_heston_hull_white_engine_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/processes/hybridhestonhullwhiteprocess.hpp>
#include <ql/pricingengines/vanilla/mcvanillaengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonhullwhiteengine.hpp>

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
               const boost::shared_ptr<HybridHestonHullWhiteProcess>& process,
               Size timeSteps,
               Size timeStepsPerYear,
               bool antitheticVariate,
               bool controlVariate,
               Size requiredSamples,
               Real requiredTolerance,
               Size maxSamples,
               BigNatural seed);

      protected:
        // just to avoid upcasting
        boost::shared_ptr<HybridHestonHullWhiteProcess> process_;

        boost::shared_ptr<path_pricer_type> pathPricer() const;

        boost::shared_ptr<path_pricer_type>    controlPathPricer() const;
        boost::shared_ptr<PricingEngine>       controlPricingEngine() const;
        boost::shared_ptr<path_generator_type> controlPathGenerator() const;
    };


    class HestonHullWhitePathPricer : public PathPricer<MultiPath> {
      public:
        HestonHullWhitePathPricer(
             Time exerciseTime,
             const boost::shared_ptr<Payoff> & payoff,
             const boost::shared_ptr<HybridHestonHullWhiteProcess> & process);

        Real operator()(const MultiPath& path) const;

      private:
        Time exerciseTime_;
        boost::shared_ptr<Payoff> payoff_;
        boost::shared_ptr<HybridHestonHullWhiteProcess> process_;
    };


    template<class RNG,class S>
    inline MCHestonHullWhiteEngine<RNG,S>::MCHestonHullWhiteEngine(
              const boost::shared_ptr<HybridHestonHullWhiteProcess> & process,
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


    template <class RNG,class S> inline
    boost::shared_ptr<typename MCHestonHullWhiteEngine<RNG,S>::path_pricer_type>
    MCHestonHullWhiteEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<Exercise> exercise = this->arguments_.exercise;

        QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");

        const Time exerciseTime = process_->time(exercise->lastDate());

        return boost::shared_ptr<path_pricer_type>(
             new HestonHullWhitePathPricer(exerciseTime,
                                           this->arguments_.payoff,
                                           process_));
    }

    template <class RNG, class S> inline
    boost::shared_ptr<
        typename MCHestonHullWhiteEngine<RNG,S>::path_pricer_type>
    MCHestonHullWhiteEngine<RNG,S>::controlPathPricer() const {

        boost::shared_ptr<HestonProcess> hestonProcess =
            process_->hestonProcess();

        QL_REQUIRE(hestonProcess, "first constituent of the joint stochastic "
                                  "process need to be of type HestonProcess");

        boost::shared_ptr<Exercise> exercise = this->arguments_.exercise;

        QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");

        const Time exerciseTime = process_->time(exercise->lastDate());

        return boost::shared_ptr<path_pricer_type>(
             new HestonHullWhitePathPricer(
                  exerciseTime,
                  this->arguments_.payoff,
                  process_) );
    }

    template <class RNG, class S> inline
    boost::shared_ptr<PricingEngine>
    MCHestonHullWhiteEngine<RNG,S>::controlPricingEngine() const {

        boost::shared_ptr<HestonProcess> hestonProcess =
            process_->hestonProcess();
        
        boost::shared_ptr<HullWhiteForwardProcess> hullWhiteProcess =
            process_->hullWhiteProcess();

        boost::shared_ptr<HestonModel> hestonModel(
                                              new HestonModel(hestonProcess));
        boost::shared_ptr<HullWhite> hwModel(
                              new HullWhite(hestonProcess->riskFreeRate(),
                                            hullWhiteProcess->a(),
                                            hullWhiteProcess->sigma()));

        return boost::shared_ptr<PricingEngine>(
                new AnalyticHestonHullWhiteEngine(hestonModel, hwModel, 192));
    }

    template <class RNG, class S> inline
    boost::shared_ptr<
        typename MCHestonHullWhiteEngine<RNG,S>::path_generator_type> 
    MCHestonHullWhiteEngine<RNG,S>::controlPathGenerator() const {
        
        Size dimensions = process_->factors();
        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type generator =
            RNG::make_sequence_generator(dimensions*(grid.size()-1),
                                         this->seed_);

        boost::shared_ptr<HybridHestonHullWhiteProcess> cvProcess(
            new HybridHestonHullWhiteProcess(process_->hestonProcess(),
                                             process_->hullWhiteProcess(),
                                             process_->correlation(),
                                             dimensions,
                                             true));

        return boost::shared_ptr<path_generator_type>(
                  new path_generator_type(cvProcess, grid, generator, false));
    }
}

#endif
