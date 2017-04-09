/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
Copyright (C) 2015 Andres Hernandez

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

/*! \file fireflyalgorithm.hpp
\brief Implementation based on:
Yang, Xin-She (2009) Firefly Algorithm, Levy Flights and Global
Optimization. Research and Development in Intelligent Systems XXVI, pp 209-218.
http://arxiv.org/pdf/1003.1464.pdf
*/

#ifndef quantlib_optimization_fireflyalgorithm_hpp
#define quantlib_optimization_fireflyalgorithm_hpp

#include <ql/qldefines.hpp>

#if BOOST_VERSION >= 104700

#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/experimental/math/isotropicrandomwalk.hpp>
#include <ql/experimental/math/levyflightdistribution.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>

#include <boost/random/mersenne_twister.hpp>
typedef boost::mt19937 base_generator_type;

#include <boost/random/normal_distribution.hpp>
typedef boost::random::normal_distribution<QuantLib::Real> BoostNormalDistribution;
#include <boost/random/uniform_int_distribution.hpp>
typedef boost::random::uniform_int_distribution<QuantLib::Size> uniform_integer;
#include <boost/random/variate_generator.hpp>
typedef boost::variate_generator<base_generator_type, uniform_integer> variate_integer;

#include <cmath>

namespace QuantLib {

    /*! The main process is as follows:
    M individuals are used to explore the N-dimensional parameter space
    X_{i}^k = (X_{i, 1}^k, X_{i, 2}^k, \ldots, X_{i, N}^k) is the kth-iteration 
    for the ith-individual. X is updated via the rule
    X_{i, j}^{k+1} = X_{i, j}^k + I(X^k)_{i,j} + RandomWalk_{i,j}^k

    The intensity function I(X) should be monotonic
    The optimization stops either because the number of iterations has been reached
    or because the stationary function value limit has been reached.

    The current implementation extends the normal Firefly Algorithm with a 
    differential evolution (DE) optimizer according to:
    Afnizanfaizal Abdullah, et al. "A New Hybrid Firefly Algorithm for Complex and 
    Nonlinear Problem". Volume 151 of the series Advances in Intelligent and Soft 
    Computing pp 673-680, 2012.
    http://link.springer.com/chapter/10.1007%2F978-3-642-28765-7_81
    
    In effect this implementation provides a fully fledged DE global optimizer 
    as well. The Firefly Algorithm was easy to combine with DE because it already 
    contained a step where the current solutions are sorted. The population is 
    then divided into two subpopulations based on their order. The subpopulation 
    with the best results are updated via the firefly algorithm. The worse 
    subpopulation is updated via the DE operator:
    Y^{k+1} = X_{best}^k + F(X_{r1}^k - X_{r2}^k)
    and 
    X_{i,j}^{k+1} = if R_{i,j} <= CR Y_{i,j}^{k+1}
                    if R_{i,j} > CR X_{i,j}^{k+1}
    Where CR is the crossover constant, and R is a random uniformly distributed
    number
    */
    class FireflyAlgorithm : public OptimizationMethod {
      public:
        class RandomWalk;
        class Intensity;
        friend class RandomWalk;
        friend class Intensity;
        FireflyAlgorithm(Size M, 
            boost::shared_ptr<Intensity> intensity,
            boost::shared_ptr<RandomWalk> randomWalk,
            Size Mde = 0, Real mutationFactor = 1.0,
            Real crossoverFactor = 0.5, unsigned long seed = SeedGenerator::instance().get());
        void startState(Problem &P, const EndCriteria &endCriteria);
        EndCriteria::Type minimize(Problem &P, const EndCriteria &endCriteria);

      protected:
        std::vector<Array> x_, xI_, xRW_; 
        std::vector<std::pair<Real, Size> > values_;
        Array lX_, uX_;
        Real mutation_, crossover_;
        Size M_, N_, Mde_, Mfa_;
        boost::shared_ptr<Intensity> intensity_;
        boost::shared_ptr<RandomWalk> randomWalk_;
        variate_integer drawIndex_;
        MersenneTwisterUniformRng rng_;
    };

    //! Base intensity class
    /*! Derived classes need to implement only intensityImpl
    */
    class FireflyAlgorithm::Intensity {
        friend class FireflyAlgorithm;
    public:
        virtual ~Intensity() {}
        //! find brightest firefly for each firefly
        void findBrightest();
    protected:
        Size Mfa_, N_;
        const std::vector<Array> *x_;
        const std::vector<std::pair<Real, Size> > *values_;
        std::vector<Array> *xI_;

        virtual Real intensityImpl(Real valueX, Real valueY, Real distance) = 0;
        inline Real distance(const Array& x, const Array& y) {
            Real d = 0.0;
            for (Size i = 0; i < N_; i++) {
                Real diff = x[i] - y[i];
                d += diff*diff;
            }
            return d;
        }
    private:
        void init(FireflyAlgorithm *fa) {
            x_ = &fa->x_;
            xI_ = &fa->xI_;
            values_ = &fa->values_;
            Mfa_ = fa->Mfa_;
            N_ = fa->N_;
        }
    };

