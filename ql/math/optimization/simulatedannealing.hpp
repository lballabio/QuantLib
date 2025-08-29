/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2013 Peter Caspers

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

/*! \file simulatedannealing.hpp
    \brief Numerical Recipes in C (second edition), Chapter 10.9,
           with the original exit criterion in f(x) replaced by one
           in x (see simplex.cpp for a reference to GSL concerning this)
*/

#ifndef quantlib_optimization_simulatedannealing_hpp
#define quantlib_optimization_simulatedannealing_hpp

#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <cmath>

namespace QuantLib {

    /*! Class RNG must implement the following interface:
        \code
            RNG::sample_type RNG::next() const;
        \endcode

        \ingroup optimizers
    */

    //! Simulated Annealing
    template <class RNG = MersenneTwisterUniformRng>
    class SimulatedAnnealing : public OptimizationMethod {

      public:

        enum Scheme {
            ConstantFactor,
            ConstantBudget
        };

        /*! reduce temperature T by a factor of \f$ (1-\epsilon) \f$ after m moves */
        SimulatedAnnealing(const Real lambda, const Real T0,
                           const Real epsilon, const Size m,
                           const RNG &rng = RNG())
            : scheme_(ConstantFactor), lambda_(lambda), T0_(T0),
              epsilon_(epsilon), alpha_(0.0), K_(0), rng_(rng), m_(m) {}

        /*! budget a total of K moves, set temperature T to the initial
          temperature times \f$ ( 1 - k/K )^\alpha \f$ with k being the total number
          of moves so far. After K moves the temperature is guaranteed to be
          zero, after that the optimization runs like a deterministic simplex
          algorithm.
        */
        SimulatedAnnealing(const Real lambda, const Real T0, const Size K,
                           const Real alpha, const RNG &rng = RNG())
            : scheme_(ConstantBudget), lambda_(lambda), T0_(T0), epsilon_(0.0),
              alpha_(alpha), K_(K), rng_(rng) {}

        EndCriteria::Type minimize(Problem& P, const EndCriteria& ec) override;

      private:

        const Scheme scheme_;
        const Real lambda_, T0_, epsilon_, alpha_;
        const Size K_;
        const RNG rng_;

        Real simplexSize();
        void amotsa(Problem &, Real);

        Real T_;
        std::vector<Array> vertices_;
        Array values_, sum_;
        Integer i_, ihi_, ilo_, j_, m_, n_;
        Real fac1_, fac2_, yflu_;
        Real rtol_, swap_, yhi_, ylo_, ynhi_, ysave_, yt_, ytry_, yb_, tt_;
        Array pb_, ptry_;
        Size iteration_, iterationT_;
    };

    template <class RNG>
    Real SimulatedAnnealing<RNG>::simplexSize() { // this is taken from
                                                  // simplex.cpp
        Array center(vertices_.front().size(), 0);
        for (auto& vertice : vertices_)
            center += vertice;
        center *= 1 / Real(vertices_.size());
        Real result = 0;
        for (auto& vertice : vertices_) {
            Array temp = vertice - center;
            result += Norm2(temp);
        }
        return result / Real(vertices_.size());
    }

    template <class RNG>
    void SimulatedAnnealing<RNG>::amotsa(Problem &P, Real fac) {
        fac1_ = (1.0 - fac) / ((Real)n_);
        fac2_ = fac1_ - fac;
        for (j_ = 0; j_ < n_; j_++) {
            ptry_[j_] = sum_[j_] * fac1_ - vertices_[ihi_][j_] * fac2_;
        }
        if (!P.constraint().test(ptry_))
            ytry_ = QL_MAX_REAL;
        else
            ytry_ = P.value(ptry_);
        if (std::isnan(ytry_)) {
            ytry_ = QL_MAX_REAL;
        }
        if (ytry_ <= yb_) {
            yb_ = ytry_;
            pb_ = ptry_;
        }
        yflu_ = ytry_ - tt_ * std::log(rng_.next().value);
        if (yflu_ < yhi_) {
            values_[ihi_] = ytry_;
            yhi_ = yflu_;
            for (j_ = 0; j_ < n_; j_++) {
                sum_[j_] += ptry_[j_] - vertices_[ihi_][j_];
                vertices_[ihi_][j_] = ptry_[j_];
            }
        }
        ytry_ = yflu_;
    }

