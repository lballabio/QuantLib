
/*
 Copyright (C) 2004 Neil Firth

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

/*! \file mcbasketengine.hpp
    \brief European basket MC Engine
*/

#ifndef quantlib_mc_basket_engine_hpp
#define quantlib_mc_basket_engine_hpp

#include <ql/Instruments/basketoption.hpp>
#include <ql/PricingEngines/mcsimulation.hpp>
#include <ql/Processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for basket options using Monte Carlo simulation
    /*! \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCBasketEngine  : public BasketOption::engine,
                            public McSimulation<MultiAsset<RNG>, S> {
      public:
        typedef typename McSimulation<MultiAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MultiAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MultiAsset<RNG>,S>::stats_type
            stats_type;
        // constructor
        MCBasketEngine(Size maxTimeStepsPerYear,
                       bool brownianBridge,
                       bool antitheticVariate,
                       bool controlVariate,
                       Size requiredSamples,
                       Real requiredTolerance,
                       Size maxSamples,
                       BigNatural seed);
        void calculate() const {
            McSimulation<MultiAsset<RNG>,S>::calculate(requiredTolerance_,
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
        boost::shared_ptr<path_pricer_type> pathPricer() const;
        // Real controlVariateValue() const;
        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_;
        Size maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };

    class EuropeanMultiPathPricer : public PathPricer<MultiPath> {
      public:
        EuropeanMultiPathPricer(BasketOption::BasketType basketType,
                                Option::Type type,
                                Real strike,
                                Array underlying,
                                DiscountFactor discount);
        Real operator()(const MultiPath& multiPath) const;
      private:
        BasketOption::BasketType basketType_;
        Array underlying_;
        PlainVanillaPayoff payoff_;
        DiscountFactor discount_;
    };


    // template definitions

    template<class RNG, class S>
    inline MCBasketEngine<RNG,S>::MCBasketEngine(Size maxTimeStepsPerYear,
                                                 bool brownianBridge,
                                                 bool antitheticVariate,
                                                 bool controlVariate,
                                                 Size requiredSamples,
                                                 Real requiredTolerance,
                                                 Size maxSamples,
                                                 BigNatural seed)
    : McSimulation<MultiAsset<RNG>,S>(antitheticVariate, controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance),
      brownianBridge_(brownianBridge), seed_(seed) {}

    template <class RNG, class S>
    inline TimeGrid MCBasketEngine<RNG,S>::timeGrid() const {

        Time residualTime = this->arguments_.stochasticProcesses[0]->time(
                                       this->arguments_.exercise->lastDate());

        return TimeGrid(residualTime, maxTimeStepsPerYear_);
    }

    template<class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCBasketEngine<RNG,S>::path_generator_type>
    MCBasketEngine<RNG,S>::pathGenerator() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Size numAssets = arguments_.stochasticProcesses.size();

        TimeGrid grid = timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(numAssets*(grid.size()-1),seed_);

        return boost::shared_ptr<path_generator_type>(
                       new path_generator_type(arguments_.stochasticProcesses,
                                               arguments_.correlation,
                                               grid, gen, brownianBridge_));
    }

    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCBasketEngine<RNG,S>::path_pricer_type>
    MCBasketEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Size numAssets = arguments_.stochasticProcesses.size();
        Array underlying(numAssets, 0.0);
        for (Size i = 0; i < numAssets; i++) {
            underlying[i] = arguments_.stochasticProcesses[i]->x0();
        }

        boost::shared_ptr<BlackScholesProcess> process =
            boost::dynamic_pointer_cast<BlackScholesProcess>(
                                           arguments_.stochasticProcesses[0]);
        QL_REQUIRE(process, "Black-Scholes process required");

        return boost::shared_ptr<
                         QL_TYPENAME MCBasketEngine<RNG,S>::path_pricer_type>(
            new EuropeanMultiPathPricer(
                arguments_.basketType,
                payoff->optionType(),
                payoff->strike(),
                underlying,
                process->riskFreeRate()->discount(
                                           arguments_.exercise->lastDate())));
    }

    /*
    template<class RNG, class S>
    inline Real MCBasketEngine<RNG,S>::controlVariateValue() const {

        boost::shared_ptr<PricingEngine> controlPE =
            this->controlPricingEngine();

        QL_REQUIRE(controlPE,
                    "engine does not provide "
                    "control-variation pricing engine");

        BasketOption::arguments* controlArguments =
            dynamic_cast<BasketOption::arguments*>(
                controlPE->arguments());
        *controlArguments = arguments_;
        controlPE->calculate();

        const BasketOption::results* controlResults =
            dynamic_cast<const BasketOption::results*>(
                controlPE->results());

        return controlResults->value;
    }
    */

}


#endif