    //! Exponential Intensity
    /*  Exponentially decreasing intensity
    */
    class ExponentialIntensity : public FireflyAlgorithm::Intensity {
      public:
          ExponentialIntensity(Real beta0, Real betaMin, Real gamma)
              : beta0_(beta0), betaMin_(betaMin), gamma_(gamma) {}
      protected:
          Real intensityImpl(Real valueX, Real valueY, Real d){
              return (beta0_ - betaMin_)*std::exp(-gamma_*d) + betaMin_;
          }
          Real beta0_, betaMin_, gamma_;
    };

    //! Inverse Square Intensity
    /*  Inverse law square
    */
    class InverseLawSquareIntensity : public FireflyAlgorithm::Intensity {
    public:
        InverseLawSquareIntensity(Real beta0, Real betaMin)
            : beta0_(beta0), betaMin_(betaMin) {}
    protected:
        Real intensityImpl(Real valueX, Real valueY, Real d) {
            return (beta0_ - betaMin_)/(d+ QL_EPSILON) + betaMin_;
        }
        Real beta0_, betaMin_;
    };

    //! Base Random Walk class
    /*! Derived classes need to implement only walkImpl
    */
    class FireflyAlgorithm::RandomWalk {
        friend class FireflyAlgorithm;
    public:
        virtual ~RandomWalk() {}
        //! perform random walk
        void walk() {
            for (Size i = 0; i < Mfa_; i++) {
                walkImpl((*xRW_)[(*values_)[i].second]);
            }
        }
    protected:
        Size Mfa_, N_;
        const std::vector<Array> *x_;
        const std::vector<std::pair<Real, Size> > *values_;
        std::vector<Array> *xRW_;
        Array *lX_, *uX_;

        virtual void walkImpl(Array & xRW) = 0;
        virtual void init(FireflyAlgorithm *fa) {
            x_ = &fa->x_;
            xRW_ = &fa->xRW_;
            values_ = &fa->values_;
            Mfa_ = fa->Mfa_;
            N_ = fa->N_;
            lX_ = &fa->lX_;
            uX_ = &fa->uX_;
        }
    };

    //! Distribution Walk
    /*  Random walk given by distribution template parameter. The
        distribution must be compatible with boost's Random 
        variate_generator
    */
    template <class Distribution>
    class DistributionRandomWalk : public FireflyAlgorithm::RandomWalk {
    public:
        typedef IsotropicRandomWalk<Distribution, base_generator_type> WalkRandom;
        DistributionRandomWalk(Distribution dist, 
		                       Real delta = 0.9, 
                               unsigned long seed = SeedGenerator::instance().get()) :
            walkRandom_(base_generator_type(seed), dist, 1, Array(1, 1.0), seed),
            delta_(delta) {}
    protected:
        void walkImpl(Array & xRW) {
            walkRandom_.nextReal(&xRW[0]);
            xRW *= delta_;
        }
        void init(FireflyAlgorithm *fa) {
            FireflyAlgorithm::RandomWalk::init(fa);
            walkRandom_.setDimension(N_, *lX_, *uX_);
        }
        WalkRandom walkRandom_;
        Real delta_;
    };
    
    //! Gaussian Walk
    /*  Gaussian random walk
    */
    class GaussianWalk : public DistributionRandomWalk<BoostNormalDistribution> {
    public:
        GaussianWalk(Real sigma, 
		             Real delta = 0.9, 
                     unsigned long seed = SeedGenerator::instance().get())
        : DistributionRandomWalk<BoostNormalDistribution>(
                           BoostNormalDistribution(0.0, sigma), delta, seed){}
    };

    //! Levy Flight Random Walk
    /*  Levy flight random walk
    */
    class LevyFlightWalk : public DistributionRandomWalk<LevyFlightDistribution> {
    public:
        LevyFlightWalk(Real alpha, 
		               Real xm = 0.5, 
                       Real delta = 0.9, unsigned long seed = SeedGenerator::instance().get())
        : DistributionRandomWalk<LevyFlightDistribution>(
                            LevyFlightDistribution(xm, alpha), delta, seed) {}
    };

    //! Decreasing Random Walk
    /*  Gaussian random walk, but size of step decreases with each iteration step
    */
    class DecreasingGaussianWalk : public GaussianWalk {
      public:
        DecreasingGaussianWalk(Real sigma, Real delta = 0.9, unsigned long seed = SeedGenerator::instance().get()):
            GaussianWalk(sigma, delta, seed), delta0_(delta){}
      protected:
        void walkImpl(Array & xRW) {
            iteration_++;
            if (iteration_ > Mfa_) {
                //Every time all the fireflies have been processed
                //multiply delta by itself
                iteration_ = 0;
                delta_ *= delta_;
            }
            GaussianWalk::walkImpl(xRW);
        }
        void init(FireflyAlgorithm *fa) {
            GaussianWalk::init(fa);
            iteration_ = 0;
            delta_ = delta0_;
        }
      private:
        Real delta0_;
        Size iteration_;
    };
}

#endif

#endif
