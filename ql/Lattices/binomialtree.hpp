
/*
 Copyright (C) 2003 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file binomialtree.hpp
    \brief Binomial tree class
*/

#ifndef quantlib_lattices_binomial_tree_h
#define quantlib_lattices_binomial_tree_h

#include <ql/stochasticprocess.hpp>
#include <ql/Lattices/tree.hpp>

namespace QuantLib {

    //! Binomial tree base class
    /*! \ingroup lattices */
    class BinomialTree : public Tree {
      public:
        BinomialTree(const boost::shared_ptr<StochasticProcess>& process,
                     Time end,
                     unsigned long steps);
        Size size(Size i) const {
            return i+1;
        }
        Size descendant(Size,
                        Size index,
                        Size branch) const {
            return index + branch;
        }
        virtual double underlying(Size i,
                                  Size index) const = 0;
        virtual double probability(Size i,
                                   Size index,
                                   Size branch) const = 0;
      protected:
        double x0_, driftPerStep_;
        Time dt_;
    };




    //! Base class for equal probabilities binomial tree
    /*! \ingroup lattices */
    class EqualProbabilitiesBinomialTree : public BinomialTree {
      public:
        EqualProbabilitiesBinomialTree(
                           const boost::shared_ptr<StochasticProcess>& process,
                           Time end,
                           unsigned long steps)
        : BinomialTree(process, end, steps) {}
        double underlying(Size i,
                          Size index) const;
        double probability(Size,
                           Size,
                           Size) const {return 0.5 ; }
      protected:
        double up_;
    };



    //! Base class for equal jumps binomial tree
    /*! \ingroup lattices */
    class EqualJumpsBinomialTree : public BinomialTree {
      public:
        EqualJumpsBinomialTree(
                           const boost::shared_ptr<StochasticProcess>& process,
                           Time end,
                           unsigned long steps)
        : BinomialTree(process, end, steps) {}
        double underlying(Size i,
                          Size index) const;
        double probability(Size,
                           Size,
                           Size branch) const;
      protected:
        double dx_, pu_, pd_;
    };



    //! Jarrow-Rudd (multiplicative) equal probabilities binomial tree
    /*! \ingroup lattices */
    class JarrowRudd : public EqualProbabilitiesBinomialTree {
      public:
        JarrowRudd(const boost::shared_ptr<StochasticProcess>& process,
                   Time end,
                   unsigned long steps,
                   double strike);
    };

    //! Cox-Ross-Rubinstein (multiplicative) equal jumps binomial tree
    /*! \ingroup lattices */
    class CoxRossRubinstein : public EqualJumpsBinomialTree {
      public:
        CoxRossRubinstein(const boost::shared_ptr<StochasticProcess>& process,
                          Time end,
                          unsigned long steps,
                          double strike);
    };


    //! Additive equal probabilities binomial tree
    /*! \ingroup lattices */
    class AdditiveEQPBinomialTree : public EqualProbabilitiesBinomialTree {
      public:
        AdditiveEQPBinomialTree(
                           const boost::shared_ptr<StochasticProcess>& process,
                           Time end,
                           unsigned long steps,
                           double strike);
    };

    //! %Trigeorgis (additive equal jumps) binomial tree
    /*! \ingroup lattices */
    class Trigeorgis : public EqualJumpsBinomialTree {
      public:
        Trigeorgis(const boost::shared_ptr<StochasticProcess>& process,
                   Time end,
                   unsigned long steps,
                   double strike);
    };


    //! %Tian tree: third moment matching, multiplicative approach
    /*! \ingroup lattices */
    class Tian : public BinomialTree {
      public:
        Tian(const boost::shared_ptr<StochasticProcess>& process,
             Time end,
             unsigned long steps,
             double strike);
        double underlying(Size i,
                          Size index) const;
        double probability(Size,
                           Size,
                           Size) const;
      protected:
        double up_, down_, pu_, pd_;
    };

    //! Leisen & Reimer tree: multiplicative approach
    /*! \ingroup lattices */
    class LeisenReimer : public BinomialTree {
      public:
        LeisenReimer(const boost::shared_ptr<StochasticProcess>& process,
                     Time end,
                     unsigned long steps,
                     double strike);
        double underlying(Size i,
                          Size index) const;
        double probability(Size,
                           Size,
                           Size) const;
      protected:
        double up_, down_, pu_, pd_;
    };

}


#endif
