
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#include "tridiagonaloperator.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Operators)

QL_USING(PDE,BoundaryCondition)

void TridiagonalOperatorCommon::setLowerBC(const BoundaryCondition& bc) {
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

void TridiagonalOperatorCommon::setHigherBC(const BoundaryCondition& bc) {
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


Array<double> TridiagonalOperatorCommon::applyTo(const Array<double>& v) const {
	Require(v.size()==theSize,"TridiagonalOperator::applyTo: vector of the wrong size");
	Array<double> result(theSize);

	// matricial product
	result[0] = diagonal[0]*v[0] + aboveDiagonal[0]*v[1];
	for (int j=1;j<=theSize-2;j++)
		result[j] = belowDiagonal[j-1]*v[j-1]+ diagonal[j]*v[j] + aboveDiagonal[j]*v[j+1];
	result[theSize-1] = belowDiagonal[theSize-2]*v[theSize-2]+ diagonal[theSize-1]*v[theSize-1]; 

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

Array<double> TridiagonalOperatorCommon::solveFor(const Array<double>& rhs) const {
	Require(rhs.size()==theSize,"TridiagonalOperator::solveFor: rhs vector has the wrong size");
	Array<double> bcRhs = rhs;

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
	Array<double> result(theSize), tmp(theSize);

	double bet=diagonal[0];
	Require(bet != 0.0, "TridiagonalOperator::solveFor: division by zero"); 
	result[0] = bcRhs[0]/bet;
	for(int j=1;j<=theSize-1;j++){
		tmp[j]=aboveDiagonal[j-1]/bet;
		bet=diagonal[j]-belowDiagonal[j-1]*tmp[j];
		Require(bet != 0.0, "TridiagonalOperator::solveFor: division by zero"); 
		result[j] = (bcRhs[j]-belowDiagonal[j-1]*result[j-1])/bet;
	}
	for(j=theSize-2;j>=0;j--) 
		result[j] -= tmp[j+1]*result[j+1];

	return result;
}


QL_END_NAMESPACE(Operators)

QL_END_NAMESPACE(QuantLib)
