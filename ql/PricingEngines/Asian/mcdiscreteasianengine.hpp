
/*
 Copyright (C) 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file mcdiscreteasianengine.hpp
    \brief Monte Carlo pricing engine for discrete average Asians
*/

#ifndef quantlib_mcdiscreteasian_engine_h
#define quantlib_mcdiscreteasian_engine_h

#include <ql/PricingEngines/mcsimulation.hpp>
#include <ql/Instruments/asianoption.hpp>

namespace QuantLib {

    //! Monte Carlo pricing engine for discrete average Asians
    /*! \ingroup asianengines */
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteAveragingAsianEngine :
                                public DiscreteAveragingAsianOption::engine,
                                public McSimulation<SingleAsset<RNG>, S> {
      public:
        void calculate() const;
      protected:
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteAveragingAsianEngine(Size maxTimeStepsPerYear,
                      bool antitheticVariate = false,
                      bool controlVariate = false,
                      Size requiredSamples = Null<Size>(),
                      Real requiredTolerance = Null<Real>(),
                      Size maxSamples = Null<Size>(),
                      BigNatural seed = 0);
        // McSimulation implementation
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        TimeGrid timeGrid() const;
        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        BigNatural seed_;
    };


    // inline definitions

    template<class RNG, class S>
    inline
    MCDiscreteAveragingAsianEngine<RNG,S>::MCDiscreteAveragingAsianEngine(
        Size maxTimeStepsPerYear,
        bool antitheticVariate,
        bool controlVariate,
        Size requiredSamples,
        Real requiredTolerance,
        Size maxSamples,
        BigNatural seed)
    : McSimulation<SingleAsset<RNG>,S>(antitheticVariate, controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance), seed_(seed) {}

    // template definitions

    template<class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCDiscreteAveragingAsianEngine<RNG,S>::path_generator_type>
    MCDiscreteAveragingAsianEngine<RNG,S>::pathGenerator() const {

        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1,seed_);
        // BB here
        return boost::shared_ptr<path_generator_type>(
                      new path_generator_type(arguments_.blackScholesProcess,
                                              grid, gen, true));
    }


    template<class RNG, class S>
    inline void MCDiscreteAveragingAsianEngine<RNG,S>::calculate() const {

        QL_REQUIRE(requiredTolerance_ != Null<Real>() ||
                   requiredSamples_ != Null<Size>(),
                   "neither tolerance nor number of samples set");

        //! Initialize the one-factor Monte Carlo
        if (this->controlVariate_) {

            boost::shared_ptr<path_pricer_type> controlPP =
                this->controlPathPricer();
            QL_REQUIRE(controlPP,
                       "engine does not provide "
                       "control variation path pricer");

            boost::shared_ptr<PricingEngine> controlPE =
                this->controlPricingEngine();
            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control variation pricing engine");

            DiscreteAveragingAsianOption::arguments* controlArguments =
                dynamic_cast<DiscreteAveragingAsianOption::arguments*>(
                    controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const DiscreteAveragingAsianOption::results* controlResults =
                dynamic_cast<const DiscreteAveragingAsianOption::results*>(
                    controlPE->results());
            Real controlVariateValue = controlResults->value;

            this->mcModel_ =
                boost::shared_ptr<MonteCarloModel<SingleAsset<RNG>, S> >(
                    new MonteCarloModel<SingleAsset<RNG>, S>(
                           pathGenerator(), this->pathPricer(), stats_type(),
                           this->antitheticVariate_, controlPP,
                           controlVariateValue));

        } else {
            this->mcModel_ =
                boost::shared_ptr<MonteCarloModel<SingleAsset<RNG>, S> >(
                    new MonteCarloModel<SingleAsset<RNG>, S>(
                           pathGenerator(), this->pathPricer(), S(),
                           this->antitheticVariate_));
        }

        if (requiredTolerance_ != Null<Real>()) {
            if (maxSamples_ != Null<Size>())
                this->value(requiredTolerance_, maxSamples_);
            else
                this->value(requiredTolerance_);
        } else {
            this->valueWithSamples(requiredSamples_);
        }

        results_.value = this->mcModel_->sampleAccumulator().mean();
        if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
    }

    template <class RNG, class S>
    inline
    TimeGrid MCDiscreteAveragingAsianEngine<RNG,S>::timeGrid() const {

        boost::shared_ptr<BlackScholesProcess> process =
            arguments_.blackScholesProcess;
        Date referenceDate = process->riskFreeRate()->referenceDate();
        DayCounter dc = process->blackVolatility()->dayCounter();
        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            if (arguments_.fixingDates[i]>=referenceDate) {
                Time t = dc.yearFraction(referenceDate,
                    arguments_.fixingDates[i]);
                fixingTimes.push_back(t);
            }
        }

        // handle here maxStepsPerYear
        return TimeGrid(fixingTimes.begin(), fixingTimes.end());
    }


}


#endif
