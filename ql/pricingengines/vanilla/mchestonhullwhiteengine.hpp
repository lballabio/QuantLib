/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Klaus Spanderen

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

/*! \file mchestonhullwhiteengine.hpp
    \brief Monte Carlo vanilla option engine for stochastic interest rates
*/

#ifndef quantlib_mc_heston_hull_white_engine_hpp
#define quantlib_mc_heston_hull_white_engine_hpp

#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <ql/processes/jointstochasticprocess.hpp>
#include <ql/pricingengines/vanilla/mcvanillaengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>


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

        MCHestonHullWhiteEngine(Size timeSteps,
                                Size timeStepsPerYear,
                                bool antitheticVariate,
                                bool controlVariate,
                                Size requiredSamples,
                                Real requiredTolerance,
                                Size maxSamples,
                                BigNatural seed);
        

      protected:
        boost::shared_ptr<path_pricer_type> pathPricer() const;    

        boost::shared_ptr<path_pricer_type> controlPathPricer() const;
        boost::shared_ptr<PricingEngine> controlPricingEngine() const;
    };


    class HestonHullWhitePathPricer : public PathPricer<MultiPath> {
      public:
        HestonHullWhitePathPricer(
                    Time exerciseTime,
                    const boost::shared_ptr<Payoff> & payoff,
                    const boost::shared_ptr<JointStochasticProcess> & process);

        Real operator()(const MultiPath& path) const;

      private:
        const Time exerciseTime_;
        const boost::shared_ptr<Payoff> payoff_;
        const boost::shared_ptr<JointStochasticProcess> process_;
    };

    class HestonHullWhiteCVPathPricer : public PathPricer<MultiPath> {
      public:
        HestonHullWhiteCVPathPricer(
                    DiscountFactor discountFactor,
                    const boost::shared_ptr<Payoff> & payoff,
                    const boost::shared_ptr<JointStochasticProcess> & process);

        Real operator()(const MultiPath& path) const;

      private:
        const DiscountFactor df_;
        const boost::shared_ptr<Payoff> payoff_;
        const boost::shared_ptr<JointStochasticProcess> process_;
    };


    template<class RNG,class S>
    inline MCHestonHullWhiteEngine<RNG,S>::MCHestonHullWhiteEngine(
        Size timeSteps,
        Size timeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
        : base_type(timeSteps, timeStepsPerYear,
                    false,antitheticVariate,
                    controlVariate, requiredSamples,
                    requiredTolerance, maxSamples, seed) {
    }


    template <class RNG,class S> inline 
    boost::shared_ptr<typename MCHestonHullWhiteEngine<RNG,S>::path_pricer_type>
    MCHestonHullWhiteEngine<RNG,S>::pathPricer() const {
       boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

       QL_REQUIRE(process, "joint stochastic process required");

       const boost::shared_ptr<Exercise> exercise 
                                    = this->arguments_.exercise;

       QL_REQUIRE(exercise->type() == Exercise::European,
                       "only european exercise is supported");

       const Time exerciseTime = process->time(exercise->lastDate());

       return boost::shared_ptr<path_pricer_type>(
            new HestonHullWhitePathPricer(exerciseTime,
                                          this->arguments_.payoff,
                                          process));
    }

    template <class RNG, class S> inline
    boost::shared_ptr<
        typename MCHestonHullWhiteEngine<RNG,S>::path_pricer_type> 
    MCHestonHullWhiteEngine<RNG,S>::controlPathPricer() const {

        boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

       QL_REQUIRE(process, "joint stochastic process required");

       const boost::shared_ptr<Exercise> exercise = this->arguments_.exercise;

       QL_REQUIRE(exercise->type() == Exercise::European,
                  "only european exercise is supported");

       boost::shared_ptr<HestonProcess> hestonProcess = 
           boost::dynamic_pointer_cast<HestonProcess>(
                                                 process->constituents()[0]);

       QL_REQUIRE(hestonProcess, "first constituent of the joint stochastic " 
                                 "process need to be of type HestonProcess");

       boost::shared_ptr<path_pricer_type> cvPricer(
            new HestonHullWhiteCVPathPricer(
                 hestonProcess->riskFreeRate()->discount(exercise->lastDate()),
                 this->arguments_.payoff,
                 process));

       return cvPricer;
    }

    namespace {
        class AnalyticHestonCVEngine : public PricingEngine {
          public:
            AnalyticHestonCVEngine(
                const boost::shared_ptr<AnalyticHestonEngine> & engine) 
            : engine_(engine) {}

            void reset() { engine_->reset(); }
            const results* getResults() const { return engine_->getResults();}
            arguments* getArguments() const  { return engine_->getArguments();}
            void calculate() const {
                VanillaOption::arguments* args
                    = dynamic_cast<VanillaOption::arguments*>(
                                                    engine_->getArguments());

                boost::shared_ptr<JointStochasticProcess> jointProcess = 
                    boost::dynamic_pointer_cast<JointStochasticProcess>(
                                                      args->stochasticProcess);

                args->stochasticProcess = jointProcess->constituents()[0];
                engine_->calculate();
                args->stochasticProcess = jointProcess;
            }
          private:
            const boost::shared_ptr<PricingEngine> engine_;
        };
    }
    
    template <class RNG, class S> inline
    boost::shared_ptr<PricingEngine> 
    MCHestonHullWhiteEngine<RNG,S>::controlPricingEngine() const {
        boost::shared_ptr<JointStochasticProcess> process =
            boost::dynamic_pointer_cast<JointStochasticProcess>(
                this->arguments_.stochasticProcess);

       QL_REQUIRE(process, "joint stochastic process required");

       boost::shared_ptr<HestonProcess> hestonProcess = 
           boost::dynamic_pointer_cast<HestonProcess>(
                                                 process->constituents()[0]);

       QL_REQUIRE(hestonProcess, "first constituent of the joint stochastic " 
                                 "process need to be of type HestonProcess");
       
       boost::shared_ptr<HestonModel> model(new HestonModel(hestonProcess));
       
       return boost::shared_ptr<PricingEngine>(
           new AnalyticHestonCVEngine(
                 boost::shared_ptr<AnalyticHestonEngine>(
                                    new AnalyticHestonEngine(model, 192))));
    }   
}

#endif
