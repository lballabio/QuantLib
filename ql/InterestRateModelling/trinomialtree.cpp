
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

/*! \file trinomialtree.cpp
    \brief Trinomial tree class

    \fullpath
    ql/InterestRateModelling/%trinomialtree.cpp
*/

// $Id$

#include <iostream>

#include "ql/Solvers1D/brent.hpp"
#include "ql/InterestRateModelling/model.hpp"
#include "ql/InterestRateModelling/trinomialtree.hpp"
#include "ql/InterestRateModelling/timefunction.hpp"

namespace QuantLib {

    namespace InterestRateModelling {

        using std::cout;
        using std::endl;

        using Lattices::Node;
        using Lattices::TimeGrid;

/*
        class TrinomialTree::PrivateFunction : public ObjectiveFunction {
          public:
            PrivateFunction(Tree* tree,
                            unsigned i,
                            double discountBond,
                            Handle<TimeFunction>& theta,
                            const Handle<ShortRateProcess>& process)
            : tree_(tree), i_(i), jMin_(tree->jMin(i)), jMax_(tree->jMax(i)),
              dt_(tree->dt(i)), dx_(tree->dx(i)),
              discountBond_(discountBond), theta_(theta), process_(process) {}

            double operator()(double x) const {
                theta_->set(tree_->t(i_), x);
                double value = discountBond_;
                for (int j=jMin_; j<=jMax_; j++)
                    value -= tree_->node(i_,j).statePrice()*
                        QL_EXP(-process_->shortRate(j*dx_)*dt_);
                return value;
            }
          private:
            Tree* tree_;
            unsigned int i_;
            int jMin_, jMax_;
            double dt_, dx_;
            double discountBond_;
            Handle<TimeFunction>& theta_;
            const Handle<ShortRateProcess>& process_;
        };      
            
        TrinomialTree::TrinomialTree(
            const Handle<ShortRateProcess>& process,
            const RelinkableHandle<TermStructure>& termStructure,
            Handle<TimeFunction>& theta,
            Time dtMax,
            const std::list<Time>& times) 
        : Tree(3) {

            Rate r0 = termStructure->forward(0.0);

            setTimePoints(times, dtMax);

            //adjust space intervals
            dx_.resize(t_.size());
            dx_[0] = 0.0; //Just one node
            for (unsigned i=0; i<(dx_.size()-1); i++) {
                //The diffusion term must be r-independant
                double v = process->diffusion(t_[i], 0.0)*QL_SQRT(dt(i));
                dx_[i+1] = v*QL_SQRT(3);
            }


            node(0,0).setStatePrice(1.0);

            int jMin = 0, jMax = 0;
            unsigned nTimeSteps = t_.size() - 1;

            for (unsigned int i=0; i<nTimeSteps; i++) {

                unsigned width = jMax - jMin + 1;

                //calculating u_[i]
                double discountBond = termStructure->discount(t_[i+1]);
                
                PrivateFunction 
                    finder(this, i, r0, discountBond, theta, process);
                Solvers1D::Brent s1d = Solvers1D::Brent();
                double minStrike = -10.0;
                double maxStrike = 1.0;
                s1d.setMaxEvaluations(1000);
                s1d.setLowBound(minStrike);
                s1d.setHiBound(maxStrike);
                double value = 
                    s1d.solve(finder, 1e-8, 0.05, minStrike, maxStrike);
                theta->set(t_[i], value);
                //cout << "Theta(" << t_[i] << ") = " << value << endl;

                //Determine branching
                std::vector<int> k(width);

                double v = dx_[i+1]/QL_SQRT(3);
                double v2 = v*v;

                unsigned int index = 0;
                int j;
                for (j=jMin; j<=jMax; j++) {
                    double x = j*dx_[i];
                    double m = x + process->drift(t_[i], x)*dt(i);
                    k[index] = int(m/dx_[i+1] + 0.5);
                    double e = m - k[index]*dx_[i+1];
                    double e2 = e*e;

                    double pUp  = (1.0 + e2/v2 + e*QL_SQRT(3)/v)/6.0;
                    double pMid  = (2.0 - e2/v2)/3.0;
                    double pDown = (1.0 + e2/v2 - e*QL_SQRT(3)/v)/6.0;

                    node(i,j).setProbability(pUp, 2);
                    node(i,j).setProbability(pMid, 1);
                    node(i,j).setProbability(pDown, 0);

                    Rate r = r0 + process->shortRate(j*dx(i));
                    double discount = QL_EXP(-r*dt(i));
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
                        value += parent.statePrice()
                            *parent.probability(child.ascendantBranch(l))
                            *parent.discount();
                    }
                    child.setStatePrice(value);
                }
            }

        }

*/
        TrinomialTree::TrinomialTree(
            const Handle<ShortRateProcess>& process,
            const TimeGrid& timeGrid)
        : Tree(3) {

            t_ = timeGrid;

            //adjust space intervals
            dx_.resize(t_.size());
            dx_[0] = 0.0; //Just one node
            for (size_t i=0; i<(dx_.size()-1); i++) {
                //The diffusion term must be r-independant
                double v = process->diffusion(t_[i], 0.0)*QL_SQRT(dt(i));
                dx_[i+1] = v*QL_SQRT(3);
            }

            node(0,0).setStatePrice(1.0);

            int jMin = 0, jMax = 0;
            size_t nTimeSteps = t_.size() - 1;

            for (size_t i=0; i<nTimeSteps; i++) {

                //Determine branching
                double v = dx(i+1)/QL_SQRT(3);
                double v2 = v*v;

                std::vector<int> k(0);
                int j;
                for (j=jMin; j<=jMax; j++) {
                    double x = j*dx(i);
                    double m = x + process->drift(t(i), x)*dt(i);
                    k.push_back( (int)floor(m/dx(i+1) + 0.5) );
                    double e = m - k.back()*dx(i+1);
                    double e2 = e*e;

                    double pUp  = (1.0 + e2/v2 + e*QL_SQRT(3)/v)/6.0;
                    double pMid  = (2.0 - e2/v2)/3.0;
                    double pDown = (1.0 + e2/v2 - e*QL_SQRT(3)/v)/6.0;

                    node(i,j).setProbability(pUp, 2);
                    node(i,j).setProbability(pMid, 1);
                    node(i,j).setProbability(pDown, 0);

                    Rate r = process->shortRate(j*dx(i), t(i));
                    double discount = QL_EXP(-r*dt(i));
                    node(i,j).setDiscount(discount);
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
                        value += parent.statePrice()
                            *parent.probability(child.ascendantBranch(l))
                            *parent.discount();
                    }
                    child.setStatePrice(value);
                }
            }

        }

        void TrinomialTree::addLevel(const std::vector<int>& k) {
            unsigned i = nodes_.size();
            nodes_.push_back(std::vector<Handle<Node> >());
            
            int jMin = k.front() - 1;
            int jMax = k.back() + 1;

            for (int j=jMin; j<=jMax; j++) {
                nodes_[i].push_back(Handle<Node>(new Node(3, i, j)));
            }

            QL_REQUIRE(k.size()==nodes_[i-1].size(), "Error!!!");
            for (unsigned l=0; l<k.size(); l++) {
                nodes_[i-1][l]->setDescendant(node(i, k[l] - 1), 0);
                nodes_[i-1][l]->setDescendant(node(i, k[l]    ), 1);
                nodes_[i-1][l]->setDescendant(node(i, k[l] + 1), 2);
            }
        }


    }

}

