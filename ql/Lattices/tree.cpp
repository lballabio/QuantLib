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

        void Tree::computeStatePrices(Size until) {
            for (Size i=statePricesLimit_; i<until; i++) {
                Column::const_iterator c = column(i).begin();
                for (; c != column(i).end(); c++) {
                    const Node& n = *(*c);
                    int j = n.j();
                    double discountFactor = discount(i, j);
                    for (Size l=0; l<n_; l++) {
                        descendant(i,j,l).statePrice() +=
                            n.statePrice()*n.probability(l)*discountFactor;
                    }
                }
            }
            statePricesLimit_ = until;
        }

        double Tree::presentValue(const Handle<NumericalDerivative>& asset) {
            Size i = t_.findIndex(asset->time());
            if (i>statePricesLimit_)
                computeStatePrices(i);
            double value = 0.0;
            Size l = 0;
            Column::const_iterator n = column(i).begin();
            for (; n != column(i).end(); n++, l++) {
                value += asset->values()[l]*(*n)->statePrice();
            }
            return value;
        }

        void Tree::initialize(const Handle<NumericalDerivative>& asset, 
                              Time t) const {

            Size i = t_.findIndex(t);
            Size width = column(i).size();
            asset->setTime(t);
            asset->reset(width);
        }

        void Tree::rollback(const Handle<NumericalDerivative>& asset, 
                            Time to) const {

            Time from = asset->time();

            QL_REQUIRE(from>=to, "Wrong rollback extremities");
            Size iFrom = t_.findIndex(from);
            Size iTo = t_.findIndex(to);

            for (int i=(int)(iFrom-1); i>=(int)iTo; i--) {
                Size width = column(i).size();
                Array newValues(width);
                Size k = 0;
                Column::const_iterator c = column(i).begin();
                for (; c != column(i).end(); c++, k++) {
                    const Node& n = *(*c);
                    int j = n.j();
                    double value = 0.0;
                    Size index = nodeIndex(i+1, descendant(i,j,0).j());
                    for (Size l=0; l<n_; l++) {
                        value += n.probability(l)*asset->values()[index+l];
                    }
                    value *= discount(i, j);
                    newValues[k] = value;
                }
                asset->setTime(t(i));
                asset->setValues(newValues);
                asset->applyCondition();
            }
        }

    }

}

