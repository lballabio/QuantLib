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

#include <ql/numericalmethod.hpp>
#include <ql/handle.hpp>
#include <ql/grid.hpp>
#include <ql/Lattices/node.hpp>

namespace QuantLib {

    namespace Lattices {

        class Tree : public NumericalMethod {
          public:
            Tree() {}
            Tree(Size n)
            : dx_(1, 0.0), n_(n) {
                QL_REQUIRE(n>0, "There is no zeronomial tree!");
                statePricesLimit_ = 0;
            }
            virtual ~Tree() {
                for (Size i=0; i<nodes_.size(); i++) {
                    for (Size j=0; j<nodes_[i].size(); j++) {
                        delete nodes_[i][j];
                    }
                }
            }

            double presentValue(const Handle<NumericalDerivative>& asset);

            void initialize(
                const Handle<NumericalDerivative>& asset, Time t) const;
            void rollback(
                const Handle<NumericalDerivative>& asset, Time to) const;

          protected:
            typedef std::vector<Node*> Column;
            
            const Column& column(Size i) const { return nodes_[i]; }

            virtual DiscountFactor discount(Size i, int j) const = 0;

            virtual Node& descendant(Size i, int j, Size branch) = 0;
            virtual const Node& descendant(Size i, int j, Size branch) const= 0;

            Node& node(Size i, int j) {
                return *nodes_[i][nodeIndex(i,j)];
            }

            const Node& node(Size i, int j) const {
                return *nodes_[i][nodeIndex(i,j)];
            }

            virtual Size nodeIndex(Size i, int j) const = 0;

            const TimeGrid& timeGrid() const { return t_; }

            //! Returns t_i
            Time t(Size i) const { return t_[i]; }

            //! Returns \delta t_i = t_{i+1} - t_i
            Time dt(Size i) const { return t_[i+1] - t_[i]; }

            //! Returns \delta x_i
            double dx(Size i) const { return dx_[i]; }

            void computeStatePrices(Size until);
            TimeGrid t_;
            std::vector<double> dx_;
            std::vector<Column> nodes_;

          private:
            Size n_;
            Size statePricesLimit_;
        };

    }

}

#endif
