/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2007 StatPro Italia srl
 Copyright (C) 2015, 2016 Peter Caspers
 Copyright (C) 2015 Thema Consulting SA

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

/*! \file mclongstaffschwartzengine.hpp
    \brief Longstaff Schwartz Monte Carlo engine for early exercise options
*/

#ifndef quantlib_mc_longstaff_schwartz_engine_hpp
#define quantlib_mc_longstaff_schwartz_engine_hpp

#include <ql/exercise.hpp>
#include <ql/pricingengines/mcsimulation.hpp>
#include <ql/methods/montecarlo/longstaffschwartzpathpricer.hpp>
#include <ql/optional.hpp>

namespace QuantLib {

    //! Longstaff-Schwarz Monte Carlo engine for early exercise options
    /*! References:

        Francis Longstaff, Eduardo Schwartz, 2001. Valuing American Options
        by Simulation: A Simple Least-Squares Approach, The Review of
        Financial Studies, Volume 14, No. 1, 113-147

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
    */
    template <class GenericEngine, template <class> class MC,
              class RNG, class S = Statistics, class RNG_Calibration = RNG>
    class MCLongstaffSchwartzEngine : public GenericEngine,
                                      public McSimulation<MC,RNG,S> {
      public:
        typedef typename MC<RNG>::path_type path_type;
        typedef typename McSimulation<MC,RNG,S>::stats_type
            stats_type;
        typedef typename McSimulation<MC,RNG,S>::path_pricer_type
            path_pricer_type;
        typedef typename McSimulation<MC,RNG,S>::path_generator_type
            path_generator_type;
        typedef
            typename McSimulation<MC, RNG_Calibration, S>::path_generator_type
                path_generator_type_calibration;

        /*! If the parameters brownianBridge and antitheticVariate are
          not given they are chosen to be identical to the respective
          parameters for pricing; the seed for calibration is chosen
          to be zero if the pricing seed is zero and otherwise as the
          pricing seed plus some offset to avoid identical paths in
          calibration and pricing; note however that this has no effect
          for low discrepancy RNGs usually, it is therefore recommended
          to use pseudo random generators for the calibration phase always
          (and possibly quasi monte carlo in the subsequent pricing). */
        MCLongstaffSchwartzEngine(ext::shared_ptr<StochasticProcess> process,
                                  Size timeSteps,
                                  Size timeStepsPerYear,
                                  bool brownianBridge,
                                  bool antitheticVariate,
                                  bool controlVariate,
                                  Size requiredSamples,
                                  Real requiredTolerance,
                                  Size maxSamples,
                                  BigNatural seed,
                                  Size nCalibrationSamples = Null<Size>(),
                                  ext::optional<bool> brownianBridgeCalibration = ext::nullopt,
                                  ext::optional<bool> antitheticVariateCalibration = ext::nullopt,
                                  BigNatural seedCalibration = Null<Size>());

        void calculate() const override;

      protected:
        virtual ext::shared_ptr<LongstaffSchwartzPathPricer<path_type> >
                                                   lsmPathPricer() const = 0;

        TimeGrid timeGrid() const override;
        ext::shared_ptr<path_pricer_type> pathPricer() const override;
        ext::shared_ptr<path_generator_type> pathGenerator() const override;

        ext::shared_ptr<StochasticProcess> process_;
        const Size timeSteps_;
        const Size timeStepsPerYear_;
        const bool brownianBridge_;
        const Size requiredSamples_;
        const Real requiredTolerance_;
        const Size maxSamples_;
        const BigNatural seed_;
        const Size nCalibrationSamples_;
        const bool brownianBridgeCalibration_;
        const bool antitheticVariateCalibration_;
        const BigNatural seedCalibration_;

        mutable ext::shared_ptr<LongstaffSchwartzPathPricer<path_type> >
            pathPricer_;
        mutable ext::shared_ptr<MonteCarloModel<MC, RNG_Calibration, S> >
            mcModelCalibration_;
    };

    template <class GenericEngine,
              template <class>
              class MC,
              class RNG,
              class S,
              class RNG_Calibration>
    inline MCLongstaffSchwartzEngine<GenericEngine, MC, RNG, S, RNG_Calibration>::
        MCLongstaffSchwartzEngine(ext::shared_ptr<StochasticProcess> process,
                                  Size timeSteps,
                                  Size timeStepsPerYear,
                                  bool brownianBridge,
                                  bool antitheticVariate,
                                  bool controlVariate,
                                  Size requiredSamples,
                                  Real requiredTolerance,
                                  Size maxSamples,
                                  BigNatural seed,
                                  Size nCalibrationSamples,
                                  ext::optional<bool> brownianBridgeCalibration,
                                  ext::optional<bool> antitheticVariateCalibration,
                                  BigNatural seedCalibration)
    : McSimulation<MC, RNG, S>(antitheticVariate, controlVariate), process_(std::move(process)),
      timeSteps_(timeSteps), timeStepsPerYear_(timeStepsPerYear), brownianBridge_(brownianBridge),
      requiredSamples_(requiredSamples), requiredTolerance_(requiredTolerance),
      maxSamples_(maxSamples), seed_(seed),
      nCalibrationSamples_((nCalibrationSamples == Null<Size>()) ? 2048 : nCalibrationSamples),
      // NOLINTNEXTLINE(readability-implicit-bool-conversion)
      brownianBridgeCalibration_(brownianBridgeCalibration ? *brownianBridgeCalibration :
                                                             brownianBridge),
      antitheticVariateCalibration_(
          // NOLINTNEXTLINE(readability-implicit-bool-conversion)
          antitheticVariateCalibration ? *antitheticVariateCalibration : antitheticVariate),
      seedCalibration_(seedCalibration != Null<Real>() ? seedCalibration :
                                                         (seed == 0 ? 0 : seed + 1768237423L)) {
        QL_REQUIRE(timeSteps != Null<Size>() ||
                   timeStepsPerYear != Null<Size>(),
                   "no time steps provided");
        QL_REQUIRE(timeSteps == Null<Size>() ||
                   timeStepsPerYear == Null<Size>(),
                   "both time steps and time steps per year were provided");
        QL_REQUIRE(timeSteps != 0,
                   "timeSteps must be positive, " << timeSteps <<
                   " not allowed");
        QL_REQUIRE(timeStepsPerYear != 0,
                   "timeStepsPerYear must be positive, " << timeStepsPerYear <<
                   " not allowed");
        this->registerWith(process_);
    }

