/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2005 StatPro Italia srl

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

/*! \file binomialtree.hpp
    \brief Binomial tree class
*/

#ifndef quantlib_binomial_tree_hpp
#define quantlib_binomial_tree_hpp


#include <ql/methods/lattices/tree.hpp>
#include <ql/instruments/dividendschedule.hpp>
#include <ql/stochasticprocess.hpp>

namespace QuantLib {

    //! Binomial tree base class
    /*! \ingroup lattices */
    template <class T>
    class BinomialTree : public Tree<T> {
      public:
        enum Branches { branches = 2 };
        BinomialTree(const ext::shared_ptr<StochasticProcess1D>& process,
                     Time end,
                     Size steps)
        : Tree<T>(steps+1), x0_(process->x0()), dt_(end/steps) {
            driftPerStep_ = process->drift(0.0, x0_) * dt_;
        }
        Size size(Size i) const {
            return i+1;
        }
        Size descendant(Size, Size index, Size branch) const {
            return index + branch;
        }
      protected:
        Real x0_, driftPerStep_;
        Time dt_;
    };


    //! Base class for equal probabilities binomial tree
    /*! \ingroup lattices */
    template <class T>
    class EqualProbabilitiesBinomialTree : public BinomialTree<T> {
      public:
        EqualProbabilitiesBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end,
                        Size steps)
        : BinomialTree<T>(process, end, steps) {}
        Real underlying(Size i, Size index) const {
            BigInteger j = 2*BigInteger(index) - BigInteger(i);
            // exploiting the forward value tree centering
            return this->x0_*std::exp(i*this->driftPerStep_ + j*this->up_);
        }
        Real probability(Size, Size, Size) const { return 0.5; }
      protected:
        Real up_;
    };


    //! Base class for equal jumps binomial tree
    /*! \ingroup lattices */
    template <class T>
    class EqualJumpsBinomialTree : public BinomialTree<T> {
      public:
        EqualJumpsBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>& process,
                        Time end,
                        Size steps)
        : BinomialTree<T>(process, end, steps) {}
        Real underlying(Size i, Size index) const {
            BigInteger j = 2*BigInteger(index) - BigInteger(i);
            // exploiting equal jump and the x0_ tree centering
            return this->x0_*std::exp(j*this->dx_);
        }
        Real probability(Size, Size, Size branch) const {
            return (branch == 1 ? pu_ : pd_);
        }
      protected:
        Real dx_, pu_, pd_;
    };


    //! Jarrow-Rudd (multiplicative) equal probabilities binomial tree
    /*! \ingroup lattices */
    class JarrowRudd : public EqualProbabilitiesBinomialTree<JarrowRudd> {
      public:
        JarrowRudd(const ext::shared_ptr<StochasticProcess1D>&,
                   Time end,
                   Size steps,
                   Real strike);
    };


    //! Cox-Ross-Rubinstein (multiplicative) equal jumps binomial tree
    /*! \ingroup lattices */
    class CoxRossRubinstein
        : public EqualJumpsBinomialTree<CoxRossRubinstein> {
      public:
        CoxRossRubinstein(const ext::shared_ptr<StochasticProcess1D>&,
                          Time end,
                          Size steps,
                          Real strike);
    };


    //! Additive equal probabilities binomial tree
    /*! \ingroup lattices */
    class AdditiveEQPBinomialTree
        : public EqualProbabilitiesBinomialTree<AdditiveEQPBinomialTree> {
      public:
        AdditiveEQPBinomialTree(
                        const ext::shared_ptr<StochasticProcess1D>&,
                        Time end,
                        Size steps,
                        Real strike);
    };


    //! %Trigeorgis (additive equal jumps) binomial tree
    /*! \ingroup lattices */
    class Trigeorgis : public EqualJumpsBinomialTree<Trigeorgis> {
      public:
        Trigeorgis(const ext::shared_ptr<StochasticProcess1D>&,
                   Time end,
                   Size steps,
                   Real strike);
    };


    //! %Tian tree: third moment matching, multiplicative approach
    /*! \ingroup lattices */
    class Tian : public BinomialTree<Tian> {
      public:
        Tian(const ext::shared_ptr<StochasticProcess1D>&,
             Time end,
             Size steps,
             Real strike);
        Real underlying(Size i, Size index) const {
            return x0_ * std::pow(down_, Real(BigInteger(i)-BigInteger(index)))
                       * std::pow(up_, Real(index));
        };
        Real probability(Size, Size, Size branch) const {
            return (branch == 1 ? pu_ : pd_);
        }
      protected:
        Real up_, down_, pu_, pd_;
    };

    //! Leisen & Reimer tree: multiplicative approach
    /*! \ingroup lattices */
    class LeisenReimer : public BinomialTree<LeisenReimer> {
      public:
        LeisenReimer(const ext::shared_ptr<StochasticProcess1D>&,
                     Time end,
                     Size steps,
                     Real strike);
        Real underlying(Size i, Size index) const {
            return x0_ * std::pow(down_, Real(BigInteger(i)-BigInteger(index)))
                       * std::pow(up_, Real(index));
        }
        Real probability(Size, Size, Size branch) const {
            return (branch == 1 ? pu_ : pd_);
        }
      protected:
        Real up_, down_, pu_, pd_;
    };


     class Joshi4 : public BinomialTree<Joshi4> {
      public:
        Joshi4(const ext::shared_ptr<StochasticProcess1D>&,
               Time end,
               Size steps,
               Real strike);
        Real underlying(Size i, Size index) const {
            return x0_ * std::pow(down_, Real(BigInteger(i)-BigInteger(index)))
                       * std::pow(up_, Real(index));
        }
        Real probability(Size, Size, Size branch) const {
            return (branch == 1 ? pu_ : pd_);
        }
      protected:
        Real computeUpProb(Real k, Real dj) const;
        Real up_, down_, pu_, pd_;
    };


}


#endif


#ifndef id_234ff45b12aa5592079566f6c7d761c4
#define id_234ff45b12aa5592079566f6c7d761c4
inline bool test_234ff45b12aa5592079566f6c7d761c4(const int* i) {
    return i != nullptr;
}
#endif
