
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_tridiagonal_operator_h
#define quantlib_tridiagonal_operator_h

#include "qldefines.h"
#include "operator.h"
#include "identity.h"
#include "boundarycondition.h"
#include "array.h"

QL_BEGIN_NAMESPACE(QuantLib)

QL_BEGIN_NAMESPACE(Operators)

class TridiagonalOperatorCommon {
  public:
	// constructors
	TridiagonalOperatorCommon() : theSize(0) {}
	TridiagonalOperatorCommon(int size);
	TridiagonalOperatorCommon(const Array& low, const Array& mid, const Array& high);
	// operator interface
	Array solveFor(const Array& rhs) const;
	Array applyTo(const Array& v) const;
	// inspectors
	int size() const { return theSize; }
	// modifiers
	void setLowerBC(const QL_ADD_NAMESPACE(PDE,BoundaryCondition)& bc);
	void setHigherBC(const QL_ADD_NAMESPACE(PDE,BoundaryCondition)& bc);
	void setFirstRow(double, double);
	void setMidRow(int, double, double, double);
	void setMidRows(double, double, double);
	void setLastRow(double, double);
  protected:
	Array diagonal, belowDiagonal, aboveDiagonal;
	QL_ADD_NAMESPACE(PDE,BoundaryCondition) theLowerBC, theHigherBC;
  private:
	int theSize;
};

// derived classes

// time-constant

class TridiagonalOperator : public TridiagonalOperatorCommon, public QL_ADD_NAMESPACE(PDE,TimeConstantOperator) {
	friend TridiagonalOperator operator+(const TridiagonalOperator&);
	friend TridiagonalOperator operator-(const TridiagonalOperator&);
	friend TridiagonalOperator operator*(double, const TridiagonalOperator&);
	friend TridiagonalOperator operator*(const TridiagonalOperator&, double);
	friend TridiagonalOperator operator+(const TridiagonalOperator&, const TridiagonalOperator&);
	friend TridiagonalOperator operator-(const TridiagonalOperator&, const TridiagonalOperator&);
	friend TridiagonalOperator operator+(const QL_ADD_NAMESPACE(PDE,Identity)<Array>&, const TridiagonalOperator&);
	friend TridiagonalOperator operator+(const TridiagonalOperator&, const QL_ADD_NAMESPACE(PDE,Identity)<Array>&);
	friend TridiagonalOperator operator-(const QL_ADD_NAMESPACE(PDE,Identity)<Array>&, const TridiagonalOperator&);
	friend TridiagonalOperator operator-(const TridiagonalOperator&, const QL_ADD_NAMESPACE(PDE,Identity)<Array>&);
  public:
	// constructors
	TridiagonalOperator() : TridiagonalOperatorCommon() {}
	TridiagonalOperator(int size) : TridiagonalOperatorCommon(size) {}
	TridiagonalOperator(const Array& low, const Array& mid, const Array& high)
	: TridiagonalOperatorCommon(low,mid,high) {}
};


// time-dependent

class TimeDependentTridiagonalOperator : public TridiagonalOperatorCommon, public QL_ADD_NAMESPACE(PDE,TimeDependentOperator) {
  public:
	// constructors
	TimeDependentTridiagonalOperator() : TridiagonalOperatorCommon() {}
	TimeDependentTridiagonalOperator(int size) : TridiagonalOperatorCommon(size) {}
	TimeDependentTridiagonalOperator(const Array& low, const Array& mid, const Array& high)
	: TridiagonalOperatorCommon(low,mid,high) {}
};


// inline definitions

inline TridiagonalOperatorCommon::TridiagonalOperatorCommon(int size) 
: theSize(size) {
	Require(theSize >= 3, "invalid size for tridiagonal operator (must be >= 3)");
	belowDiagonal = Array(theSize-1);
	diagonal = Array(theSize);
	aboveDiagonal = Array(theSize-1);
}