    template <class RNG>
    EndCriteria::Type SimulatedAnnealing<RNG>::minimize(Problem &P,
                                                        const EndCriteria &ec) {

        Size stationaryStateIterations_ = 0;
        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();
        Array x = P.currentValue();
        iteration_ = 0;
        n_ = x.size();
        ptry_ = Array(n_, 0.0);

        // build vertices

        vertices_ = std::vector<Array>(n_ + 1, x);
        for (i_ = 0; i_ < n_; i_++) {
            Array direction(n_, 0.0);
            direction[i_] = 1.0;
            P.constraint().update(vertices_[i_ + 1], direction, lambda_);
        }
        values_ = Array(n_ + 1, 0.0);
        for (i_ = 0; i_ <= n_; i_++) {
            if (!P.constraint().test(vertices_[i_]))
                values_[i_] = QL_MAX_REAL;
            else
                values_[i_] = P.value(vertices_[i_]);
            if (std::isnan(ytry_)) { // handle NAN
                values_[i_] = QL_MAX_REAL;
            }
        }

        // minimize

        T_ = T0_;
        yb_ = QL_MAX_REAL;
        pb_ = Array(n_, 0.0);
        do {
            iterationT_ = iteration_;
            do {
                sum_ = Array(n_, 0.0);
                for (i_ = 0; i_ <= n_; i_++)
                    sum_ += vertices_[i_];
                tt_ = -T_;
                ilo_ = 0;
                ihi_ = 1;
                ynhi_ = values_[0] + tt_ * std::log(rng_.next().value);
                ylo_ = ynhi_;
                yhi_ = values_[1] + tt_ * std::log(rng_.next().value);
                if (ylo_ > yhi_) {
                    ihi_ = 0;
                    ilo_ = 1;
                    ynhi_ = yhi_;
                    yhi_ = ylo_;
                    ylo_ = ynhi_;
                }
                for (i_ = 2; i_ < n_ + 1; i_++) {
                    yt_ = values_[i_] + tt_ * std::log(rng_.next().value);
                    if (yt_ <= ylo_) {
                        ilo_ = i_;
                        ylo_ = yt_;
                    }
                    if (yt_ > yhi_) {
                        ynhi_ = yhi_;
                        ihi_ = i_;
                        yhi_ = yt_;
                    } else {
                        if (yt_ > ynhi_) {
                            ynhi_ = yt_;
                        }
                    }
                }

                // rtol_ = 2.0 * std::fabs(yhi_ - ylo_) /
                //         (std::fabs(yhi_) + std::fabs(ylo_));
                // check rtol against some ftol... // NR end criterion in f(x)

                // GSL end criterion in x (cf. above)
                if (ec.checkStationaryPoint(simplexSize(), 0.0,
                                            stationaryStateIterations_,
                                            ecType) ||
                    ec.checkMaxIterations(iteration_, ecType)) {
                    // no matter what, we return the best ever point !
                    P.setCurrentValue(pb_);
                    P.setFunctionValue(yb_);
                    return ecType;
                }

                iteration_ += 2;
                amotsa(P, -1.0);
                if (ytry_ <= ylo_) {
                    amotsa(P, 2.0);
                } else {
                    if (ytry_ >= ynhi_) {
                        ysave_ = yhi_;
                        amotsa(P, 0.5);
                        if (ytry_ >= ysave_) {
                            for (i_ = 0; i_ < n_ + 1; i_++) {
                                if (i_ != ilo_) {
                                    for (j_ = 0; j_ < n_; j_++) {
                                        sum_[j_] = 0.5 * (vertices_[i_][j_] +
                                                          vertices_[ilo_][j_]);
                                        vertices_[i_][j_] = sum_[j_];
                                    }
                                    values_[i_] = P.value(sum_);
                                }
                            }
                            iteration_ += n_;
                            for (i_ = 0; i_ < n_; i_++)
                                sum_[i_] = 0.0;
                            for (i_ = 0; i_ <= n_; i_++)
                                sum_ += vertices_[i_];
                        }
                    } else {
                        iteration_ += 1;
                    }
                }
            } while (iteration_ <
                     iterationT_ + (scheme_ == ConstantFactor ? m_ : 1));

            switch (scheme_) {
            case ConstantFactor:
                T_ *= (1.0 - epsilon_);
                break;
            case ConstantBudget:
                if (iteration_ <= K_)
                    T_ = T0_ *
                         std::pow(1.0 - (Real)iteration_ / (Real)K_, alpha_);
                else
                    T_ = 0.0;
                break;
            }

        } while (true);
    }
}

#endif
