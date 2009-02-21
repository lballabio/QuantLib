/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2007, 2008 StatPro Italia srl

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

/*! \file mcdiscreteasianengine.hpp
    \brief Monte Carlo pricing engine for discrete average Asians
*/

#ifndef quantlib_mcdiscreteasian_engine_hpp
#define quantlib_mcdiscreteasian_engine_hpp

#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/instruments/asianoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for discrete average Asians using Monte Carlo simulation
    /*! \warning control-variate calculation is disabled under VC++6.

        \ingroup asianengines
    */
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteAveragingAsianEngine :
                                public DiscreteAveragingAsianOption::engine,
                                public McSimulation<SingleVariate,RNG,S> {
      public:
        typedef
        typename McSimulation<SingleVariate,RNG,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleVariate,RNG,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteAveragingAsianEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             bool controlVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed);
        void calculate() const {
            McSimulation<SingleVariate,RNG,S>::calculate(requiredTolerance_,
                                                         requiredSamples_,
                                                         maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            
            if (this->controlVariate_) {
                // control variate might lead to small negative
                // option values for deep OTM options
                this->results_.value = std::max(0.0, this->results_.value);
            }
                
            if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
      protected:
        // McSimulation implementation
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_generator_type> pathGenerator() const {

            TimeGrid grid = this->timeGrid();
            typename RNG::rsg_type gen =
                RNG::make_sequence_generator(grid.size()-1,seed_);
            return boost::shared_ptr<path_generator_type>(
                         new path_generator_type(process_, grid,
                                                 gen, brownianBridge_));
        }
        Real controlVariateValue() const;
        // data members
        boost::shared_ptr<GeneralizedBlackScholesProcess> process_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    // template definitions

    template<class RNG, class S>
    inline
    MCDiscreteAveragingAsianEngine<RNG,S>::MCDiscreteAveragingAsianEngine(
             const boost::shared_ptr<GeneralizedBlackScholesProcess>& process,
             bool brownianBridge,
             bool antitheticVariate,
             bool controlVariate,
             Size requiredSamples,
             Real requiredTolerance,
             Size maxSamples,
             BigNatural seed)
    : McSimulation<SingleVariate,RNG,S>(antitheticVariate, controlVariate),
      process_(process), requiredSamples_(requiredSamples),
      maxSamples_(maxSamples), requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {
        registerWith(process_);
    }

    template <class RNG, class S>
    inline TimeGrid MCDiscreteAveragingAsianEngine<RNG,S>::timeGrid() const {

        Date referenceDate = process_->riskFreeRate()->referenceDate();
        DayCounter voldc = process_->blackVolatility()->dayCounter();
        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            if (arguments_.fixingDates[i]>=referenceDate) {
                Time t = voldc.yearFraction(referenceDate,
                    arguments_.fixingDates[i]);
                fixingTimes.push_back(t);
            }
        }

        return TimeGrid(fixingTimes.begin(), fixingTimes.end());
    }

    template<class RNG, class S>
    inline
    Real MCDiscreteAveragingAsianEngine<RNG,S>::controlVariateValue() const {

        boost::shared_ptr<PricingEngine> controlPE =
                this->controlPricingEngine();
            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control variation pricing engine");

            DiscreteAveragingAsianOption::arguments* controlArguments =
                dynamic_cast<DiscreteAveragingAsianOption::arguments*>(
                    controlPE->getArguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const DiscreteAveragingAsianOption::results* controlResults =
                dynamic_cast<const DiscreteAveragingAsianOption::results*>(
                    controlPE->getResults());

            return controlResults->value;
    }

}


#endif
