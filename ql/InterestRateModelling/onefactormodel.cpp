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
    ql/InterestRateModelling/%onefactormodel.cpp
*/

// $Id$

#include "ql/InterestRateModelling/onefactormodel.hpp"
#include "ql/Lattices/trinomialtree.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using Optimization::Constraint;

        class OneFactorModel::StandardConstraint : public Constraint {
            virtual bool test(const Array& params) const {
                if (params[params.size()-1] > 0.0)
                    return true;
                else
                    return false;
            }
            virtual void correct(Array& params) const {
                if (!test(params))
                    params[params.size()-1] = 0.000001;
            }
        };

        //Private function used by solver to determine time-dependent parameter
        class OneFactorModel::OwnTrinomialTree::Helper 
            : public ObjectiveFunction {
          public:
            Helper(
                Time t, Time dt, double dx, 
                int jMin, int jMax,
                const std::vector<double>& statePrices,
                double discountBondPrice,
                const Handle<ShortRateProcess>& process,
                const 
                    Handle<TermStructureFittingParameter::NumericalImpl>& theta)
            : t_(t), dt_(dt), dx_(dx), 
              jMin_(jMin), jMax_(jMax),
              statePrices_(statePrices),
              discountBondPrice_(discountBondPrice),
              process_(process), theta_(theta) {
                theta_->set(t, 0.0);
            }

            double operator()(double theta) const {
                double value = discountBondPrice_;
                theta_->change(theta);
                Size k=0;
                double x = process_->x0() + jMin_*dx_;
                for (int j=jMin_; j<=jMax_; j++, k++) {
                    Rate r = process_->shortRate(t_, x);
                    value -= statePrices_[k]*QL_EXP(-r*dt_);
                    x += dx_;
                }
                return value;
            }

          private:
            Time t_, dt_;
            double dx_;
            int jMin_, jMax_;
            const std::vector<double>& statePrices_;
            double discountBondPrice_;
            Handle<ShortRateProcess> process_;
            Handle<TermStructureFittingParameter::NumericalImpl> theta_;
        };

        OneFactorModel::OwnTrinomialTree::OwnTrinomialTree(
            const Handle<ShortRateProcess>& process,
            const Handle<TermStructureFittingParameter::NumericalImpl>& theta,
            const TimeGrid& timeGrid,
            bool isPositive)
        : Lattices::TrinomialTree(process, timeGrid, isPositive), 
          process_(process) {

            theta->reset();
            for (Size i=0; i<(timeGrid.size() - 1); i++) {
                double discountBond = theta->termStructure()->discount(t(i+1));
                std::vector<double> statePrices(0);
                for (int j=jMin(i); j<=jMax(i); j++)
                    statePrices.push_back(node(i,j).statePrice());
                Helper finder(t(i), dt(i), dx(i), jMin(i), jMax(i), 
                              statePrices, discountBond, process_, theta);
                Solvers1D::Brent s1d = Solvers1D::Brent();
                s1d.setMaxEvaluations(1000);
                double value = s1d.solve(finder, 1e-6, 1.0, -50.0, 50.0);
                theta->change(value);
                computeStatePrices(i+1);
            }
        }

        OneFactorModel::OneFactorModel(
            Size nParameters,
            const RelinkableHandle<TermStructure>& termStructure)
        : Model(nParameters, termStructure) {
            constraint_ = Handle<Constraint>(new StandardConstraint());
        }

    }

}
