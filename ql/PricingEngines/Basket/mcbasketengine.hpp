
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
    \brief European Basket MC Engine    
*/

#ifndef quantlib_mc_basket_engine_hpp
#define quantlib_mc_basket_engine_hpp

#include <ql/Instruments/basketoption.hpp>
#include <ql/PricingEngines/mcsimulation.hpp>

namespace QuantLib {

    //! MC Pricing engine for European Baskets
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCBasketEngine  : public BasketOption::engine,
                            public McSimulation<MultiAsset<RNG>, S> {
      public:
        MCBasketEngine(Size maxTimeStepsPerYear,
                       bool antitheticVariate = false,
                       bool controlVariate = false,
                       Size requiredSamples = Null<int>(),
                       double requiredTolerance = Null<double>(),
                       Size maxSamples = Null<int>(),
                       bool brownianBridge = false,
                       long seed = 0);

        typedef typename McSimulation<MultiAsset<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MultiAsset<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MultiAsset<RNG>,S>::stats_type
            stats_type;

        void calculate() const;

      protected:
        // McSimulation implementation
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_pricer_type> pathPricer() const;
        // data members
        Size maxTimeStepsPerYear_;
        Size requiredSamples_;
        Size maxSamples_;
        double requiredTolerance_;
        bool brownianBridge_;
        long seed_;
    };

    class EuropeanMultiPathPricer : public PathPricer<MultiPath> {
      public:
        EuropeanMultiPathPricer(
                BasketOption::BasketType basketType,
                Option::Type type,
                double strike,
                Array underlying,
                const RelinkableHandle<TermStructure>& discountTS);
        double operator()(const MultiPath& multiPath) const;
      private:
        BasketOption::BasketType basketType_;
        Array underlying_;
        PlainVanillaPayoff payoff_;
    };


    // template definitions

    template<class RNG, class S>
    inline MCBasketEngine<RNG,S>::MCBasketEngine(Size maxTimeStepsPerYear,
                                                   bool antitheticVariate,
                                                   bool controlVariate,
                                                   Size requiredSamples,
                                                   double requiredTolerance,
                                                   Size maxSamples,
                                                   bool brownianBridge,
                                                   long seed)
    : McSimulation<MultiAsset<RNG>,S>(antitheticVariate, controlVariate),
      maxTimeStepsPerYear_(maxTimeStepsPerYear),
      requiredSamples_(requiredSamples), 
      maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance), 
      brownianBridge_(brownianBridge),
      seed_(seed) {}


    template<class RNG, class S>
    inline 
    boost::shared_ptr<QL_TYPENAME MCBasketEngine<RNG,S>::path_generator_type>
    MCBasketEngine<RNG,S>::pathGenerator() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Size numAssets = arguments_.blackScholesProcesses.size();

        TimeGrid grid = timeGrid();
        typename RNG::rsg_type gen =
            RNG::make_sequence_generator(numAssets*(grid.size()-1),seed_);

        std::vector<boost::shared_ptr<DiffusionProcess> > diffusionProcs;
        for (Size j = 0; j < numAssets; j++) {
            RelinkableHandle<TermStructure> riskFree(
                        arguments_.blackScholesProcesses[j]->riskFreeRate());
            RelinkableHandle<TermStructure> dividend(
                        arguments_.blackScholesProcesses[j]->dividendYield());
            RelinkableHandle<BlackVolTermStructure> vol(
                        arguments_.blackScholesProcesses[j]->volatility());
            boost::shared_ptr<DiffusionProcess> bs(
                   new BlackScholesProcess(riskFree, dividend, vol,
                                           arguments_.blackScholesProcesses[j]
                                           ->stateVariable()->value()));
            diffusionProcs.push_back(bs);
        }

        return boost::shared_ptr<path_generator_type>(new
            path_generator_type(diffusionProcs, arguments_.correlation, 
                                    grid, gen, brownianBridge_));

    }


    template <class RNG, class S>
    inline
    boost::shared_ptr<QL_TYPENAME MCBasketEngine<RNG,S>::path_pricer_type>
    MCBasketEngine<RNG,S>::pathPricer() const {

        boost::shared_ptr<PlainVanillaPayoff> payoff =
            boost::dynamic_pointer_cast<PlainVanillaPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "non-plain payoff given");

        Size numAssets = arguments_.blackScholesProcesses.size();
        Array underlying(numAssets, 0.0);
        for (Size i = 0; i < numAssets; i++) {
            underlying[i] = arguments_.blackScholesProcesses[i]
                ->stateVariable()->value();
        }

        return boost::shared_ptr<MCBasketEngine<RNG,S>::path_pricer_type>(new
            EuropeanMultiPathPricer(
                arguments_.basketType,
                payoff->optionType(),
                payoff->strike(),
                underlying,
                RelinkableHandle<TermStructure>(
                       arguments_.blackScholesProcesses[0]->riskFreeRate())));
    }


    template <class RNG, class S>
    inline TimeGrid MCBasketEngine<RNG,S>::timeGrid() const {

        Time t = arguments_.blackScholesProcesses[0]->riskFreeRate()
            ->dayCounter().yearFraction(
                arguments_.blackScholesProcesses[0]
                    ->riskFreeRate()->referenceDate(),
                arguments_.exercise->lastDate());

        return TimeGrid(t, maxTimeStepsPerYear_);
    }


    template<class RNG, class S>
    inline void MCBasketEngine<RNG,S>::calculate() const {

        QL_REQUIRE(requiredTolerance_ != Null<double>() ||
                   int(requiredSamples_) != Null<int>(),
                   "neither tolerance nor number of samples set");
/*
        QL_REQUIRE(arguments_.exercise->type() == Exercise::European,
                   "not an European Option");
*/
        //! Initialize the multi-factor Monte Carlo
        if (controlVariate_) {

            boost::shared_ptr<path_pricer_type> controlPP = 
                controlPathPricer();
            QL_REQUIRE(controlPP,
                       "engine does not provide "
                       "control variation path pricer");

            boost::shared_ptr<PricingEngine> controlPE = 
                controlPricingEngine();

            QL_REQUIRE(controlPE,
                       "engine does not provide "
                       "control variation pricing engine");

            BasketOption::arguments* controlArguments =
                dynamic_cast<BasketOption::arguments*>(
                    controlPE->arguments());
            *controlArguments = arguments_;
            controlPE->calculate();

            const BasketOption::results* controlResults =
                dynamic_cast<const BasketOption::results*>(
                    controlPE->results());
            double controlVariateValue = controlResults->value;

            mcModel_ =
                boost::shared_ptr<MonteCarloModel<MultiAsset<RNG>, S> >(
                    new MonteCarloModel<MultiAsset<RNG>, S>(
                           pathGenerator(), pathPricer(), stats_type(),
                           antitheticVariate_, controlPP,
                           controlVariateValue));

        } else {
            mcModel_ =
                boost::shared_ptr<MonteCarloModel<MultiAsset<RNG>, S> >(
                    new MonteCarloModel<MultiAsset<RNG>, S>(
                           pathGenerator(), pathPricer(), S(),
                           antitheticVariate_));
        }

        if (requiredTolerance_ != Null<double>()) {
            if (int(maxSamples_) != Null<int>())
                value(requiredTolerance_, maxSamples_);
            else
                value(requiredTolerance_);
        } else {
            valueWithSamples(requiredSamples_);
        }

        results_.value = mcModel_->sampleAccumulator().mean();
        if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                mcModel_->sampleAccumulator().errorEstimate();
    }

}


#endif
