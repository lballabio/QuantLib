/*
 Copyright (C) 2001, 2002 Sadruddin Rejeb

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file tree.cpp
    \brief Tree class

    \fullpath
    ql/Lattices/%tree.cpp
*/

// $Id$

#include "ql/Lattices/tree.hpp"

namespace QuantLib {

    namespace Lattices {

        using std::cout;
        using std::endl;

        void Tree::computeStatePrices(Size until) {
            for (Size i=statePricesLimit_; i<until; i++) {
                for (int j=jMin(i); j<=jMax(i); j++) {
                    for (Size n=0; n<n_; n++) {
                        node(i,j).descendant(n).statePrice() +=
                            node(i,j).statePrice()*
                            node(i,j).probability(n)*
                            discount(i,j);
                    }
                }
            }
            statePricesLimit_ = until;
        }

        double Tree::presentValue(const Handle<Asset>& asset) {
            Size i = t_.findIndex(asset->time());
            if (i>statePricesLimit_)
                computeStatePrices(i);
            double value = 0.0;
            Size l = 0;
            for (int j=jMin(i); j<=jMax(i); j++, l++) {
                value += asset->values()[l]*node(i,j).statePrice();
            }
            return value;
        }

        void Tree::initialize(const Handle<Asset>& asset, Time t) const {
            Size i = t_.findIndex(t);
            Size width = jMax(i) - jMin(i) + 1;
            asset->setTime(t);
            asset->reset(width);
        }

        void Tree::rollback(const Handle<Asset>& asset, Time to) const {
            std::vector<Handle<Asset> > assets(1, asset);
            rollback(assets, to);
        }

        void Tree::rollback(
            const std::vector<Handle<Asset> >& assets,
            Time to) const {

            std::vector<Handle<Asset> >::const_iterator begin;
            Time from = (*begin)->time();
            ++begin;
            for (; begin != assets.end(); ++begin) {
                QL_REQUIRE((*begin)->time()==from,
                    "Assets must be at the same time!");
            }

            QL_REQUIRE(from>=to, "Wrong rollback extremities");
            Size iFrom = t_.findIndex(from);
            Size iTo = t_.findIndex(to);

            for (int i=(int)(iFrom-1); i>=(int)iTo; i--) {
                for (begin = assets.begin(); begin != assets.end(); ++begin) {
                    Size width = jMax(i) - jMin(i) + 1;
                    Array newValues(width);
                    Size l = 0;
                    for (int j=jMin(i); j<=jMax(i); j++, l++) {
                        double value = 0.0;
                        for (Size k=0; k<n_; k++) {
                            Size index = node(i,j).descendant(k).j() -
                                jMin(i+1);
                            value += node(i,j).probability(k)
                                *(*begin)->values()[index];
                        }
                        value *= discount(i, j);
                        newValues[l] = value;
                    }
                    (*begin)->setTime(t(i));
                    (*begin)->setValues(newValues);
                    (*begin)->applyCondition();
                }
            }
        }

    }

}

