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
/*! \file timefunction.cpp
    \brief Time function used for term structure fitting

    \fullpath
    ql/InterestRateModelling/%timefunction.cpp
*/

// $Id$

#include "ql/InterestRateModelling/timefunction.hpp"
/*#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/FiniteDifferences/expliciteuler.hpp"
#include "ql/FiniteDifferences/onefactoroperator.hpp"
#include "ql/Solvers1D/brent.hpp"
*/
namespace QuantLib {

    namespace InterestRateModelling {
/*
        using namespace FiniteDifferences;

        typedef FiniteDifferenceModel<ExplicitEuler<TridiagonalOperator> >
            CustomFiniteDifferenceModel;

        class TimeFunction::FitFunction : public ObjectiveFunction {
          public:
            FitFunction(
                double discountBond,
                const Array& statePrices,
                std::vector<double>& values,
                CustomFiniteDifferenceModel& fd,
                double dt, Size nit, const Grid& grid)
            : discountBond_(discountBond), statePrices_(statePrices),
              values_(values), fd_(fd), nit_(nit), grid_(grid) {
                from_ = (nit_ - 1)*dt;
                to_ = nit_*dt;
            }
            double operator()(double x) const {
                Size index = grid_.index();
                values_.back() = x;

                Array prices(statePrices_);
                fd_.rollback(prices, from_, to_, 1);

                double value = discountBond_;
                for (Size k=(index-nit_-1); k<=(index+nit_+1); k++) {
                    std::cout << k << "," << x <<  " --> " << prices[k] << std::endl;
                    value -= prices[k];
                }
                return value;
            }
          private:
            double discountBond_;
            const Array& statePrices_;
            std::vector<double>& values_;
            CustomFiniteDifferenceModel& fd_;
            Size nit_;
            const Grid& grid_;
            double from_;
            double to_;
        };

        void TimeFunction::fitToTermStructure(
            const OneFactorModel& model, Size timeSteps) {

            reset();
            double dt = model.termStructure()->maxTime()/timeSteps;

            Grid grid(timeSteps, 0.0, 0.0, timeSteps*dt, dt, model);

            Array statePrices(grid.size(), 0.0);
            statePrices[grid.index()] = 1.0;

            OneFactorOperator op(grid, model.process());
            op.setLowerBC( BoundaryCondition(BoundaryCondition::Neumann, 0));
            op.setUpperBC( BoundaryCondition(BoundaryCondition::Neumann, 0));
            CustomFiniteDifferenceModel finiteDifferenceModel(op);

            Solvers1D::Brent s1d = Solvers1D::Brent();
            double minStrike = -0.10;
            double maxStrike = 0.10;
            s1d.setMaxEvaluations(1000);

            for (Size i=0; i<timeSteps; i++) {
                Time from = i*dt;
                Time to = (i+1)*dt;

                times_.push_back(from);
                values_.push_back(0.0);

                double discountBond = model.termStructure()->discount(to);
                FitFunction finder(discountBond, statePrices, values_,
                    finiteDifferenceModel, dt, i+1, grid);
                double accuracy = 1e-10;
                double initialValue = 0.05;
                double value = s1d.solve(finder, accuracy, initialValue,
                    minStrike, maxStrike);
                values_.back() = value;
                finiteDifferenceModel.rollback(statePrices, from, to, 1);
            }

        }
*/
    }

}
