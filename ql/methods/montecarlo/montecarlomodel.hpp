/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl
 Copyright (C) 2007 StatPro Italia srl

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

/*! \file montecarlomodel.hpp
    \brief General-purpose Monte Carlo model
*/

#ifndef quantlib_montecarlo_model_hpp
#define quantlib_montecarlo_model_hpp

#include <ql/math/statistics/statistics.hpp>
#include <ql/methods/montecarlo/mctraits.hpp>
#include <memory>
#include <utility>

namespace QuantLib {

    //! General-purpose Monte Carlo model for path samples
    /*! The template arguments of this class correspond to available
        policies for the particular model to be instantiated---i.e.,
        whether it is single- or multi-asset, or whether it should use
        pseudo-random or low-discrepancy numbers for path
        generation. Such decisions are grouped in trait classes so as
        to be orthogonal---see mctraits.hpp for examples.

        The constructor accepts two safe references, i.e. two smart
        pointers, one to a path generator and the other to a path
        pricer.  In case of control variate technique the user should
        provide the additional control option, namely the option path
        pricer and the option value.

        \ingroup mcarlo
    */
    template <template <class> class MC, class RNG, class S = Statistics>
    class MonteCarloModel {
      public:
        typedef MC<RNG> mc_traits;
        typedef RNG rng_traits;
        typedef typename MC<RNG>::path_generator_type path_generator_type;
        typedef typename MC<RNG>::path_pricer_type path_pricer_type;
        typedef typename path_generator_type::sample_type sample_type;
        typedef typename path_pricer_type::result_type result_type;
        typedef S stats_type;
        // constructor
        MonteCarloModel(
            std::shared_ptr<path_generator_type> pathGenerator,
            std::shared_ptr<path_pricer_type> pathPricer,
            stats_type sampleAccumulator,
            bool antitheticVariate,
            std::shared_ptr<path_pricer_type> cvPathPricer = std::shared_ptr<path_pricer_type>(),
            result_type cvOptionValue = result_type(),
            std::shared_ptr<path_generator_type> cvPathGenerator =
                std::shared_ptr<path_generator_type>())
        : pathGenerator_(std::move(pathGenerator)), pathPricer_(std::move(pathPricer)),
          sampleAccumulator_(std::move(sampleAccumulator)), isAntitheticVariate_(antitheticVariate),
          cvPathPricer_(std::move(cvPathPricer)), cvOptionValue_(cvOptionValue),
          cvPathGenerator_(std::move(cvPathGenerator)) {
            isControlVariate_ = static_cast<bool>(cvPathPricer_);
        }
        void addSamples(Size samples);
        const stats_type& sampleAccumulator() const;
      private:
        std::shared_ptr<path_generator_type> pathGenerator_;
        std::shared_ptr<path_pricer_type> pathPricer_;
        stats_type sampleAccumulator_;
        bool isAntitheticVariate_;
        std::shared_ptr<path_pricer_type> cvPathPricer_;
        result_type cvOptionValue_;
        bool isControlVariate_;
        std::shared_ptr<path_generator_type> cvPathGenerator_;
    };

    // inline definitions
    template <template <class> class MC, class RNG, class S>
    inline void MonteCarloModel<MC,RNG,S>::addSamples(Size samples) {
        for(Size j = 1; j <= samples; j++) {

            const sample_type& path = pathGenerator_->next();
            result_type price = (*pathPricer_)(path.value);

            if (isControlVariate_) {
                if (!cvPathGenerator_) {
                    price += cvOptionValue_-(*cvPathPricer_)(path.value);
                }
                else {
                    const sample_type& cvPath = cvPathGenerator_->next();
                    price += cvOptionValue_-(*cvPathPricer_)(cvPath.value);
                }
            }

            if (isAntitheticVariate_) {
                const sample_type& atPath = pathGenerator_->antithetic();
                result_type price2 = (*pathPricer_)(atPath.value);
                if (isControlVariate_) {
                    if (!cvPathGenerator_)
                        price2 += cvOptionValue_-(*cvPathPricer_)(atPath.value);
                    else {
                        const sample_type& cvPath = cvPathGenerator_->antithetic();
                        price2 += cvOptionValue_-(*cvPathPricer_)(cvPath.value);
                    }
                }

                sampleAccumulator_.add((price+price2)/2.0, path.weight);
            } else {
                sampleAccumulator_.add(price, path.weight);
            }
        }
    }

    template <template <class> class MC, class RNG, class S>
    inline const typename MonteCarloModel<MC,RNG,S>::stats_type&
    MonteCarloModel<MC,RNG,S>::sampleAccumulator() const {
        return sampleAccumulator_;
    }

}


#endif
