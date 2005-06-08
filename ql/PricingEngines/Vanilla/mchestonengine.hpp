/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Klaus Spanderen

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

/*! \file mchestonengine.hpp
    \brief Monte Carlo Heston-model engine
*/

#ifndef quantlib_mc_heston_engine_hpp
#define quantlib_mc_heston_engine_hpp

#include <ql/PricingEngines/mcsimulation.hpp>
#include <ql/Instruments/vanillaoption.hpp>
#include <ql/Processes/hestonprocess.hpp>

namespace QuantLib {

    //! Monte Carlo Heston-model engine
    /*! \ingroup vanillaengines */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCHestonEngine : public VanillaOption::engine,
                           public McSimulation<MultiVariate<RNG>, S> {
      public:
        void calculate() const {
            McSimulation<MultiVariate<RNG>,S>::calculate(requiredTolerance_,
                                                         requiredSamples_,
                                                         maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            if (RNG::allowsErrorEstimate)
            results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
      protected:
        typedef typename McSimulation<MultiVariate<RNG>,S>::path_generator_type
            path_generator_type;
        typedef typename McSimulation<MultiVariate<RNG>,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MultiVariate<RNG>,S>::stats_type
            stats_type;
        // constructor
        MCHestonEngine(Size timeSteps,
                       Size timeStepsPerYear,
                       bool antitheticVariate,
                       Size requiredSamples,
                       Real requiredTolerance,
                       Size maxSamples,
                       BigNatural seed);
        // McSimulation implementation
        TimeGrid timeGrid() const;
        boost::shared_ptr<path_generator_type> pathGenerator() const;
        // data members
        Size timeSteps_, timeStepsPerYear_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
    };


    // template definitions

    template <class RNG, class S>
    MCHestonEngine<RNG, S>::MCHestonEngine(Size timeSteps,
                                           Size timeStepsPerYear,
                                           bool antitheticVariate,
                                           Size requiredSamples,
                                           Real requiredTolerance,
                                           Size maxSamples,
                                           BigNatural seed)
    : McSimulation<MultiVariate<RNG>,S>(antitheticVariate, false),
      timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear),
      requiredSamples_(requiredSamples), maxSamples_(maxSamples),
      requiredTolerance_(requiredTolerance), seed_(seed) {}


    template <class RNG, class S>
    inline TimeGrid MCHestonEngine<RNG,S>::timeGrid() const {
        Date lastExerciseDate = this->arguments_.exercise->lastDate();
        Time t = this->arguments_.stochasticProcess->time(lastExerciseDate);
        if (this->timeSteps_ != Null<Size>()) {
            return TimeGrid(t, this->timeSteps_);
        } else if (this->timeStepsPerYear_ != Null<Size>()) {
            Size steps = static_cast<Size>(this->timeStepsPerYear_*t);
            return TimeGrid(t, std::max<Size>(steps, 1));
        } else {
            QL_FAIL("time steps not specified");
        }
    }


    template<class RNG, class S>
    boost::shared_ptr<QL_TYPENAME MCHestonEngine<RNG, S>::path_generator_type>
    MCHestonEngine<RNG, S>::pathGenerator() const {

        boost::shared_ptr<HestonProcess> process =
            boost::dynamic_pointer_cast<HestonProcess>(
                                                arguments_.stochasticProcess);
        QL_REQUIRE(process, "Heston process required");

        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type generator =
            RNG::make_sequence_generator(2*(grid.size()-1),seed_);

        return boost::shared_ptr<path_generator_type>(
                           new path_generator_type(process, grid, generator));
    }

}


#endif
