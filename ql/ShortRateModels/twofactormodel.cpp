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
/*! \file twofactormodel.cpp
    \brief Abstract two-factor interest rate model class

    \fullpath
    ql/ShortRateModels/%twofactormodel.cpp
*/

// $Id$

#include <ql/ShortRateModels/twofactormodel.hpp>

namespace QuantLib {

    namespace ShortRateModels {

        using namespace Lattices;

        TwoFactorModel::ShortRateDiscounting::ShortRateDiscounting(
            const Handle<ShortRateDynamics>& dynamics,
            const Handle<TwoDimensionalBranching>& branching,
            Time t, Time dt, double dx, double dy)
        : t_(t), dt_(dt), dx_(dx), dy_(dy), dynamics_(dynamics) {
              modulo_ = branching->modulo();
              xMin_ = dynamics->xProcess()->x0() +
                      branching->branching1()->jMin()*dx;
              yMin_ = dynamics->yProcess()->x0() +
                      branching->branching2()->jMin()*dy;
          }

        double 
        TwoFactorModel::ShortRateDiscounting::discount(Size index) const {
            double x = xMin_ + (index%modulo_)*dx_;
            double y = yMin_ + ((int)(index/modulo_))*dy_;

            Rate r = dynamics_->shortRate(t_, x, y);
            return QL_EXP(-r*dt_);
        }

        TwoFactorModel::TwoFactorModel(Size nParameters) : Model(nParameters) {}

        Handle<Tree> TwoFactorModel::tree(const TimeGrid& grid) const {
            Handle<ShortRateDynamics> dyn = dynamics();

            Handle<TrinomialTree> tree1(
                new TrinomialTree(dyn->xProcess(), grid));
            Handle<TrinomialTree> tree2(
                new TrinomialTree(dyn->yProcess(), grid));

            Handle<Tree> temp(
                new TwoDimensionalTree(tree1, tree2, dyn->correlation()));
            for (Size i=0; i<(grid.size() - 1); i++) {
                Column& column = const_cast<Column&>(temp->column(i));
                column.setDiscounting(Handle<Discounting>(new 
                    ShortRateDiscounting(dyn, temp->column(i).branching(), 
                                         grid[i], grid.dt(i), 
                                         tree1->dx(i), tree2->dx(i))));
            }

            return temp;
        }

    }

}
