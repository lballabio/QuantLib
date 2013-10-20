/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2004, 2005 StatPro Italia srl

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

/*! \file lattice.hpp
    \brief Tree-based lattice-method class
*/

#ifndef quantlib_tree_based_lattice_hpp
#define quantlib_tree_based_lattice_hpp

#include <ql/numericalmethod.hpp>
#include <ql/discretizedasset.hpp>
#include <ql/patterns/curiouslyrecurring.hpp>

namespace QuantLib {

    //! Tree-based lattice-method base class
    /*! This class defines a lattice method that is able to rollback
        (with discount) a discretized asset object. It will be based
        on one or more trees.

        Derived classes must implement the following interface:
        \code
        public:
          DiscountFactor discount(Size i, Size index) const;
          Size descendant(Size i, Size index, Size branch) const;
          Real probability(Size i, Size index, Size branch) const;
        \endcode
        and may implement the following:
        \code
        public:
          void stepback(Size i,
                        const Array& values,
                        Array& newValues) const;
        \endcode

        \ingroup lattices
    */
    template <class Impl>
    class TreeLattice : public Lattice,
                        public CuriouslyRecurringTemplate<Impl> {
      public:
        TreeLattice(const TimeGrid& timeGrid,
                    Size n)
        : Lattice(timeGrid), n_(n) {
            QL_REQUIRE(n>0, "there is no zeronomial lattice!");
            statePrices_ = std::vector<Array>(1, Array(1, 1.0));
            statePricesLimit_ = 0;
        }

        //! \name Lattice interface
        //@{
        void initialize(DiscretizedAsset&, Time t) const;
        void rollback(DiscretizedAsset&, Time to) const;
        void partialRollback(DiscretizedAsset&, Time to) const;
        //! Computes the present value of an asset using Arrow-Debrew prices
        Real presentValue(DiscretizedAsset&) const;
        //@}

        const Array& statePrices(Size i) const;

        void stepback(Size i,
                      const Array& values,
                      Array& newValues) const;

      protected:
        void computeStatePrices(Size until) const;

        // Arrow-Debrew state prices
        mutable std::vector<Array> statePrices_;

      private:
        Size n_;
        mutable Size statePricesLimit_;
    };


    // template definitions

    template <class Impl>
    void TreeLattice<Impl>::computeStatePrices(Size until) const {
        for (Size i=statePricesLimit_; i<until; i++) {
            statePrices_.push_back(Array(this->impl().size(i+1), 0.0));
            for (Size j=0; j<this->impl().size(i); j++) {
                DiscountFactor disc = this->impl().discount(i,j);
                Real statePrice = statePrices_[i][j];
                for (Size l=0; l<n_; l++) {
                    statePrices_[i+1][this->impl().descendant(i,j,l)] +=
                        statePrice*disc*this->impl().probability(i,j,l);
                }
            }
        }
        statePricesLimit_ = until;
    }

    template <class Impl>
    const Array& TreeLattice<Impl>::statePrices(Size i) const {
        if (i>statePricesLimit_)
            computeStatePrices(i);
        return statePrices_[i];
    }

    template <class Impl>
    inline Real TreeLattice<Impl>::presentValue(DiscretizedAsset& asset) const {
        Size i = t_.index(asset.time());
        return DotProduct(asset.values(), statePrices(i));
    }

    template <class Impl>
    inline void TreeLattice<Impl>::initialize(DiscretizedAsset& asset, Time t) const {
        Size i = t_.index(t);
        asset.time() = t;
        asset.reset(this->impl().size(i));
    }

    template <class Impl>
    inline void TreeLattice<Impl>::rollback(DiscretizedAsset& asset, Time to) const {
        partialRollback(asset,to);
        asset.adjustValues();
    }

    template <class Impl>
    void TreeLattice<Impl>::partialRollback(DiscretizedAsset& asset,
                                            Time to) const {

        Time from = asset.time();

        if (close(from,to))
            return;

        QL_REQUIRE(from > to,
                   "cannot roll the asset back to" << to
                   << " (it is already at t = " << from << ")");

        Integer iFrom = Integer(t_.index(from));
        Integer iTo = Integer(t_.index(to));

        for (Integer i=iFrom-1; i>=iTo; --i) {
            Array newValues(this->impl().size(i));
            this->impl().stepback(i, asset.values(), newValues);
            asset.time() = t_[i];
            asset.values() = newValues;
            // skip the very last adjustment
            if (i != iTo)
                asset.adjustValues();
        }
    }

    template <class Impl>
    void TreeLattice<Impl>::stepback(Size i, const Array& values,
                                     Array& newValues) const {
        #pragma omp parallel for
        for (Size j=0; j<this->impl().size(i); j++) {
            Real value = 0.0;
            for (Size l=0; l<n_; l++) {
                value += this->impl().probability(i,j,l) *
                         values[this->impl().descendant(i,j,l)];
            }
            value *= this->impl().discount(i,j);
            newValues[j] = value;
        }
    }

}


#endif
