
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
 *
 * QuantLib is free software and you are allowed to use, copy, modify, merge,
 * publish, distribute, and/or sell copies of it under the conditions stated
 * in the QuantLib License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
 *
 * You should have received a copy of the license along with this file;
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file blackkarasinski.cpp
    \brief Black-Karasinski model

    \fullpath
    ql/%blackkarasinski.cpp
*/

// $Id$

#include <cmath>
#include "ql/InterestRateModelling/OneFactorModels/blackkarasinski.hpp"
#include "ql/Solvers1D/brent.hpp"

using std::vector;

namespace QuantLib {

    namespace InterestRateModelling {

        class BlackKarasinski::Process : public StochasticProcess {
          public:
            Process(BlackKarasinski * model) 
            : StochasticProcess(LogShortRate), model_(model) {}

            virtual double drift(double lnR, double t) const {
                return model_->theta(t) - lnR*model_->alpha_;
            }
            virtual double diffusion(double lnR, Time t) const {
                return model_->sigma_;
            }
          private:
            BlackKarasinski * model_;
        };

        class BlackKarasinski::PrivateFunction : public ObjectiveFunction {
          public:
            PrivateFunction(double dt, double dx, signed jMin, signed jMax,
                const vector<double>& statePrices, double discountBondPrice);
            double operator()(double x) const;
          private:
            double dt_, dx_;
            signed jMin_, jMax_;
            const vector<double>& statePrices_;
            double discountBondPrice_;
            unsigned int nit_;
        };      
            
        inline BlackKarasinski::PrivateFunction::PrivateFunction( 
            double dt, double dx, signed jMin, signed jMax,
            const vector<double>& statePrices, double discountBondPrice) 
        : dt_(dt), dx_(dx), jMin_(jMin), jMax_(jMax), 
          statePrices_(statePrices), discountBondPrice_(discountBondPrice) {}

        inline double BlackKarasinski::PrivateFunction::operator()(double x) const {
            double value = discountBondPrice_;
            unsigned k=0;
            for (signed j=jMin_; j<=jMax_; j++)
                value -= statePrices_[k++]*QL_EXP(-QL_EXP(x + j*dx_)*dt_);
            return value;
        }

        BlackKarasinski::BlackKarasinski(
            const RelinkableHandle<TermStructure>& termStructure, 
            unsigned int timeSteps) 
        : OneFactorModel(2, termStructure), timeSteps_(timeSteps), 
          theta_(timeSteps), u_(timeSteps) {

            process_ = Handle<StochasticProcess>(new Process(this));

            dt_ = termStructure->maxTime()/(timeSteps_*1.0);

            constraint_ = Handle<Constraint>(new Constraint(
                  vector<double>(1, 0.0),
                  vector<double>(1, 1.0)));

            alpha_ = 0.10;
            sigma_ = 0.10;
            calculateTree();
        }

