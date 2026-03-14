/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Banca Profilo S.p.A.
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file mchullwhiteengine.hpp
    \brief Monte Carlo Hull-White engine for cap/floors
*/

#ifndef quantlib_mc_hull_white_cap_floor_engine_hpp
#define quantlib_mc_hull_white_cap_floor_engine_hpp

#include <ql/instruments/capfloor.hpp>
#include <ql/models/shortrate/onefactormodels/hullwhite.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/processes/hullwhiteprocess.hpp>
#include <utility>

namespace QuantLib {

    namespace detail {

        class HullWhiteCapFloorPricer : public PathPricer<Path> {
          public:
            HullWhiteCapFloorPricer(const CapFloor::arguments&,
                                    ext::shared_ptr<HullWhite>,
                                    Time forwardMeasureTime);
            Real operator()(const Path& path) const override;

          private:
            CapFloor::arguments args_;
            ext::shared_ptr<HullWhite> model_;
            Time forwardMeasureTime_;
            DiscountFactor endDiscount_;
            std::vector<Time> startTimes_, endTimes_, fixingTimes_;
        };

    }


    //! Monte Carlo Hull-White engine for cap/floors
    /*! \ingroup capfloorengines */
    template <class RNG = PseudoRandom, class S = Statistics>
    class MCHullWhiteCapFloorEngine
        : public CapFloor::engine,
          public McSimulation<SingleVariate,RNG,S> {
      private:
        typedef McSimulation<SingleVariate,RNG,S> simulation;
        ext::shared_ptr<HullWhite> model_;
        Size requiredSamples_, maxSamples_;
        Real requiredTolerance_;
        bool brownianBridge_;
        BigNatural seed_;
      public:
        typedef typename simulation::path_generator_type path_generator_type;
        typedef typename simulation::path_pricer_type path_pricer_type;
        typedef typename simulation::stats_type stats_type;

        MCHullWhiteCapFloorEngine(ext::shared_ptr<HullWhite> model,
                                  bool brownianBridge,
                                  bool antitheticVariate,
                                  Size requiredSamples,
                                  Real requiredTolerance,
                                  Size maxSamples,
                                  BigNatural seed)
        : McSimulation<SingleVariate, RNG, S>(antitheticVariate, false), model_(std::move(model)),
          requiredSamples_(requiredSamples), maxSamples_(maxSamples),
          requiredTolerance_(requiredTolerance), brownianBridge_(brownianBridge), seed_(seed) {
            registerWith(model_);
        }

        void calculate() const override {
            simulation::calculate(requiredTolerance_,
                                  requiredSamples_,
                                  maxSamples_);
            results_.value = this->mcModel_->sampleAccumulator().mean();
            if constexpr (RNG::allowsErrorEstimate)
                results_.errorEstimate =
                    this->mcModel_->sampleAccumulator().errorEstimate();
        }

      protected:
        ext::shared_ptr<path_pricer_type> pathPricer() const {
            Date referenceDate = model_->termStructure()->referenceDate();
            DayCounter dayCounter = model_->termStructure()->dayCounter();
            Time forwardMeasureTime =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.endDates.back());
            return ext::shared_ptr<path_pricer_type>(
                     new detail::HullWhiteCapFloorPricer(arguments_, model_,
                                                         forwardMeasureTime));
        }

        TimeGrid timeGrid() const {

            Date referenceDate = model_->termStructure()->referenceDate();
            DayCounter dayCounter = model_->termStructure()->dayCounter();

            // only add future fixing times...
            std::vector<Time> times;
            for (Size i=0; i<arguments_.fixingDates.size(); i++) {
                if (arguments_.fixingDates[i] > referenceDate)
                    times.push_back(
                          dayCounter.yearFraction(referenceDate,
                                                  arguments_.fixingDates[i]));
            }
            // ...and maturity.
            times.push_back(
                        dayCounter.yearFraction(referenceDate,
                                                arguments_.endDates.back()));
            return TimeGrid(times.begin(), times.end());
        }

