/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2015 Peter Caspers
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

/*! \file longstaffschwartzpathpricer.hpp
    \brief Longstaff-Schwarz path pricer for early exercise options
*/

#ifndef quantlib_longstaff_schwartz_path_pricer_hpp
#define quantlib_longstaff_schwartz_path_pricer_hpp

#include <ql/functional.hpp>
#include <ql/math/generallinearleastsquares.hpp>
#include <ql/math/statistics/incrementalstatistics.hpp>
#include <ql/methods/montecarlo/earlyexercisepathpricer.hpp>
#include <ql/methods/montecarlo/pathpricer.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <utility>
#include <memory>

namespace QuantLib {

    //! Longstaff-Schwarz path pricer for early exercise options
    /*! References:

        Francis Longstaff, Eduardo Schwartz, 2001. Valuing American Options
        by Simulation: A Simple Least-Squares Approach, The Review of
        Financial Studies, Volume 14, No. 1, 113-147

        \ingroup mcarlo

        \test the correctness of the returned value is tested by
              reproducing results available in web/literature
    */
    template <class PathType>
    class LongstaffSchwartzPathPricer : public PathPricer<PathType> {
      public:
        typedef typename EarlyExerciseTraits<PathType>::StateType StateType;

        LongstaffSchwartzPathPricer(const TimeGrid& times,
                                    ext::shared_ptr<EarlyExercisePathPricer<PathType> >,
                                    const ext::shared_ptr<YieldTermStructure>& termStructure);

        Real operator()(const PathType& path) const override;
        virtual void calibrate();

        Real exerciseProbability() const;

      protected:
        virtual void post_processing(const Size i,
                                     const std::vector<StateType> &state,
                                     const std::vector<Real> &price,
                                     const std::vector<Real> &exercise) {}
        bool  calibrationPhase_;
        const ext::shared_ptr<EarlyExercisePathPricer<PathType> >
            pathPricer_;

        mutable QuantLib::IncrementalStatistics exerciseProbability_;

        std::unique_ptr<Array[]> coeff_;
        std::unique_ptr<DiscountFactor[]> dF_;

        mutable std::vector<PathType> paths_;
        const   std::vector<ext::function<Real(StateType)> > v_;

        const Size len_;
    };

    template <class PathType>
    inline LongstaffSchwartzPathPricer<PathType>::LongstaffSchwartzPathPricer(
        const TimeGrid& times,
        ext::shared_ptr<EarlyExercisePathPricer<PathType> > pathPricer,
        const ext::shared_ptr<YieldTermStructure>& termStructure)
    : calibrationPhase_(true), pathPricer_(std::move(pathPricer)),
      coeff_(new Array[times.size() - 2]), dF_(new DiscountFactor[times.size() - 1]),
      v_(pathPricer_->basisSystem()), len_(times.size()) {

        for (Size i=0; i<times.size()-1; ++i) {
            dF_[i] =   termStructure->discount(times[i+1])
                     / termStructure->discount(times[i]);
        }
    }

    template <class PathType> inline
    Real LongstaffSchwartzPathPricer<PathType>::operator()
        (const PathType& path) const {
        if (calibrationPhase_) {
            // store paths for the calibration
            paths_.push_back(path);
            // result doesn't matter
            return 0.0;
        }

        Real price = (*pathPricer_)(path, len_-1);

        // Initialize with exercise on last date
        bool exercised = (price > 0.0);

        for (Size i=len_-2; i>0; --i) {
            price*=dF_[i];

            const Real exercise = (*pathPricer_)(path, i);
            if (exercise > 0.0) {
                const StateType regValue = pathPricer_->state(path, i);

                Real continuationValue = 0.0;
                for (Size l=0; l<v_.size(); ++l) {
                    continuationValue += coeff_[i-1][l] * v_[l](regValue);
                }

                if (continuationValue < exercise) {
                    price = exercise;

                    // Exercised
                    exercised = true;
                }
            }
        }

        exerciseProbability_.add(exercised ? 1.0 : 0.0);

        return price*dF_[0];
    }

    template <class PathType> inline
    void LongstaffSchwartzPathPricer<PathType>::calibrate() {
        const Size n = paths_.size();
        Array prices(n), exercise(n);
        std::vector<StateType> p_state(n);
        std::vector<Real> p_price(n), p_exercise(n);

        for (Size i=0; i<n; ++i) {
            p_state[i] = pathPricer_->state(paths_[i],len_-1);
            prices[i] = p_price[i] = (*pathPricer_)(paths_[i], len_-1);
            p_exercise[i] = prices[i];
        }

        post_processing(len_ - 1, p_state, p_price, p_exercise);

        std::vector<Real>      y;
        std::vector<StateType> x;
        for (Size i=len_-2; i>0; --i) {
            y.clear();
            x.clear();

            //roll back step
            for (Size j=0; j<n; ++j) {
                exercise[j]=(*pathPricer_)(paths_[j], i);
                if (exercise[j]>0.0) {
                    x.push_back(pathPricer_->state(paths_[j], i));
                    y.push_back(dF_[i]*prices[j]);
                }
            }

            if (v_.size() <=  x.size()) {
                coeff_[i-1] = GeneralLinearLeastSquares(x, y, v_).coefficients();
            }
            else {
            // if number of itm paths is smaller then the number of
            // calibration functions then early exercise if exerciseValue > 0
                coeff_[i-1] = Array(v_.size(), 0.0);
            }

            for (Size j=0, k=0; j<n; ++j) {
                prices[j]*=dF_[i];
                if (exercise[j]>0.0) {
                    Real continuationValue = 0.0;
                    for (Size l=0; l<v_.size(); ++l) {
                        continuationValue += coeff_[i-1][l] * v_[l](x[k]);
                    }
                    if (continuationValue < exercise[j]) {
                        prices[j] = exercise[j];
                    }
                    ++k;
                }
                p_state[j] = pathPricer_->state(paths_[j],i);
                p_price[j] = prices[j];
                p_exercise[j] = exercise[j];
            }

            post_processing(i, p_state, p_price, p_exercise);
        }

        // remove calibration paths and release memory
        std::vector<PathType> empty;
        paths_.swap(empty);
        // entering the calculation phase
        calibrationPhase_ = false;
    }

    template <class PathType> inline
    Real LongstaffSchwartzPathPricer<PathType>::exerciseProbability() const {
        return exerciseProbability_.mean();
    }


}


#endif
