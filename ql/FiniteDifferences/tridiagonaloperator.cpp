
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

/*! \file tridiagonaloperator.cpp
    \brief tridiagonal operator

    \fullpath
    ql/FiniteDifferences/%tridiagonaloperator.cpp
*/

// $Id$

#include "ql/FiniteDifferences/tridiagonaloperator.hpp"
#include "ql/dataformatters.hpp"
#include "ql/errors.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        TridiagonalOperator::TridiagonalOperator(
            unsigned int size) {

            if (size>=3) {
                diagonal_      = Array(size);
                belowDiagonal_ = Array(size-1);
                aboveDiagonal_ = Array(size-1);
            } else if (size==0) {
                diagonal_      = Array(0);
                belowDiagonal_ = Array(0);
                aboveDiagonal_ = Array(0);
            } else {
                throw Error("invalid size for tridiagonal operator (must be >= 3)");
            }
        }

        TridiagonalOperator::TridiagonalOperator(
            const Array& low, const Array& mid, const Array& high)
        : diagonal_(mid), belowDiagonal_(low), aboveDiagonal_(high) {
            QL_ENSURE(low.size() == mid.size()-1,
                "wrong size for lower diagonal vector");
            QL_ENSURE(high.size() == mid.size()-1,
                "wrong size for upper diagonal vector");
        }

        void TridiagonalOperator::setLowerBC(
          const BoundaryCondition& bc) {
            lowerBC_ = bc;
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                setFirstRow(-1.0,1.0);
                break;
              case BoundaryCondition::Dirichlet:
                setFirstRow(1.0,0.0);
                break;
            }
        }

        void TridiagonalOperator::setUpperBC(
          const BoundaryCondition& bc) {
            upperBC_ = bc;
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                setLastRow(-1.0,1.0);
                break;
              case BoundaryCondition::Dirichlet:
                setLastRow(0.0,1.0);
                break;
            }
        }


        Array
        TridiagonalOperator::applyTo(const Array& v) const {
            QL_REQUIRE(v.size()==size(),
                "TridiagonalOperator::applyTo: vector of the wrong size (" +
                IntegerFormatter::toString(v.size()) + "instead of " +
                IntegerFormatter::toString(size()) + ")"  );
            Array result(size());

            // matricial product
            result[0] = diagonal_[0]*v[0] + aboveDiagonal_[0]*v[1];
            for (unsigned int j=1;j<=size()-2;j++)
                result[j] = belowDiagonal_[j-1]*v[j-1]+ diagonal_[j]*v[j] +
                    aboveDiagonal_[j]*v[j+1];
            result[size()-1] = belowDiagonal_[size()-2]*v[size()-2] +
                diagonal_[size()-1]*v[size()-1];

            // apply lower boundary condition
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                result[0] = result[1] - lowerBC_.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[0] = lowerBC_.value();
                break;
            }

            // apply upper boundary condition
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                result[size()-1] = result[size()-2] + upperBC_.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[size()-1] = upperBC_.value();
                break;
            }

            return result;
        }

        Array
        TridiagonalOperator::solveFor(const Array& rhs) const {
            QL_REQUIRE(rhs.size()==size(),
                "TridiagonalOperator::solveFor: rhs vector has the wrong size");
            Array bcRhs = rhs;

            // apply lower boundary condition
            switch (lowerBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                bcRhs[0] = lowerBC_.value();
                break;
            }

            // apply upper boundary condition
            switch (upperBC_.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                bcRhs[size()-1] = upperBC_.value();
                break;
            }

            // solve tridiagonal system
            Array result(size()), tmp(size());

            double bet=diagonal_[0];
            QL_REQUIRE(bet != 0.0,
                "TridiagonalOperator::solveFor: division by zero");
            result[0] = bcRhs[0]/bet;
            unsigned int j;
            for (j=1;j<=size()-1;j++){
                tmp[j]=aboveDiagonal_[j-1]/bet;
                bet=diagonal_[j]-belowDiagonal_[j-1]*tmp[j];
                QL_ENSURE(bet != 0.0,
                    "TridiagonalOperator::solveFor: division by zero");
                result[j] = (bcRhs[j]-belowDiagonal_[j-1]*result[j-1])/bet;
            }
// cannot be j>=0 with unsigned int j
            for (j=size()-2;j>0;j--)
                result[j] -= tmp[j+1]*result[j+1];
            result[0] -= tmp[1]*result[1];

            return result;
        }

    }

}
