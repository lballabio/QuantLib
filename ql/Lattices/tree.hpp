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
/*! \file tree.hpp
    \brief Tree class

    \fullpath
    ql/Lattices/%tree.hpp
*/

// $Id$

#ifndef quantlib_lattices_tree_h
#define quantlib_lattices_tree_h

#include <ql/handle.hpp>
#include <ql/grid.hpp>
#include <ql/numericalmethod.hpp>
#include <ql/Lattices/column.hpp>

namespace QuantLib {

    namespace Lattices {

        class Tree : public NumericalMethod {
          public:
            Tree(const TimeGrid& timeGrid, Size n) 
            : NumericalMethod(timeGrid), n_(n) {
                QL_REQUIRE(n>0, "There is no zeronomial tree!");
                statePricesLimit_ = 0;
            }

            //! Computes the present value of an asset using Arrow-Debrew prices
            double presentValue(const Handle<DiscretizedAsset>& asset);

            void initialize(
                const Handle<DiscretizedAsset>& asset, Time t) const;
            void rollback(
                const Handle<DiscretizedAsset>& asset, Time to) const;

            const Column& column(Size i) const { return columns_[i]; }

            const std::vector<double>& statePrices(Size i);

          protected:
            void computeStatePrices(Size until);
            std::vector<Column> columns_;

          private:
            Size n_;
            Size statePricesLimit_;
        };

    }

}

#endif
