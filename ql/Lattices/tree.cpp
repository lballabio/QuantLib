

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

        double Tree::presentValue(const Handle<Asset>& asset) const {
            Size i = t_.findIndex(asset->time());
            double value = 0.0;
            Size l = 0;
            for (int j=jMin(i); j<=jMax(i); j++, l++) {
                value += asset->values()[l]*node(i,j).statePrice();
            }
            return value;
        }

        void Tree::rollback(
            const std::vector<Handle<Asset> >& assets,
            Time from, Time to) const {

            QL_REQUIRE(from>=to, "Wrong rollback extremities");

            Size iFrom = t_.findIndex(from);
            Size iTo = t_.findIndex(to);

            Size width = jMax(iFrom) - jMin(iFrom) + 1;
            std::vector<Handle<Asset> >::const_iterator begin;
            for (begin = assets.begin(); begin != assets.end(); ++begin) {
                (*begin)->setTime(t(iFrom));
                (*begin)->reset(width);
            }

            for (int i=(int)(iFrom-1); i>=(int)iTo; i--) {
                for (begin = assets.begin(); begin != assets.end(); ++begin) {
                    width = jMax(i) - jMin(i) + 1;
                    (*begin)->newValues() = Array(width);
                    Size l = 0;
                    for (int j=jMin(i); j<=jMax(i); j++, l++) {
                        double value = 0.0;
                        for (Size k=0; k<n_; k++) {
                            Size index = node(i,j).descendant(k).j() -
                                jMin(i+1);
                            value += node(i,j).probability(k)
                                *(*begin)->values()[index];
                        }
                        value *= node(i,j).discount();
                        (*begin)->newValues()[l] = value;
                    }
                    (*begin)->setTime(t(i));
                    (*begin)->applyCondition();
                }
            }
        }

    }

}