        void BlackKarasinski::calculateTree() {
            cout << "Initializing BK for sigma = " << sigma_*100.0 << "%" << endl;

            tree_.node(0,0).setStatePrice(1.0);

            cout << "tree built with depth " << timeSteps_ << endl;

            double v = sigma_*QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*alpha_*dt_))/alpha_);
            double v2 = v*v;
            double dx = v*QL_SQRT(3);

            signed jMin = 0, jMax = 0;

            for (unsigned i=0; i<timeSteps_; i++) {
                unsigned width = jMax - jMin + 1;

                double discountBond = termStructure()->discount(dt_*(i+1));
                vector<double> statePrices(width);
                unsigned index=0;
                for (signed j=jMin; j<=jMax; j++)
                    statePrices[index++] = tree_.node(i,j).statePrice();
                PrivateFunction finder(dt_, dx, jMin, jMax, statePrices, discountBond);
                // solver
                Solvers1D::Brent s1d = Solvers1D::Brent();
                double minStrike = -10.0;
                double maxStrike = 1.0;
                s1d.setMaxEvaluations(1000);
                s1d.setLowBound(minStrike);
                s1d.setHiBound(maxStrike);
                cout << "Solving for u" << endl;
                u_[i] = s1d.solve(finder, 1e-8, 0.05, minStrike, maxStrike);
                cout << "u[" << i << "] = " << u_[i] << endl;

                vector<signed> k(width);

                index = 0;
                for (signed j=jMin; j<=jMax; j++) {
                    double x = j*dx;
                    double m = x*QL_EXP(-alpha_*dt_);
                    k[index] = (signed)round(m/dx);
                    double e = m - k[index]*dx;
                    double e2 = e*e;

                    double pUp  = (1.0 + e2/v2 + e*QL_SQRT(3)/v)/6.0;
                    double pMid  = (2.0 - e2/v2)/3.0;
                    double pDown = (1.0 + e2/v2 - e*QL_SQRT(3)/v)/6.0;

                    tree_.node(i,j).setProbability(pUp, 2);
                    tree_.node(i,j).setProbability(pMid, 1);
                    tree_.node(i,j).setProbability(pDown, 0);

                    index++;
                }
                tree_.addLevel(k);
                jMin = k.front() - 1;
                jMax = k.back() + 1;
                for (signed j=jMin; j<=jMax; j++) {
                    double value = 0.0;
                    Node& node = tree_.node(i+1,j);
                    unsigned nb = node.nbAscendants();
                    for (unsigned l=0; l<nb; l++) {
                        const Node& parent = node.ascendant(l);
                        value += parent.statePrice()
                            *parent.probability(node.ascendantBranch(l))
                            *QL_EXP(-QL_EXP(u_[i]+parent.j()*dx)*dt_);
                    }
                    node.setStatePrice(value);
                }
            }
            
            cout << "Tree filled " << endl;
            for (unsigned i=0; i<(timeSteps_-1); i++)
                theta_[i] = (u_[i+1] - u_[i])/dt_ + alpha_*u_[i];
            theta_[timeSteps_-1] = theta_[timeSteps_-2];
        }

        double BlackKarasinski::discountBond(Time now, Time maturity, Rate r) const {

            Rate r0 = termStructure()->forward(0.0);
            TrinomialTree tree;

            unsigned iNow = (unsigned)(now/dt_) + 1;
            unsigned iMat = (unsigned)(maturity/dt_);
            unsigned timeSteps = iMat - iNow + 2;

            vector<double> dts(timeSteps, dt_);
            dts.front() = iNow*dt_ - now;
            dts.back() = maturity - iMat*dt_;
            
            signed jMin = 0, jMax = 0;

            for (unsigned i=0; i<timeSteps_; i++) {
                double dt = dts[i];
                double v = sigma_*QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*alpha_*dt))/alpha_);
                double v2 = v*v;
                double dx = v*QL_SQRT(3);

                unsigned width = jMax - jMin + 1;

                unsigned index = 0;
                vector<signed> k(width);

                for (signed j=jMin; j<=jMax; j++) {
                    double x = j*dx;
                    double m = x*QL_EXP(-alpha_*dt);
                    k[index] = (signed)round(m/dx);
                    double e = m - k[index]*dx;
                    double e2 = e*e;

                    double pUp  = (1.0 + e2/v2 + e*QL_SQRT(3)/v)/6.0;
                    double pMid  = (2.0 - e2/v2)/3.0;
                    double pDown = (1.0 + e2/v2 - e*QL_SQRT(3)/v)/6.0;

                    double rate;
                    if (i>0)
                        rate = QL_EXP(u_[iNow+i-1]+j*dx) + (r - r0);
                    else
                        rate = r;
                    tree.node(i,j).setDiscount(QL_EXP(-rate*dt));
                    tree.node(i,j).setProbability(pUp, 2);
                    tree.node(i,j).setProbability(pMid, 1);
                    tree.node(i,j).setProbability(pDown, 0);

                    index++;
                }
                tree.addLevel(k);
                jMin = k.front() - 1;
                jMax = k.back() + 1;
            }
            cout << "Tree filled " << endl;
            return tree.calcDiscount();

        }


    }
}
