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
/*! \file column.hpp
    \brief Tree column class

    \fullpath
    ql/Lattices/%column.hpp
*/

// $Id$

#ifndef quantlib_lattices_column_h
#define quantlib_lattices_column_h

namespace QuantLib {

    namespace Lattices {

        class Branching {
          public:
            virtual ~Branching() {}
            virtual Size descendant(Size index, Size branch) const = 0;
            virtual double probability(Size index, Size branch) const = 0;
        };
        
        class Discounting {
          public:
            virtual ~Discounting() {}
            virtual double discount(Size index) const = 0;
        };

        class Column {
          public:
            Column(Size size) : statePrices_(size, 0.0) {}

            double discount(Size index) const {
                return discounting_->discount(index);
            }

            Size descendant(Size index, Size branch) const {
                return branching_->descendant(index, branch);
            }

            double probability(Size index, Size branch) const {
                return branching_->probability(index, branch);
            }

            Size size() const { return statePrices_.size(); }

            const Handle<Branching>& branching() const {
                return branching_;
            }
            void setBranching(const Handle<Branching>& branching) {
                branching_ = branching;
            }
            const Handle<Discounting>& discounting() const {
                return discounting_;
            }
            void setDiscounting(const Handle<Discounting>& discounting) {
                discounting_ = discounting;
            }

            //! returns the Arrow-Debrew price of the jth node
            double statePrice(Size j) const { return statePrices_[j];}

            void addToStatePrice(Size j, double dx) const { 
                double& statePrice = const_cast<double&>(statePrices_[j]);
                statePrice += dx;
            }

            //! returns Arrow-Debrew prices
            const std::vector<double>& statePrices() const { 
                return statePrices_;
            }

          private:
            std::vector<double> statePrices_;
            Handle<Branching> branching_;
            Handle<Discounting> discounting_;
        };


    }

}

#endif
