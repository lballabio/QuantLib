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

/*! \file particleswarmoptimization.hpp
\brief Implementation based on:
Clerc, M., Kennedy, J. (2002) The particle swarm-explosion, stability and
convergence in a multidimensional complex space. IEEE Transactions on Evolutionary
Computation, 6(2): 58–73.
*/

#ifndef quantlib_optimization_particleswarmoptimization_hpp
#define quantlib_optimization_particleswarmoptimization_hpp

#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/randomnumbers/mt19937uniformrng.hpp>
#include <ql/experimental/math/isotropicrandomwalk.hpp>
#include <ql/experimental/math/levyflightdistribution.hpp>
#include <ql/math/randomnumbers/seedgenerator.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>

namespace QuantLib {

    typedef boost::mt19937 base_generator_type;
    typedef boost::random::uniform_int_distribution<QuantLib::Size> uniform_integer;

    /*! The process is as follows:
    M individuals are used to explore the N-dimensional parameter space:
    \f$ X_{i}^k = (X_{i, 1}^k, X_{i, 2}^k, \ldots, X_{i, N}^k) \f$ is the kth-iteration for the ith-individual.

    X is updated via the rule
    \f[
    X_{i, j}^{k+1} = X_{i, j}^k + V_{i, j}^{k+1}
    \f]
    with V being the "velocity" that updates the position:
    \f[
    V_{i, j}^{k+1} = \chi\left(V_{i, j}^k + c_1 r_{i, j}^k (P_{i, j}^k - X_{i, j}^k)
    + c_2 R_{i, j}^k (G_{i, j}^k - X_{i, j}^k)\right)
    \f]
    where c are constants, r and R are uniformly distributed random numbers in the range [0, 1], and
    \f$ P_{i, j} \f$ is the personal best parameter set for individual i up to iteration k
    \f$ G_{i, j} \f$ is the global best parameter set for the swarm up to iteration k.
    \f$ c_1 \f$ is the self recognition coefficient
    \f$ c_2 \f$ is the social recognition coefficient

    This version is known as the PSO with constriction factor (PSO-Co).
    PSO with inertia factor (PSO-In) updates the velocity according to:
    \f[
    V_{i, j}^{k+1} = \omega V_{i, j}^k + \hat{c}_1 r_{i, j}^k (P_{i, j}^k - X_{i, j}^k)
    + \hat{c}_2 R_{i, j}^k (G_{i, j}^k - X_{i, j}^k)
    \f]
    and is accessible from PSO-Co by setting \f$ \omega = \chi \f$,
    and \f$ \hat{c}_{1,2} = \chi c_{1,2} \f$.

    These two versions of PSO are normally referred to as canonical PSO.

    Convergence of PSO-Co is improved if \f$ \chi \f$ is chosen as
    \f$ \chi = \frac{2}{\vert 2-\phi-\sqrt{\phi^2 - 4\phi}\vert} \f$,
    with \f$ \phi = c_1 + c_2 \f$.
    Stable convergence is achieved if \f$ \phi >= 4 \f$. Clerc and Kennedy recommend
    \f$ c_1 = c_2 = 2.05 \f$ and \f$ \phi = 4.1 \f$.

    Different topologies can be chosen for G, e.g. instead of it being the best
    of the swarm, it is the best of the nearest neighbours, or some other form.

    In the canonical PSO, the inertia function is trivial. It is simply a
    constant (the inertia) multiplying the previous iteration's velocity. The
    value of the inertia constant determines the weight of a global search over
    local search. Like in the case of the topology, other possibilities for the
    inertia function are also possible, e.g. a function that interpolates between a
    high inertia at the beginning of the optimization (hence prioritizing a global
    search) and a low inertia towards the end of the optimization (hence prioritizing
    a local search).

    The optimization stops either because the number of iterations has been reached
    or because the stationary function value limit has been reached.
    */
    class ParticleSwarmOptimization : public OptimizationMethod {
      public:
        class Inertia;
        class Topology;
        friend class Inertia;
        friend class Topology;
        ParticleSwarmOptimization(Size M,
                                  ext::shared_ptr<Topology> topology,
                                  ext::shared_ptr<Inertia> inertia,
                                  Real c1 = 2.05,
                                  Real c2 = 2.05,
                                  unsigned long seed = SeedGenerator::instance().get());
        explicit ParticleSwarmOptimization(Size M,
                                           ext::shared_ptr<Topology> topology,
                                           ext::shared_ptr<Inertia> inertia,
                                           Real omega,
                                           Real c1,
                                           Real c2,
                                           unsigned long seed = SeedGenerator::instance().get());
        void startState(Problem &P, const EndCriteria &endCriteria);
        EndCriteria::Type minimize(Problem& P, const EndCriteria& endCriteria) override;

