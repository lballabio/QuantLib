

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
/*! \file blackkarasinski.cpp
    \brief Black-Karasinski model

    \fullpath
    ql/InterestRateModelling/OneFactorModels/%blackkarasinski.cpp
*/

// $Id$

#include "ql/InterestRateModelling/OneFactorModels/blackkarasinski.hpp"
#include "ql/Solvers1D/brent.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using Lattices::Node;
        using Lattices::TimeGrid;
        using Lattices::Tree;

        //Short-rate diffusion process
        class BlackKarasinski::Process : public ShortRateProcess {
          public:
            Process(BlackKarasinski * model)
            :  model_(model), a_(model_->a_), sigma_(model_->sigma_) {}

            virtual double variable(Time t, Rate r) const {
                return QL_LOG(r) - model_->alpha(t);
            }

            virtual double shortRate(Time t, double x) const {
                return QL_EXP(model_->alpha(t) + x);
            }

            virtual double drift(Time t, double lnR) const {
                return -a_*lnR;
            }
            virtual double diffusion(Time t, double lnR) const {
                return sigma_;
            }
          private:
            BlackKarasinski * model_;
            const double& a_;
            const double& sigma_;
        };

        //Private function used by solver to determine time-dependent parameter
        class BlackKarasinski::PrivateFunction : public ObjectiveFunction {
          public:
            PrivateFunction::PrivateFunction( double dt, double dx, int jMin,
                int jMax, const std::vector<double>& statePrices,
                double discountBondPrice)
            : dt_(dt), dx_(dx), jMin_(jMin), jMax_(jMax),
              statePrices_(statePrices),
              discountBondPrice_(discountBondPrice) {}

            double operator()(double x) const {
                double value = discountBondPrice_;
                Size k=0;
                for (int j=jMin_; j<=jMax_; j++)
                    value -= statePrices_[k++]*QL_EXP(-QL_EXP(x + j*dx_)*dt_);
                return value;
            }

          private:
            double dt_, dx_;
            int jMin_, jMax_;
            const std::vector<double>& statePrices_;
            double discountBondPrice_;
            Size nit_;
        };

        //Trinomial tree specific to the BK model
        class BlackKarasinski::PrivateTree : public TrinomialTree {
          public:
            PrivateTree(double a,
                        double sigma,
                        const RelinkableHandle<TermStructure>& termStructure,
                        const Handle<TimeFunction>& alpha,
                        const TimeGrid& timeGrid) {

                t_ = timeGrid;

                //adjust space intervals
                dx_.resize(t_.size());
                dx_[0] = 0.0; //Just one node
                Size i;
                for (i=0; i<(dx_.size()-1); i++) {
                    double v = sigma*
                        QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*a*dt(i)))/a);
                    dx_[i+1] = v*QL_SQRT(3);
                }

                Size nTimeSteps = t_.size() - 1;

                alpha->reset();

                double lastValue = 0.0;
                for (i=0; i<nTimeSteps; i++) {
                    double discountBond = termStructure->discount(t(i+1));
                    std::vector<double> statePrices(0);
                    int j;
                    for (j=jMin(i); j<=jMax(i); j++)
                        statePrices.push_back(node(i,j).statePrice());
                    BlackKarasinski::PrivateFunction finder(dt(i), dx(i),
                        jMin(i), jMax(i), statePrices, discountBond);
                    // solver
                    Solvers1D::Brent s1d = Solvers1D::Brent();
                    s1d.setMaxEvaluations(1000);
                    lastValue = s1d.solve(finder, 1e-6, lastValue, -10.0, 1.0);
                    alpha->set(t(i), lastValue);

                    std::vector<int> k(0);

                    double v = dx(i+1)/QL_SQRT(3);
                    double v2 = v*v;

                    for (j=jMin(i); j<=jMax(i); j++) {
                        double x = j*dx(i);
                        double m = x*QL_EXP(-a*dt(i));
                        k.push_back( (int)floor(m/dx(i+1) + 0.5) );
                        double e = m - k.back()*dx(i+1);
                        double e2 = e*e;

                        double pUp  = (1.0 + e2/v2 + e*QL_SQRT(3)/v)/6.0;
                        double pMid  = (2.0 - e2/v2)/3.0;
                        double pDown = (1.0 + e2/v2 - e*QL_SQRT(3)/v)/6.0;

                        node(i,j).setProbability(pUp, 2);
                        node(i,j).setProbability(pMid, 1);
                        node(i,j).setProbability(pDown, 0);

                        double discount =
                            QL_EXP(-QL_EXP(lastValue+j*dx(i))*dt(i));
                        node(i,j).setDiscount(discount);
                    }
                    addLevel(k);
                }
            }
        };


        BlackKarasinski::BlackKarasinski(
            const RelinkableHandle<TermStructure>& termStructure)
        : OneFactorModel(2, termStructure), alpha_(new TimeFunction()),
          a_(params_[0]), sigma_(params_[1]) {

            process_ = Handle<ShortRateProcess>(new Process(this));

            constraint_ = Handle<Constraint>(new Constraint(2));
            constraint_->setLowerBound(1, 0.000001);
        }

        Handle<Tree> BlackKarasinski::tree(
            const TimeGrid& timeGrid) const {
            return Handle<Tree>(new PrivateTree(a_, sigma_,
                                     termStructure(), alpha_, timeGrid));
        }

    }

}
