
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

    //! Pricing engine for discrete average Asians using Monte Carlo simulation
    /*! \ingroup asianengines */
    template<class RNG = PseudoRandom, class S = Statistics>
    class MCDiscreteAveragingAsianEngine :
                                public DiscreteAveragingAsianOption::engine,
                                public McSimulation<SingleAsset<RNG>, S> {
      public:
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<SingleAsset<RNG>,S>::stats_type
            stats_type;
        // constructor
        MCDiscreteAveragingAsianEngine(Size maxTimeStepsPerYear,
                                       bool brownianBridge,
                                       bool antitheticVariate,
                                       bool controlVariate,
                                       Size requiredSamples,
                                       Real requiredTolerance,
                                       Size maxSamples,
                                       BigNatural seed);
        void calculate() const {
            McSimulation<SingleAsset<RNG>,S>::calculate(requiredTolerance_,
                                                        requiredSamples_,
                                                        maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
      protected:
        // McSimulation implementation
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        Real controlVariateValue() const;
        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    // template definitions

    template<class RNG, class S>
    inline
    MCDiscreteAveragingAsianEngine<RNG,S>::MCDiscreteAveragingAsianEngine(
                                                    Size maxTimeStepsPerYear,
                                                    bool brownianBridge,
                                                    bool antitheticVariate,
                                                    bool controlVariate,
                                                    Size requiredSamples,
                                                    Real requiredTolerance,
                                                    Size maxSamples,
                                                    BigNatural seed)
    : McSimulation<SingleAsset<RNG>,S>(antitheticVariate, controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {}

    template <class RNG, class S>
    inline TimeGrid MCDiscreteAveragingAsianEngine<RNG,S>::timeGrid() const {

        boost::shared_ptr<BlackScholesProcess> process =
            arguments_.blackScholesProcess;
        Date referenceDate = process->riskFreeRate()->referenceDate();
        DayCounter voldc = process->blackVolatility()->dayCounter();
        std::vector<Time> fixingTimes;
        Size i;
        for (i=0; i<arguments_.fixingDates.size(); i++) {
            if (arguments_.fixingDates[i]>=referenceDate) {
                Time t = voldc.yearFraction(referenceDate,
                    arguments_.fixingDates[i]);
                fixingTimes.push_back(t);
            }
        }

        // handle here maxStepsPerYear
        return TimeGrid(fixingTimes.begin(), fixingTimes.end());
    }

    template<class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCDiscreteAveragingAsianEngine<RNG,S>::path_generator_type>
    MCDiscreteAveragingAsianEngine<RNG,S>::pathGenerator() const {

        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(grid.size()-1,seed_);
        return boost::shared_ptr<path_generator_type>(new
            path_generator_type(arguments_.blackScholesProcess,
                                grid, gen, brownianBridge_));
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
                    controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const DiscreteAveragingAsianOption::results* controlResults =
                dynamic_cast<const DiscreteAveragingAsianOption::results*>(
                    controlPE->results());

            return controlResults->value;
    }

}


#endif