      protected:
        std::vector<Array> X_, V_, pBX_, gBX_;
        Array pBF_, gBF_;
        Array lX_, uX_;
        Size M_, N_;
        Real c0_, c1_, c2_;
        MersenneTwisterUniformRng rng_;
        ext::shared_ptr<Topology> topology_;
        ext::shared_ptr<Inertia> inertia_;
    };

    //! Base inertia class used to alter the PSO state
    /*! This pure virtual base class provides the access to the PSO state
    which the particular inertia algorithm will change upon each iteration.
    */
    class ParticleSwarmOptimization::Inertia {
        friend class ParticleSwarmOptimization;
      public:
        virtual ~Inertia() = default;
        //! initialize state for current problem
        virtual void setSize(Size M, Size N, Real c0, const EndCriteria &endCriteria) = 0;
        //! produce changes to PSO state for current iteration
        virtual void setValues() = 0;
      protected:
        ParticleSwarmOptimization *pso_;
        std::vector<Array> *X_, *V_, *pBX_, *gBX_;
        Array *pBF_, *gBF_;
        Array *lX_, *uX_;

        virtual void init(ParticleSwarmOptimization *pso) {
            pso_ = pso;
            X_ = &pso_->X_;
            V_ = &pso_->V_;
            pBX_ = &pso_->pBX_;
            gBX_ = &pso_->gBX_;
            pBF_ = &pso_->pBF_;
            gBF_ = &pso_->gBF_;
            lX_ = &pso_->lX_;
            uX_ = &pso_->uX_;
        }
    };

    //! Trivial Inertia
    /*     Inertia is a static value
    */
    class TrivialInertia : public ParticleSwarmOptimization::Inertia {
      public:
        inline void setSize(Size M, Size N, Real c0, const EndCriteria& endCriteria) override {
            c0_ = c0;
            M_ = M;
        }
        inline void setValues() override {
            for (Size i = 0; i < M_; i++) {
                (*V_)[i] *= c0_;
            }
        }

      private:
        Real c0_;
        Size M_;
    };

    //! Simple Random Inertia
    /*     Inertia value gets multiplied with a random number
    between (threshold, 1)
    */
    class SimpleRandomInertia : public ParticleSwarmOptimization::Inertia {
      public:
        SimpleRandomInertia(Real threshold = 0.5, unsigned long seed = SeedGenerator::instance().get())
            : threshold_(threshold), rng_(seed) {
            QL_REQUIRE(threshold_ >= 0.0 && threshold_ < 1.0, "Threshold must be a Real in [0, 1)");
        }
        inline void setSize(Size M, Size N, Real c0, const EndCriteria& endCriteria) override {
            M_ = M;
            c0_ = c0;
        }
        inline void setValues() override {
            for (Size i = 0; i < M_; i++) {
                Real val = c0_*(threshold_ + (1.0 - threshold_)*rng_.nextReal());
                (*V_)[i] *= val;
            }
        }

      private:
        Real c0_, threshold_;
        Size M_;
        MersenneTwisterUniformRng rng_;
    };

