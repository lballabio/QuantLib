/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2005 StatPro Italia srl
 Copyright (C) 2008 John Maiden

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

/*! \file extendedbinomialtree.hpp
    \brief Time-dependent binomial tree class
*/

#ifndef quantlib_extended_binomial_tree_hpp
#define quantlib_extended_binomial_tree_hpp

#include <ql/methods/lattices/tree.hpp>
#include <ql/instruments/dividendschedule.hpp>
#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! Binomial tree base class
    /*! \ingroup lattices */
    template <class T>
    class ExtendedBinomialTree : public Tree<T> {
      public:
        enum Branches { branches = 2 };
        ExtendedBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end,
                        Size steps)
        : Tree<T>(steps+1), x0_(process->x0()), dt_(end/steps),
          treeProcess_(process) {}
        Size size(Size i) const {
            return i+1;
        }
        Size descendant(Size, Size index, Size branch) const {
            return index + branch;
        }
      protected:
        //time dependent drift per step
        Real driftStep(Time driftTime) const {
            return this->treeProcess_->drift(driftTime, x0_) * dt_;
        }

        Real x0_;
        Time dt_;


        ext::shared_ptr<StochasticProcess1D> treeProcess_;
    };


    //! Base class for equal probabilities binomial tree
    /*! \ingroup lattices */
    template <class T>
    class ExtendedEqualProbabilitiesBinomialTree
        : public ExtendedBinomialTree<T> {
      public:
        ExtendedEqualProbabilitiesBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end,
                        Size steps)
        : ExtendedBinomialTree<T>(process, end, steps) {}
        virtual ~ExtendedEqualProbabilitiesBinomialTree() = default;

        Real underlying(Size i, Size index) const {
            Time stepTime = i*this->dt_;
            BigInteger j = 2*BigInteger(index) - BigInteger(i);
            // exploiting the forward value tree centering
            return this->x0_*std::exp(i*this->driftStep(stepTime) + j*this->upStep(stepTime));
        }

        Real probability(Size, Size, Size) const { return 0.5; }
      protected:
        //the tree dependent up move term at time stepTime
        virtual Real upStep(Time stepTime) const = 0;
        Real up_;
    };


    //! Base class for equal jumps binomial tree
    /*! \ingroup lattices */
    template <class T>
    class ExtendedEqualJumpsBinomialTree : public ExtendedBinomialTree<T> {
      public:
        ExtendedEqualJumpsBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end,
                        Size steps)
        : ExtendedBinomialTree<T>(process, end, steps) {}
        virtual ~ExtendedEqualJumpsBinomialTree() = default;

        Real underlying(Size i, Size index) const {
            Time stepTime = i*this->dt_;
            BigInteger j = 2*BigInteger(index) - BigInteger(i);
            // exploiting equal jump and the x0_ tree centering
            return this->x0_*std::exp(j*this->dxStep(stepTime));
        }

        Real probability(Size i, Size, Size branch) const {
            Time stepTime = i*this->dt_;
            Real upProb = this->probUp(stepTime);
            Real downProb = 1 - upProb;
            return (branch == 1 ? upProb : downProb);
        }
      protected:
        //probability of a up move
        virtual Real probUp(Time stepTime) const = 0;
        //time dependent term dx_
        virtual Real dxStep(Time stepTime) const = 0;

        Real dx_, pu_, pd_;
    };


    //! Jarrow-Rudd (multiplicative) equal probabilities binomial tree
    /*! \ingroup lattices */
    class ExtendedJarrowRudd
        : public ExtendedEqualProbabilitiesBinomialTree<ExtendedJarrowRudd> {
      public:
        ExtendedJarrowRudd(const ext::shared_ptr<StochasticProcess1D>&,
                           Time end,
                           Size steps,
                           Real strike);
      protected:
        Real upStep(Time stepTime) const override;
    };


    //! Cox-Ross-Rubinstein (multiplicative) equal jumps binomial tree
    /*! \ingroup lattices */
    class ExtendedCoxRossRubinstein
        : public ExtendedEqualJumpsBinomialTree<ExtendedCoxRossRubinstein> {
      public:
        ExtendedCoxRossRubinstein(const ext::shared_ptr<StochasticProcess1D>&,
                                  Time end,
                                  Size steps,
                                  Real strike);
      protected:
        Real dxStep(Time stepTime) const override;
        Real probUp(Time stepTime) const override;
    };


    //! Additive equal probabilities binomial tree
    /*! \ingroup lattices */
    class ExtendedAdditiveEQPBinomialTree
        : public ExtendedEqualProbabilitiesBinomialTree<
                                            ExtendedAdditiveEQPBinomialTree> {
      public:
        ExtendedAdditiveEQPBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>&,
                        Time end,
                        Size steps,
                        Real strike);

      protected:
        Real upStep(Time stepTime) const override;
    };


    //! %Trigeorgis (additive equal jumps) binomial tree
    /*! \ingroup lattices */
    class ExtendedTrigeorgis
        : public ExtendedEqualJumpsBinomialTree<ExtendedTrigeorgis> {
      public:
        ExtendedTrigeorgis(const ext::shared_ptr<StochasticProcess1D>&,
                           Time end,
                           Size steps,
                           Real strike);
    protected:
      Real dxStep(Time stepTime) const override;
      Real probUp(Time stepTime) const override;
    };


    //! %Tian tree: third moment matching, multiplicative approach
    /*! \ingroup lattices */
    class ExtendedTian : public ExtendedBinomialTree<ExtendedTian> {
      public:
        ExtendedTian(const ext::shared_ptr<StochasticProcess1D>&,
                     Time end,
                     Size steps,
                     Real strike);

        Real underlying(Size i, Size index) const;
        Real probability(Size, Size, Size branch) const;
      protected:
        Real up_, down_, pu_, pd_;
    };

    //! Leisen & Reimer tree: multiplicative approach
    /*! \ingroup lattices */
    class ExtendedLeisenReimer
        : public ExtendedBinomialTree<ExtendedLeisenReimer> {
      public:
        ExtendedLeisenReimer(const ext::shared_ptr<StochasticProcess1D>&,
                             Time end,
                             Size steps,
                             Real strike);

        Real underlying(Size i, Size index) const;
        Real probability(Size, Size, Size branch) const;
      protected:
        Time end_;
        Size oddSteps_;
        Real strike_, up_, down_, pu_, pd_;
    };


     class ExtendedJoshi4 : public ExtendedBinomialTree<ExtendedJoshi4> {
      public:
        ExtendedJoshi4(const ext::shared_ptr<StochasticProcess1D>&,
                       Time end,
                       Size steps,
                       Real strike);

        Real underlying(Size i, Size index) const;
        Real probability(Size, Size, Size branch) const;
      protected:
        Real computeUpProb(Real k, Real dj) const;
        Time end_;
        Size oddSteps_;
        Real strike_, up_, down_, pu_, pd_;
    };


}


#endif