        ext::shared_ptr<path_generator_type> pathGenerator() const {

            Handle<YieldTermStructure> curve = model_->termStructure();
            Date referenceDate = curve->referenceDate();
            DayCounter dayCounter = curve->dayCounter();

            Time forwardMeasureTime =
                dayCounter.yearFraction(referenceDate,
                                        arguments_.endDates.back());
            Array parameters = model_->params();
            Real a = parameters[0], sigma = parameters[1];
            ext::shared_ptr<HullWhiteForwardProcess> process(
                                new HullWhiteForwardProcess(curve, a, sigma));
            process->setForwardMeasureTime(forwardMeasureTime);

            TimeGrid grid = this->timeGrid();
            typename RNG::rsg_type generator =
                RNG::make_sequence_generator(grid.size()-1,seed_);
            return ext::shared_ptr<path_generator_type>(
                             new path_generator_type(process, grid, generator,
                                                     brownianBridge_));
        }
    };



    //! Monte Carlo Hull-White cap-floor engine factory
    template <class RNG = PseudoRandom, class S = Statistics>
    class MakeMCHullWhiteCapFloorEngine {
      public:
        MakeMCHullWhiteCapFloorEngine(ext::shared_ptr<HullWhite>);
        // named parameters
        MakeMCHullWhiteCapFloorEngine& withBrownianBridge(bool b = true);
        MakeMCHullWhiteCapFloorEngine& withSamples(Size samples);
        MakeMCHullWhiteCapFloorEngine& withAbsoluteTolerance(Real tolerance);
        MakeMCHullWhiteCapFloorEngine& withMaxSamples(Size samples);
        MakeMCHullWhiteCapFloorEngine& withSeed(BigNatural seed);
        MakeMCHullWhiteCapFloorEngine& withAntitheticVariate(bool b = true);
        // conversion to pricing engine
        operator ext::shared_ptr<PricingEngine>() const;
      private:
        ext::shared_ptr<HullWhite> model_;
        bool antithetic_ = false;
        Size samples_, maxSamples_;
        Real tolerance_;
        bool brownianBridge_ = false;
        BigNatural seed_ = 0;
    };


    // inline definitions

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG, S>::MakeMCHullWhiteCapFloorEngine(
        ext::shared_ptr<HullWhite> model)
    : model_(std::move(model)), samples_(Null<Size>()), maxSamples_(Null<Size>()),
      tolerance_(Null<Real>()) {}

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>&
    MakeMCHullWhiteCapFloorEngine<RNG,S>::withSamples(Size samples) {
        QL_REQUIRE(tolerance_ == Null<Real>(),
                   "tolerance already set");
        samples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>&
    MakeMCHullWhiteCapFloorEngine<RNG,S>::withAbsoluteTolerance(
                                                             Real tolerance) {
        QL_REQUIRE(samples_ == Null<Size>(),
                   "number of samples already set");
        QL_REQUIRE(RNG::allowsErrorEstimate,
                   "chosen random generator policy "
                   "does not allow an error estimate");
        tolerance_ = tolerance;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>&
    MakeMCHullWhiteCapFloorEngine<RNG,S>::withMaxSamples(Size samples) {
        maxSamples_ = samples;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>&
    MakeMCHullWhiteCapFloorEngine<RNG,S>::withSeed(BigNatural seed) {
        seed_ = seed;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>&
    MakeMCHullWhiteCapFloorEngine<RNG,S>::withBrownianBridge(bool b) {
        brownianBridge_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>&
    MakeMCHullWhiteCapFloorEngine<RNG,S>::withAntitheticVariate(bool b) {
        antithetic_ = b;
        return *this;
    }

    template <class RNG, class S>
    inline MakeMCHullWhiteCapFloorEngine<RNG,S>::
    operator ext::shared_ptr<PricingEngine>() const {
        return ext::shared_ptr<PricingEngine>(new
            MCHullWhiteCapFloorEngine<RNG,S>(model_,
                                             brownianBridge_, antithetic_,
                                             samples_, tolerance_,
                                             maxSamples_, seed_));
    }

}


#endif