    //! Decreasing Inertia
    /*     Inertia value gets decreased every iteration until it reaches
    a value of threshold when iteration reaches the maximum level
    */
    class DecreasingInertia : public ParticleSwarmOptimization::Inertia {
      public:
        DecreasingInertia(Real threshold = 0.5)
            : threshold_(threshold) {
            QL_REQUIRE(threshold_ >= 0.0 && threshold_ < 1.0, "Threshold must be a Real in [0, 1)");
        }
        inline void setSize(Size M, Size N, Real c0, const EndCriteria& endCriteria) override {
            N_ = N;
            c0_ = c0;
            iteration_ = 0;
            maxIterations_ = endCriteria.maxIterations();
        }
        inline void setValues() override {
            Real c0 = c0_*(threshold_ + (1.0 - threshold_)*(maxIterations_ - iteration_) / maxIterations_);
            for (Size i = 0; i < M_; i++) {
                (*V_)[i] *= c0;
            }
        }

      private:
        Real c0_, threshold_;
        Size M_, N_, maxIterations_, iteration_;
    };

    //! AdaptiveInertia
    /*    Alen Lukic, Approximating Kinetic Parameters Using Particle
    Swarm Optimization.
    */
    class AdaptiveInertia : public ParticleSwarmOptimization::Inertia {
      public:
        AdaptiveInertia(Real minInertia, Real maxInertia, Size sh = 5, Size sl = 2)
            :minInertia_(minInertia), maxInertia_(maxInertia),
            sh_(sh), sl_(sl) {};
        inline void setSize(Size M, Size N, Real c0, const EndCriteria& endCriteria) override {
            M_ = M;
            c0_ = c0;
            adaptiveCounter = 0;
            best_ = QL_MAX_REAL;
            started_ = false;
        }
        void setValues() override;

      private:
        Real c0_, best_;
        Real minInertia_, maxInertia_;
        Size M_;
        Size sh_, sl_;
        Size adaptiveCounter;
        bool started_;
    };

    //! Levy Flight Inertia
    /*    As long as the particle keeps getting frequent updates to its
    personal best value, the inertia behaves like a SimpleRandomInertia,
    but after a number of iterations without improvement, the behaviour
    changes to that of a Levy flight ~ u^{-1/\alpha}
    */
    class LevyFlightInertia : public ParticleSwarmOptimization::Inertia {
      public:
        typedef IsotropicRandomWalk<LevyFlightDistribution, base_generator_type> IsotropicLevyFlight;
        LevyFlightInertia(Real alpha, Size threshold,
                          unsigned long seed = SeedGenerator::instance().get())
            :rng_(seed), flight_(base_generator_type(seed), LevyFlightDistribution(1.0, alpha),
                1, Array(1, 1.0), seed),
            threshold_(threshold) {};
        inline void setSize(Size M, Size N, Real c0, const EndCriteria& endCriteria) override {
            M_ = M;
            N_ = N;
            c0_ = c0;
            adaptiveCounter_ = std::vector<Size>(M_, 0);
        }
        inline void setValues() override {
            for (Size i = 0; i < M_; i++) {
                if ((*pBF_)[i] < personalBestF_[i]) {
                    personalBestF_[i] = (*pBF_)[i];
                    adaptiveCounter_[i] = 0;
                }
                else {
                    adaptiveCounter_[i]++;
                }
                if (adaptiveCounter_[i] <= threshold_) {
                    //Simple Random Inertia
                    (*V_)[i] *= c0_*(0.5 + 0.5*rng_.nextReal());
                }
                else {
                    //If particle has not found a new personal best after threshold_ iterations
                    //then trigger a Levy flight pattern for the speed
                    flight_.nextReal<Real *>(&(*V_)[i][0]);
                }
            }
        }

      protected:
        void init(ParticleSwarmOptimization* pso) override {
            ParticleSwarmOptimization::Inertia::init(pso);
            personalBestF_ = *pBF_;
            flight_.setDimension(N_, *lX_, *uX_);
        }