    template <class GenericEngine, template <class> class MC, class RNG,
              class S, class RNG_Calibration>
    inline ext::shared_ptr<typename MCLongstaffSchwartzEngine<
        GenericEngine, MC, RNG, S, RNG_Calibration>::path_pricer_type>
    MCLongstaffSchwartzEngine<GenericEngine, MC, RNG, S,
                              RNG_Calibration>::pathPricer() const {

        QL_REQUIRE(pathPricer_, "path pricer unknown");
        return pathPricer_;
    }

    template <class GenericEngine, template <class> class MC, class RNG,
              class S, class RNG_Calibration>
    inline void MCLongstaffSchwartzEngine<GenericEngine, MC, RNG, S,
                                          RNG_Calibration>::calculate() const {
        // calibration
        pathPricer_ = this->lsmPathPricer();
        Size dimensions = process_->factors();
        TimeGrid grid = this->timeGrid();
        typename RNG_Calibration::rsg_type generator =
            RNG_Calibration::make_sequence_generator(
                dimensions * (grid.size() - 1), seedCalibration_);
        ext::shared_ptr<path_generator_type_calibration>
            pathGeneratorCalibration =
                ext::make_shared<path_generator_type_calibration>(
                    process_, grid, generator, brownianBridgeCalibration_);
        mcModelCalibration_ =
            ext::shared_ptr<MonteCarloModel<MC, RNG_Calibration, S> >(
                new MonteCarloModel<MC, RNG_Calibration, S>(
                    pathGeneratorCalibration, pathPricer_, stats_type(),
                    this->antitheticVariateCalibration_));

        mcModelCalibration_->addSamples(nCalibrationSamples_);
        pathPricer_->calibrate();
        // pricing
        McSimulation<MC,RNG,S>::calculate(requiredTolerance_,
                                          requiredSamples_,
                                          maxSamples_);
        this->results_.value = this->mcModel_->sampleAccumulator().mean();
        this->results_.additionalResults["exerciseProbability"] =
            this->pathPricer_->exerciseProbability();
        if constexpr (RNG::allowsErrorEstimate) {
            this->results_.errorEstimate =
                this->mcModel_->sampleAccumulator().errorEstimate();
        }
    }

    template <class GenericEngine, template <class> class MC, class RNG,
              class S, class RNG_Calibration>
    inline TimeGrid
    MCLongstaffSchwartzEngine<GenericEngine, MC, RNG, S,
                              RNG_Calibration>::timeGrid() const {
        std::vector<Time> requiredTimes;
        if (this->arguments_.exercise->type() == Exercise::American) {
            Date lastExerciseDate = this->arguments_.exercise->lastDate();
            requiredTimes.push_back(process_->time(lastExerciseDate));
        } else {
            for (Size i = 0; i < this->arguments_.exercise->dates().size();
                 ++i) {
                Time t = process_->time(this->arguments_.exercise->date(i));
                if (t > 0.0)
                    requiredTimes.push_back(t);
            }
        }
        if (this->timeSteps_ != Null<Size>()) {
            return TimeGrid(requiredTimes.begin(), requiredTimes.end(),
                            this->timeSteps_);
        } else if (this->timeStepsPerYear_ != Null<Size>()) {
            Size steps = static_cast<Size>(this->timeStepsPerYear_ *
                                           requiredTimes.back());
            return TimeGrid(requiredTimes.begin(), requiredTimes.end(),
                            std::max<Size>(steps, 1));
        } else {
            QL_FAIL("time steps not specified");
        }
    }

    template <class GenericEngine, template <class> class MC, class RNG,
              class S, class RNG_Calibration>
    inline ext::shared_ptr<typename MCLongstaffSchwartzEngine<
        GenericEngine, MC, RNG, S, RNG_Calibration>::path_generator_type>
    MCLongstaffSchwartzEngine<GenericEngine, MC, RNG, S,
                              RNG_Calibration>::pathGenerator() const {

        Size dimensions = process_->factors();
        TimeGrid grid = this->timeGrid();
        typename RNG::rsg_type generator =
            RNG::make_sequence_generator(dimensions*(grid.size()-1),seed_);
        return ext::shared_ptr<path_generator_type>(
                   new path_generator_type(process_,
                                           grid, generator, brownianBridge_));
    }

}


#endif
