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

#include <ql/asset.hpp>
#include <ql/Lattices/node.hpp>
#include <ql/Lattices/timegrid.hpp>

namespace QuantLib {

    namespace Lattices {

        class Tree {
          public:
            Tree() {}
            Tree(Size n)
            : dx_(0), n_(n) {
                QL_REQUIRE(n>0, "There is no zeronomial tree!");
                nodes_.push_back(std::vector<Handle<Node> >());
                nodes_[0].push_back(Handle<Node>(new Node(n,0,0)));
                nodes_[0][0]->statePrice() = 1.0;
                statePricesLimit_ = 0;
            }
            virtual ~Tree() {}

            double presentValue(const Handle<Asset>& asset);

            void initialize(const Handle<Asset>& assets, Time t) const;
            void rollback(const Handle<Asset>& asset, Time to) const;
            void rollback(const std::vector<Handle<Asset> >& assets,
                          Time to) const;

          protected:
            virtual DiscountFactor discount(Size i, int j) const = 0;

            virtual const Node& node(Size i, int j) const = 0;

            virtual Node& node(Size i, int j) = 0;

            const TimeGrid& timeGrid() const { return t_; }

            //! Returns t_i
            Time t(Size i) const { return t_[i]; }

            //! Returns \delta t_i = t_{i+1} - t_i
            Time dt(Size i) const { return t_[i+1] - t_[i]; }

            //! Returns \delta x_i
            double dx(Size i) const { return dx_[i]; }

            //! Returns jMin
            int jMin(Size i) const { return nodes_[i].front()->j(); }

            //! Returns jMax
            int jMax(Size i) const { return nodes_[i].back()->j(); }

            void computeStatePrices(Size until);
            TimeGrid t_;
            std::vector<double> dx_;
            std::vector<std::vector<Handle<Node> > > nodes_;

          private:
            Size n_;
            Size statePricesLimit_;
        };

    }

}

#endif
