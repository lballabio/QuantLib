
/*
 * Copyright (C) 2000
 * Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
 * 
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.sourceforge.net/
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
 * if not, contact ferdinando@ametrano.net
 *
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file tridiagonaloperator.cpp
    \brief tridiagonal operator

    $Source$
    $Name$
    $Log$
    Revision 1.12  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.11  2001/04/04 11:07:24  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.10  2001/02/26 17:00:07  lballabio
    Moved constructor to .cpp - inlined constructors raising exceptions are bad medicine

    Revision 1.9  2001/02/21 11:31:04  lballabio
    Removed redundant theSize data member

    Revision 1.8  2001/01/08 11:44:18  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.7  2001/01/08 10:28:17  lballabio
    Moved Array to Math namespace

    Revision 1.6  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros
    
    Revision 1.5  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#include "FiniteDifferences/tridiagonaloperator.hpp"
#include "dataformatters.hpp"
#include "qlerrors.hpp"

namespace QuantLib {

    namespace FiniteDifferences {

        TridiagonalOperatorCommon::TridiagonalOperatorCommon(int size)
        : diagonal(size), belowDiagonal(size-1), aboveDiagonal(size-1) {
            QL_ENSURE(diagonal.size() >= 3 || diagonal.size() == 0, 
                "invalid size for tridiagonal operator (must be >= 3)");
        }

        TridiagonalOperatorCommon::TridiagonalOperatorCommon(
            const Array& low, const Array& mid, const Array& high)
        : diagonal(mid), belowDiagonal(low), aboveDiagonal(high) {
            QL_ENSURE(low.size() == mid.size()-1, 
                "wrong size for lower diagonal vector");
            QL_ENSURE(high.size() == mid.size()-1, 
                "wrong size for upper diagonal vector");
        }

        void TridiagonalOperatorCommon::setLowerBC(
          const BoundaryCondition& bc) {
            theLowerBC = bc;
            switch (theLowerBC.type()) {
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
        
        void TridiagonalOperatorCommon::setHigherBC(
          const BoundaryCondition& bc) {
            theHigherBC = bc;
            switch (theHigherBC.type()) {
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
        TridiagonalOperatorCommon::applyTo(const Array& v) const {
            QL_REQUIRE(v.size()==size(),
                "TridiagonalOperator::applyTo: vector of the wrong size (" +
                IntegerFormatter::toString(v.size()) + "instead of " + 
                IntegerFormatter::toString(size()) + ")"  );
            Array result(size());
        
            // matricial product
            result[0] = diagonal[0]*v[0] + aboveDiagonal[0]*v[1];
            for (int j=1;j<=size()-2;j++)
                result[j] = belowDiagonal[j-1]*v[j-1]+ diagonal[j]*v[j] +
                    aboveDiagonal[j]*v[j+1];
            result[size()-1] = belowDiagonal[size()-2]*v[size()-2] +
                diagonal[size()-1]*v[size()-1]; 
        
            // apply lower boundary condition
            switch (theLowerBC.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                result[0] = result[1] + theLowerBC.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[0] = theLowerBC.value();
                break;
            }
            
            // apply higher boundary condition
            switch (theHigherBC.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
                result[size()-1] = result[size()-2] + theHigherBC.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[size()-1] = theHigherBC.value();
                break;
            }
        
            return result;
        }
        
        Array 
        TridiagonalOperatorCommon::solveFor(const Array& rhs) const {
            QL_REQUIRE(rhs.size()==size(),
                "TridiagonalOperator::solveFor: rhs vector has the wrong size");
            Array bcRhs = rhs;
        
            // apply lower boundary condition
            switch (theLowerBC.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                bcRhs[0] = theLowerBC.value();
                break;
            }
            
            // apply higher boundary condition
            switch (theHigherBC.type()) {
              case BoundaryCondition::None:
                // does nothing
                break;
              case BoundaryCondition::Neumann:
              case BoundaryCondition::Dirichlet:
                bcRhs[size()-1] = theHigherBC.value();
                break;
            }
        
            // solve tridiagonal system
            Array result(size()), tmp(size());
        
            double bet=diagonal[0];
            QL_REQUIRE(bet != 0.0, 
                "TridiagonalOperator::solveFor: division by zero"); 
            result[0] = bcRhs[0]/bet;
            int j;
            for (j=1;j<=size()-1;j++){
                tmp[j]=aboveDiagonal[j-1]/bet;
                bet=diagonal[j]-belowDiagonal[j-1]*tmp[j];
                QL_ENSURE(bet != 0.0, 
                    "TridiagonalOperator::solveFor: division by zero"); 
                result[j] = (bcRhs[j]-belowDiagonal[j-1]*result[j-1])/bet;
            }
            for (j=size()-2;j>=0;j--) 
                result[j] -= tmp[j+1]*result[j+1];
        
            return result;
        }
    
    }

}
