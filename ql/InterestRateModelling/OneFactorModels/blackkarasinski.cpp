
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
    ql/InterestRateModelling/OneFactorModels/%blackkarasinski.cpp
*/

// $Id$

#include <cmath>
#include "ql/InterestRateModelling/OneFactorModels/blackkarasinski.hpp"
#include "ql/InterestRateModelling/trinomialtree.hpp"
#include "ql/Solvers1D/brent.hpp"

using std::vector;
using std::cout;
using std::endl;

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

            virtual double variable(Rate r, Time t) const { 
                return QL_LOG(r) - model_->alpha(t); 
            }
            virtual double shortRate(double x, Time t) const { 
                return QL_EXP(model_->alpha(t) + x); 
            }

            virtual double drift(double lnR, double t) const {
                return -a_*lnR;
            }
            virtual double diffusion(double lnR, Time t) const {
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
                int jMax, const vector<double>& statePrices, 
                double discountBondPrice) 
            : dt_(dt), dx_(dx), jMin_(jMin), jMax_(jMax), 
              statePrices_(statePrices), 
              discountBondPrice_(discountBondPrice) {}

            double operator()(double x) const {
                double value = discountBondPrice_;
                unsigned k=0;
                for (int j=jMin_; j<=jMax_; j++)
                    value -= statePrices_[k++]*QL_EXP(-QL_EXP(x + j*dx_)*dt_);
                return value;
            }
     
          private:
            double dt_, dx_;
            int jMin_, jMax_;
            const vector<double>& statePrices_;
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
                unsigned i;
                for (i=0; i<(dx_.size()-1); i++) {
                    //The diffusion term must be r-independant
                    double v = sigma*
                        QL_SQRT(0.5*(1.0 - QL_EXP(-2.0*a*dt(i)))/a);
                    dx_[i+1] = v*QL_SQRT(3);
                    //cout << "dx: " << i+1 << " " << v << " " << dt(i) << endl;
                }

                node(0,0).setStatePrice(1.0);

                int jMin = 0, jMax = 0;
                unsigned int nTimeSteps = t_.size() - 1;

                alpha->clear();

                double lastValue = 0.0;
                for (i=0; i<nTimeSteps; i++) {
                    unsigned width = jMax - jMin + 1;

                    double discountBond = termStructure->discount(t(i+1));
                    vector<double> statePrices(width);
                    unsigned index=0;
                    int j;
                    for (j=jMin; j<=jMax; j++)
                        statePrices[index++] = node(i,j).statePrice();
                    BlackKarasinski::PrivateFunction finder(dt(i), dx(i), jMin, 
                        jMax, statePrices, discountBond);
                    // solver
                    Solvers1D::Brent s1d = Solvers1D::Brent();
                    double minStrike = -10.0;
                    double maxStrike = 1.0;
                    s1d.setMaxEvaluations(1000);
                    s1d.setLowBound(minStrike);
                    s1d.setHiBound(maxStrike);
                    lastValue = s1d.solve(finder, 1e-6, lastValue, 
                        minStrike, maxStrike);
                    alpha->set(t(i), lastValue);

                    vector<int> k(width);

                    double v = dx(i+1)/QL_SQRT(3);
                    double v2 = v*v;

                    index = 0;
                    for (j=jMin; j<=jMax; j++) {
                        double x = j*dx(i);
                        double m = x*QL_EXP(-a*dt(i));
                        k[index] = (int)floor(m/dx(i+1) + 0.5);
                        double e = m - k[index]*dx(i+1);
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

                        index++;
                    }
                    addLevel(k);
                    jMin = k.front() - 1;
                    jMax = k.back() + 1;
                    for (j=jMin; j<=jMax; j++) {
                        double value = 0.0;
                        Node& child = node(i+1,j);
                        unsigned nAscendants = child.nAscendants();
                        for (unsigned l=0; l<nAscendants; l++) {
                            const Node& parent = child.ascendant(l);
                            value += parent.statePrice()*
                                parent.probability(child.ascendantBranch(l))*
                                parent.discount();
                        }
                        child.setStatePrice(value);
                    }
                }
                
                cout << "Tree filled " << endl;
            }
        };


        BlackKarasinski::BlackKarasinski(
            const RelinkableHandle<TermStructure>& termStructure, 
            double dtMax) 
        : OneFactorModel(2, termStructure), alpha_(new TimeFunction()), 
          a_(params_[0]), sigma_(params_[1]), dtMax_(dtMax) {

            process_ = Handle<ShortRateProcess>(new Process(this));

            constraint_ = Handle<Constraint>(new Constraint(2));
            constraint_->setLowerBound(1, 0.000001);
        }

        Handle<Tree> BlackKarasinski::tree(
            const TimeGrid& timeGrid) const {
            cout << "Reconfiguring for sigma = " << sigma_*100.0 << "%" << endl;
            return Handle<Tree>(new PrivateTree(a_, sigma_, 
                                     termStructure(), alpha_, timeGrid));
        }

    }

}