      private:
        MersenneTwisterUniformRng rng_;
        IsotropicLevyFlight flight_;
        Array personalBestF_;
        std::vector<Size> adaptiveCounter_;
        Real c0_;
        Size M_, N_;
        Size threshold_;
    };

    //! Base topology class used to determine the personal and global best
    /*! This pure virtual base class provides the access to the PSO state
    which the particular topology algorithm will change upon each iteration.
    */
    class ParticleSwarmOptimization::Topology {
        friend class ParticleSwarmOptimization;
      public:
        virtual ~Topology() = default;
        //! initialize state for current problem
        virtual void setSize(Size M) = 0;
        //! produce changes to PSO state for current iteration
        virtual void findSocialBest() = 0;
      protected:
        ParticleSwarmOptimization *pso_;
        std::vector<Array> *X_, *V_, *pBX_, *gBX_;
        Array *pBF_, *gBF_;
      private:
        void init(ParticleSwarmOptimization *pso) {
            pso_ = pso;
            X_ = &pso_->X_;
            V_ = &pso_->V_;
            pBX_ = &pso_->pBX_;
            gBX_ = &pso_->gBX_;
            pBF_ = &pso_->pBF_;
            gBF_ = &pso_->gBF_;
        }
    };

    //! Global Topology
    /*  The global best as seen by each particle is the best from amongst
    all particles
    */
    class GlobalTopology : public ParticleSwarmOptimization::Topology {
      public:
        inline void setSize(Size M) override { M_ = M; }
        inline void findSocialBest() override {
            Real bestF = (*pBF_)[0];
            Size bestP = 0;
            for (Size i = 1; i < M_; i++) {
                if (bestF < (*pBF_)[i]) {
                    bestF = (*pBF_)[i];
                    bestP = i;
                }
            }
            Array& x = (*pBX_)[bestP];
            for (Size i = 0; i < M_; i++) {
                if (i != bestP) {
                    (*gBX_)[i] = x;
                    (*gBF_)[i] = bestF;
                }
            }
        }

      private:
        Size M_;
    };

    //! K-Neighbor Topology
    /*  The global best as seen by each particle is the best from amongst
    the previous K and next K neighbors. For particle I, the best is
    then taken from amongst the [I - K, I + K] particles.
    */
    class KNeighbors : public ParticleSwarmOptimization::Topology {
      public:
        KNeighbors(Size K = 1) :K_(K) {
            QL_REQUIRE(K > 0, "Neighbors need to be larger than 0");
        }
        inline void setSize(Size M) override {
            M_ = M;
            QL_ENSURE(K_ < M, "Number of neighbors need to be smaller than total particles in swarm");
        }
        void findSocialBest() override;

      private:
        Size K_, M_;
    };

    //! Clubs Topology
    /*  H.M. Emara,  Adaptive Clubs-based Particle Swarm Optimization
    Each particle is originally assigned to a default number of clubs
    from among the total set. The best as seen by each particle is the
    best from amongst the clubs to which the particle belongs.
    Underperforming particles join more clubs randomly (up to a maximum
    number) to widen the particles that influence them, while
    overperforming particles leave clubs randomly (down to a minimum
    number) to avoid early convergence to local minima.
    */
    class ClubsTopology : public ParticleSwarmOptimization::Topology {
      public:
        ClubsTopology(Size defaultClubs, Size totalClubs,
            Size maxClubs, Size minClubs,
            Size resetIteration, unsigned long seed = SeedGenerator::instance().get());
        void setSize(Size M) override;
        void findSocialBest() override;

      private:
        Size totalClubs_, maxClubs_, minClubs_, defaultClubs_;
        Size iteration_, resetIteration_;
        Size M_;
        std::vector<std::vector<bool> > clubs4particles_;
        std::vector<std::vector<bool> > particles4clubs_;
        std::vector<Size> bestByClub_;
        std::vector<Size> worstByClub_;
        base_generator_type generator_;
        uniform_integer distribution_;

        void leaveRandomClub(Size particle, Size currentClubs);
        void joinRandomClub(Size particle, Size currentClubs);
    };

}

#endif
