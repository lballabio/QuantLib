
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

/*! \file fdeuropeanswaption.cpp
    \brief European swaption calculated using finite differences

    \fullpath
    ql/Pricers/%fdeuropeanswaption.cpp
*/

// $Id$

#include "ql/FiniteDifferences/fdtypedefs.hpp"
#include "ql/Pricers/fdeuropeanswaption.hpp"
#include "ql/FiniteDifferences/boundarycondition.hpp"
#include "ql/InterestRateModelling/grid.hpp"
#include "ql/InterestRateModelling/swapfuturevalue.hpp"

namespace QuantLib {

    namespace Pricers {

        using FiniteDifferences::BoundaryCondition;
        using FiniteDifferences::OneFactorOperator;
        using Instruments::SimpleSwap;
        using namespace InterestRateModelling;

        FDEuropeanSwaption::FDEuropeanSwaption(
            const Handle<SimpleSwap>& swap,
            Time maturity,
            const Handle<Model>& model)
        : swap_(swap), maturity_(maturity), model_(model) {}

        double FDEuropeanSwaption::value(Rate rate,
          size_t timeSteps, unsigned int gridPoints) {

            Handle<OneFactorModel> model = model_;
            QL_REQUIRE(!model.isNull(),
                "This pricer requires a single-factor model!");
            double dt = maturity_/timeSteps;
            double initialCenter = model->stateVariable(rate);
            Grid grid(gridPoints, initialCenter, initialCenter,
                maturity_, dt, model);

            size_t size = grid.size();
            std::vector<double> rateGrid(size);
            for (unsigned i = 0; i < size; i++)
                rateGrid[i] = model->getRateFrom(grid[i]);

            OneFactorOperator op(grid, model->process());
            op.setLowerBC( BoundaryCondition(BoundaryCondition::Neumann,
                grid[1] - grid[0]));
            op.setUpperBC( BoundaryCondition(BoundaryCondition::Neumann,
                grid[size-1] - grid[size-2]));
            FiniteDifferences::StandardFiniteDifferenceModel
                                finiteDifferenceModel(op);

            Array prices(size);
            for(unsigned j = 0; j < size; j++) {
                prices[j] = QL_MAX(swapFutureValue(swap_, model_,
                    rateGrid[j], maturity_), 0.0);
            }

            finiteDifferenceModel.rollback(prices, maturity_, 0.0,
                timeSteps);
            return prices[grid.index()];
        }

    }

}
