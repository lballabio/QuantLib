
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

/*! \file onefactormodel.cpp
    \brief Abstract one-factor interest rate model class

    \fullpath
    ql/InterestRateModelling/%onefactormodel.cpp
*/

// $Id$

#include "ql/InterestRateModelling/grid.hpp"
#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/FiniteDifferences/expliciteuler.hpp"
#include "ql/FiniteDifferences/onefactoroperator.hpp"
#include "ql/Solvers1D/brent.hpp"
using std::cout;
using std::endl;

namespace QuantLib {

    namespace InterestRateModelling {

        using namespace FiniteDifferences;

        typedef FiniteDifferenceModel<ExplicitEuler<TridiagonalOperator> > 
            CustomFiniteDifferenceModel;

        class OneFactorModel::FitFunction : public ObjectiveFunction {
          public:
            FitFunction(const RelinkableHandle<TermStructure>& termStructure,
                const Array& statePrices,
                std::vector<double>& theta, CustomFiniteDifferenceModel& fd, 
                double dt, unsigned nit, const Grid& grid)
            : termStructure_(termStructure), statePrices_(statePrices), 
              theta_(theta), fd_(fd), nit_(nit), grid_(grid) {
                from_ = (nit_ - 1)*dt;
                to_ = nit_*dt;
            }
            double operator()(double x) const;
          private:
            const RelinkableHandle<TermStructure>& termStructure_;
            const Array& statePrices_;
            std::vector<double>& theta_;
            CustomFiniteDifferenceModel& fd_;
            unsigned nit_;
            const Grid& grid_;
            double from_;
            double to_;
        };      
            
        inline double OneFactorModel::FitFunction::operator()(double x) const {
            unsigned int index = grid_.index();
            theta_[nit_ - 1] = x;

            Array prices(statePrices_);
            fd_.rollback(prices, from_, to_, 1);

            double value = termStructure_->discount(to_);
            for (unsigned k=(index-nit_-1); k<=(index+nit_+1); k++) {
                cout << k << "," << x <<  " --> " << prices[k] << endl;
                value -= prices[k];
            }
            return value;
        }

        void OneFactorModel::fitToTermStructure(std::vector<double>& theta_) {
            unsigned timeSteps_ = theta_.size() - 1;
            unsigned gridPoints = timeSteps_;
            double dt_ = termStructure()->maxTime()/timeSteps_;

            Rate r0 = termStructure()->forward(0.0);
            double center = stateVariable(r0);
            Grid grid(gridPoints, center, center, timeSteps_*dt_, dt_, this);

            Array statePrices(grid.size(), 0.0);
            statePrices[grid.index()] = 1.0;

            OneFactorOperator op(grid, process());
            op.setLowerBC( BoundaryCondition(BoundaryCondition::Neumann, 0));
            op.setUpperBC( BoundaryCondition(BoundaryCondition::Neumann, 0));
            CustomFiniteDifferenceModel finiteDifferenceModel(op);

            Solvers1D::Brent s1d = Solvers1D::Brent();
            double minStrike = -0.02;
            double maxStrike = 0.0;
            s1d.setMaxEvaluations(1000);
            s1d.setLowBound(minStrike);
            s1d.setHiBound(maxStrike);

            double alpha_ = -0.2994;
            double sigma_ = 0.0087;

            for (unsigned i=0; i<timeSteps_; i++) {
                FitFunction finder(termStructure(), statePrices, theta_, 
                    finiteDifferenceModel, dt_, i+1, grid);

                double forwardRate = termStructure()->forward(i*dt_);
                double theta =  alpha_*forwardRate + sigma_*sigma_*
                    (1.0-QL_EXP(-2.0*alpha_*i*dt_))/(2.0*alpha_);

                double accuracy = 1e-10;
                double initialValue = 0.05;
                theta_[i] = s1d.solve(finder, accuracy, initialValue,
                    minStrike, maxStrike);
                finiteDifferenceModel.
                    rollback(statePrices, i*dt_, (i+1)*dt_, 1);
                cout << i << " ---> " << theta_[i] << " =? " << theta << endl;
            }
            theta_[timeSteps_] = theta_[timeSteps_ - 1];
        }

    }

}
