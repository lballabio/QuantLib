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
/*! \file onefactormodel.cpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/ShortRateModels/%onefactormodel.cpp
*/

// $Id$

#include "ql/ShortRateModels/onefactormodel.hpp"
#include "ql/Lattices/trinomialtree.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace ShortRateModels {

        using namespace Lattices;

        //Private function used by solver to determine time-dependent parameter
        class OneFactorModel::ShortRateTree::Helper : public ObjectiveFunction {
          public:
            Helper(
                Size i,
                double discountBondPrice,
                const Handle<TermStructureFittingParameter::NumericalImpl>& 
                      theta,
                ShortRateTree& tree)
            : size_(tree.column(i).size()),
              statePrices_(tree.statePrices(i)),
              discountBondPrice_(discountBondPrice),
              theta_(theta),
              discounting_(tree.column(i).discounting()) {
                theta_->set(tree.timeGrid()[i], 0.0);
            }

            double operator()(double theta) const {
                double value = discountBondPrice_;
                theta_->change(theta);
                for (Size j=0; j<size_; j++)
                    value -= statePrices_[j]*discounting_->discount(j);
                return value;
            }

          private:
            Size size_;
            const std::vector<double>& statePrices_;
            double discountBondPrice_;
            Handle<TermStructureFittingParameter::NumericalImpl> theta_;
            Handle<Lattices::Discounting> discounting_;
        };

        OneFactorModel::ShortRateTree::ShortRateTree(
            const Handle<ShortRateDynamics>& dynamics,
            const Handle<TermStructureFittingParameter::NumericalImpl>& theta,
            const TimeGrid& timeGrid,
            bool isPositive)
        : Lattices::TrinomialTree(
              dynamics->process(), 
              timeGrid, 
              isPositive) {

            theta->reset();
            double value = 1.0;
            double vMin = -50.0;
            double vMax = 50.0;
            for (Size i=0; i<(timeGrid.size() - 1); i++) {
                columns_[i].setDiscounting(Handle<Discounting>(new 
                    ShortRateDiscounting(dynamics, column(i).branching(), 
                                         timeGrid[i], timeGrid.dt(i), dx(i))));
                double discountBond = theta->termStructure()->discount(t_[i+1]);
                Helper finder(i, discountBond, theta, *this);
                Solvers1D::Brent s1d = Solvers1D::Brent();
                s1d.setMaxEvaluations(1000);
                value = s1d.solve(finder, 1e-7, value, vMin, vMax);
                std::cout << value << std::endl;
                vMin = value - 1.0;
                vMax = value + 1.0;
                theta->change(value);
            }
        }

        OneFactorModel::ShortRateTree::ShortRateTree(
            const Handle<ShortRateDynamics>& dynamics,
            const TimeGrid& timeGrid,
            bool isPositive)
        : TrinomialTree(dynamics->process(), timeGrid, isPositive) {
            for (Size i=0; i<(timeGrid.size() - 1); i++)
                columns_[i].setDiscounting(Handle<Discounting>(new 
                    ShortRateDiscounting(dynamics, column(i).branching(), 
                                         timeGrid[i], timeGrid.dt(i), dx(i))));
        }

        OneFactorModel::ShortRateDiscounting::ShortRateDiscounting(
            const Handle<ShortRateDynamics>& dynamics,
            const Handle<TrinomialBranching>& branching,
            Time t, Time dt, double dx)
        : t_(t), dt_(dt), dx_(dx), dynamics_(dynamics) {
              xMin_ = dynamics->process()->x0() +
                      branching->jMin()*dx;
          }

        double OneFactorModel::ShortRateDiscounting::discount(Size index) const {
            double x = xMin_ + index*dx_;
            Rate r = dynamics_->shortRate(t_, x);
            return QL_EXP(-r*dt_);
        }

        OneFactorModel::OneFactorModel(Size nParameters) : Model(nParameters) {}

        Handle<Tree> OneFactorModel::tree(const TimeGrid& grid) const {
            return Handle<Tree>(new ShortRateTree(dynamics(), grid));
        }

    }

}
