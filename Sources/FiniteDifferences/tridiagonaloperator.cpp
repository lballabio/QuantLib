
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
 * QuantLib license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file tridiagonaloperator.cpp
    \brief tridiagonal operator

    $Source$
    $Name$
    $Log$
    Revision 1.8  2001/01/08 11:44:18  lballabio
    Array back into QuantLib namespace - Math namespace broke expression templates, go figure

    Revision 1.7  2001/01/08 10:28:17  lballabio
    Moved Array to Math namespace

    Revision 1.6  2000/12/27 14:05:57  lballabio
    Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros
    
    Revision 1.5  2000/12/14 12:32:31  lballabio
    Added CVS tags in Doxygen file documentation blocks
    
*/

#include "tridiagonaloperator.h"
#include "dataformatters.h"
#include "qlerrors.h"

namespace QuantLib {

    namespace FiniteDifferences {
    
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
            QL_REQUIRE(v.size()==theSize,
                "TridiagonalOperator::applyTo: vector of the wrong size (" +
                IntegerFormatter::toString(v.size()) + "instead of " + 
                IntegerFormatter::toString(theSize) + ")"  );
            Array result(theSize);
        
            // matricial product
            result[0] = diagonal[0]*v[0] + aboveDiagonal[0]*v[1];
            for (int j=1;j<=theSize-2;j++)
                result[j] = belowDiagonal[j-1]*v[j-1]+ diagonal[j]*v[j] +
                    aboveDiagonal[j]*v[j+1];
            result[theSize-1] = belowDiagonal[theSize-2]*v[theSize-2] +
                diagonal[theSize-1]*v[theSize-1]; 
        
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
                result[theSize-1] = result[theSize-2] + theHigherBC.value();
                break;
              case BoundaryCondition::Dirichlet:
                result[theSize-1] = theHigherBC.value();
                break;
            }
        
            return result;
        }
        
        Array 
        TridiagonalOperatorCommon::solveFor(const Array& rhs) const {
            QL_REQUIRE(rhs.size()==theSize,
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
                bcRhs[theSize-1] = theHigherBC.value();
                break;
            }
        
            // solve tridiagonal system
            Array result(theSize), tmp(theSize);
        
            double bet=diagonal[0];
            QL_REQUIRE(bet != 0.0, 
                "TridiagonalOperator::solveFor: division by zero"); 
            result[0] = bcRhs[0]/bet;
            int j;
            for (j=1;j<=theSize-1;j++){
                tmp[j]=aboveDiagonal[j-1]/bet;
                bet=diagonal[j]-belowDiagonal[j-1]*tmp[j];
                QL_ENSURE(bet != 0.0, 
                    "TridiagonalOperator::solveFor: division by zero"); 
                result[j] = (bcRhs[j]-belowDiagonal[j-1]*result[j-1])/bet;
            }
            for (j=theSize-2;j>=0;j--) 
                result[j] -= tmp[j+1]*result[j+1];
        
            return result;
        }
    
    }

}