inline TridiagonalOperatorCommon::TridiagonalOperatorCommon(const Array& low, const Array& mid, const Array& high)
: diagonal(mid), belowDiagonal(low), aboveDiagonal(high), theSize(mid.size()) {
	Require(belowDiagonal.size() == theSize-1, "wrong size for lower diagonal vector");
	Require(aboveDiagonal.size() == theSize-1, "wrong size for upper diagonal vector");
}

inline void TridiagonalOperatorCommon::setFirstRow(double valB, double valC){
	diagonal[0]      = valB;
	aboveDiagonal[0] = valC;
}

inline void TridiagonalOperatorCommon::setMidRow(int i, double valA, double valB, double valC){
	Require(i>=1 && i<=theSize-2, "out of range in TridiagonalSystem::setMidRow");
	belowDiagonal[i-1] = valA;
	diagonal[i]        = valB;
	aboveDiagonal[i]   = valC;
}

inline void TridiagonalOperatorCommon::setMidRows(double valA, double valB, double valC){
	for(int i=1; i<=theSize-2; i++){
		belowDiagonal[i-1] = valA;
		diagonal[i]        = valB;
		aboveDiagonal[i]   = valC;
	}
}

inline void TridiagonalOperatorCommon::setLastRow(double valA, double valB){	
	belowDiagonal[theSize-2] = valA;
	diagonal[theSize-1]      = valB;
}

// time-constant algebra

inline TridiagonalOperator operator+(const TridiagonalOperator& D) {
	return D;
}

inline TridiagonalOperator operator-(const TridiagonalOperator& D) {
	TridiagonalOperator result(-D.belowDiagonal,-D.diagonal,-D.aboveDiagonal);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}

inline TridiagonalOperator operator*(double a, const TridiagonalOperator& D) {
	TridiagonalOperator result(D.belowDiagonal*a,D.diagonal*a,D.aboveDiagonal*a);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}

inline TridiagonalOperator operator*(const TridiagonalOperator& D, double a) {
	TridiagonalOperator result(D.belowDiagonal*a,D.diagonal*a,D.aboveDiagonal*a);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}

inline TridiagonalOperator operator+(const TridiagonalOperator& D1, const TridiagonalOperator& D2) {
	// to do: check boundary conditions
	return TridiagonalOperator(D1.belowDiagonal+D2.belowDiagonal,D1.diagonal+D2.diagonal,D1.aboveDiagonal+D2.aboveDiagonal);
}

inline TridiagonalOperator operator-(const TridiagonalOperator& D1, const TridiagonalOperator& D2) {
	// to do: check boundary conditions
	return TridiagonalOperator(D1.belowDiagonal-D2.belowDiagonal,D1.diagonal-D2.diagonal,D1.aboveDiagonal-D2.aboveDiagonal);
}

inline TridiagonalOperator operator+(const TridiagonalOperator& D, const QL_ADD_NAMESPACE(PDE,Identity)<Array>& I) {
	Array temp(D.size(),1.0);
	TridiagonalOperator result(D.belowDiagonal,D.diagonal+temp,D.aboveDiagonal);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}

inline TridiagonalOperator operator+(const QL_ADD_NAMESPACE(PDE,Identity)<Array>& I, const TridiagonalOperator& D) {
	Array temp(D.size(),1.0);
	TridiagonalOperator result(D.belowDiagonal,D.diagonal+temp,D.aboveDiagonal);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}

inline TridiagonalOperator operator-(const TridiagonalOperator& D, const QL_ADD_NAMESPACE(PDE,Identity)<Array>& I) {
	Array temp(D.size(),1.0);
	TridiagonalOperator result(D.belowDiagonal,D.diagonal-temp,D.aboveDiagonal);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}

inline TridiagonalOperator operator-(const QL_ADD_NAMESPACE(PDE,Identity)<Array>& I, const TridiagonalOperator& D) {
	Array temp(D.size(),1.0);
	TridiagonalOperator result(-D.belowDiagonal,temp-D.diagonal,-D.aboveDiagonal);
	result.setLowerBC(D.theLowerBC);
	result.setHigherBC(D.theHigherBC);
	return result;
}


QL_END_NAMESPACE(Operators)

QL_END_NAMESPACE(QuantLib)


#endif
