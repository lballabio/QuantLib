
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_array_h
#define quantlib_array_h

#include "qldefines.h"
#include "expressiontemplates.h"
#include <algorithm>
#include <functional>
#include <numeric>

/* The concepts implemented here with regard to vector algebra are from 
   T. L. Veldhuizen, "Expression templates", C++ Report, 7(5):26-31, June 1995
   http://extreme.indiana.edu/~tveldhui/papers/
*/

QL_BEGIN_NAMESPACE(QuantLib)

class Array {
  public:
	// constructors
	explicit Array(int size = 0);
	Array(int size, double value);
	Array(int size, double value, double increment);
	Array(const Array& from)
	  : pointer(0), n(0), bufferSize(0) { allocate(from.size()); copy(from); }
	template <class Iter> Array(VectorialExpression<Iter>& e)
	  : pointer(0), n(0), bufferSize(0) { allocate(e.size()); copy(e); }
	// destructor
	~Array();
	// assignment
	Array& operator=(const Array& from) {
		if (this != &from) { resize(from.size()); copy(from); } 
		return *this;
	}
	template <class Iter> Array& operator=(VectorialExpression<Iter>& e) {
		resize(e.size()); copy(e); return *this;
	}
	// computed assignment
	Array& operator+=(const Array&);
	Array& operator+=(double);
	template <class Iter> Array& operator+=(VectorialExpression<Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "adding arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i += *e; ++i; ++e; }
		return *this;
	}
	Array& operator-=(const Array&);
	Array& operator-=(double);
	template <class Iter> Array& operator-=(VectorialExpression<Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "subtracting arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i -= *e; ++i; ++e; }
		return *this;
	}
	Array& operator*=(const Array&);
	Array& operator*=(double);
	template <class Iter> Array& operator*=(VectorialExpression<Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "multiplying arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i *= *e; ++i; ++e; }
		return *this;
	}
	Array& operator/=(const Array&);
	Array& operator/=(double);
	template <class Iter> Array& operator/=(VectorialExpression<Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "dividing arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i /= *e; ++i; ++e; }
		return *this;
	}
	// element access
	double operator[](int) const;
	double& operator[](int);
	// info
	int size() const;
	// iterators
	typedef double* iterator;
	typedef const double* const_iterator;
	typedef QL_REVERSE_ITERATOR(iterator,double) reverse_iterator;
	typedef QL_REVERSE_ITERATOR(const_iterator,double) const_reverse_iterator;
	const_iterator begin() const;
	iterator begin();
	const_iterator end() const;
	iterator end();
	const_reverse_iterator rbegin() const;
	reverse_iterator rbegin();
	const_reverse_iterator rend() const;
	reverse_iterator rend();
  protected:
	void allocate(int size);
	void resize(int size);
	void copy(const Array& from) {
		std::copy(from.begin(),from.end(),begin());
	}
	template <class Iter> void copy(VectorialExpression<Iter>& e) {
		iterator i = begin(), j = end();
		while (i != j) {
			*i = *e;
			++i; ++e;
		}
	}
  private:
	double* pointer;
	int n, bufferSize;
};

// vectorial products
 double DotProduct(const Array&, const Array&);

// unary operators
 
VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Plus> >
operator+(const Array& v);
template <class Iter1> 
VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Plus> >
operator+(const VectorialExpression<Iter1>& e);

VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Minus> >
operator-(const Array& v);
template <class Iter1> 
VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Minus> >
operator-(const VectorialExpression<Iter1>& e);

// binary operators
// addition
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Add> >
operator+(const Array& v1, const Array& v2);
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Add> >
operator+(const Array& v1, double x);
VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Add> >
operator+(double x, const Array& v2);
template <class Iter2> 
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Add> >
operator+(const Array& v1, const VectorialExpression<Iter2>& e2);
template <class Iter1> 
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Add> >
operator+(const VectorialExpression<Iter1>& e1, const Array& v2);
template <class Iter1>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Add> >
operator+(const VectorialExpression<Iter1>& e1, double x);
template <class Iter2>
VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Add> >
operator+(double x, const VectorialExpression<Iter2>& e2);
template <class Iter1, class Iter2>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Add> >
operator+(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2);
// subtraction
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Subtract> >
operator-(const Array& v1, const Array& v2);
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Subtract> >
operator-(const Array& v1, double x);
VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Subtract> >
operator-(double x, const Array& v2);
template <class Iter2> 
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Subtract> >
operator-(const Array& v1, const VectorialExpression<Iter2>& e2);
template <class Iter1> 
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Subtract> >
operator-(const VectorialExpression<Iter1>& e1, const Array& v2);
template <class Iter1>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Subtract> >
operator-(const VectorialExpression<Iter1>& e1, double x);
template <class Iter2>
VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Subtract> >
operator-(double x, const VectorialExpression<Iter2>& e2);
template <class Iter1, class Iter2>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Subtract> >
operator-(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2);
// multiplication
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Multiply> >
operator*(const Array& v1, const Array& v2);
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Multiply> >
operator*(const Array& v1, double x);
VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Multiply> >
operator*(double x, const Array& v2);
template <class Iter2> 
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Multiply> >
operator*(const Array& v1, const VectorialExpression<Iter2>& e2);
template <class Iter1> 
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Multiply> >
operator*(const VectorialExpression<Iter1>& e1, const Array& v2);
template <class Iter1>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Multiply> >
operator*(const VectorialExpression<Iter1>& e1, double x);
template <class Iter2>
VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Multiply> >
operator*(double x, const VectorialExpression<Iter2>& e2);
template <class Iter1, class Iter2>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Multiply> >
operator*(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2);
// division
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Divide> >
operator/(const Array& v1, const Array& v2);
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Divide> >
operator/(const Array& v1, double x);
VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Divide> >
operator/(double x, const Array& v2);
template <class Iter2> 
VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Divide> >
operator/(const Array& v1, const VectorialExpression<Iter2>& e2);
template <class Iter1> 
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Divide> >
operator/(const VectorialExpression<Iter1>& e1, const Array& v2);
template <class Iter1>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Divide> >
operator/(const VectorialExpression<Iter1>& e1, double x);
template <class Iter2>
VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Divide> >
operator/(double x, const VectorialExpression<Iter2>& e2);
template <class Iter1, class Iter2>
VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Divide> >
operator/(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2);

// math functions

VectorialExpression<UnaryVectorialExpression<Array::const_iterator,AbsoluteValue> >
Abs(const Array& v);
template <class Iter1> 
VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,AbsoluteValue> >
Abs(const VectorialExpression<Iter1>& e);

VectorialExpression<UnaryVectorialExpression<Array::const_iterator,SquareRoot> >
Sqrt(const Array& v);
template <class Iter1> 
VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,SquareRoot> >
Sqrt(const VectorialExpression<Iter1>& e);

VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Logarithm> >
Log(const Array& v);
template <class Iter1> 
VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Logarithm> >
Log(const VectorialExpression<Iter1>& e);

VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Exponential> >
Exp(const Array& v);
template <class Iter1> 
VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Exponential> >
Exp(const VectorialExpression<Iter1>& e);

// inline definitions

inline Array::Array(int size)
: pointer(0), n(0), bufferSize(0) {
	if (size > 0)
		allocate(size);
}

inline Array::Array(int size, double value)
: pointer(0), n(0), bufferSize(0) {
	if (size > 0)
		allocate(size);
	std::fill(begin(),end(),value);
}

inline Array::Array(int size, double value, double increment)
: pointer(0), n(0), bufferSize(0) {
	if (size > 0)
		allocate(size);
	for (iterator i=begin(); i!=end(); i++,value+=increment)
		*i = value;
}

inline Array::~Array() {
	if (pointer != 0 && bufferSize != 0)
		delete[] pointer;
	pointer = 0;
	n = bufferSize = 0;
}

inline Array& Array::operator+=(const Array& v) {
	Require(n == v.n, "arrays with different sizes cannot be added");
	std::transform(begin(),end(),v.begin(),begin(),std::plus<double>());
	return *this;
}

inline Array& Array::operator+=(double x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::plus<double>(),x));
	return *this;
}

inline Array& Array::operator-=(const Array& v) {
	Require(n == v.n, "arrays with different sizes cannot be subtracted");
	std::transform(begin(),end(),v.begin(),begin(),std::minus<double>());
	return *this;
}

inline Array& Array::operator-=(double x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::minus<double>(),x));
	return *this;
}

inline Array& Array::operator*=(const Array& v) {
	Require(n == v.n, "arrays with different sizes cannot be multiplied");
	std::transform(begin(),end(),v.begin(),begin(),std::multiplies<double>());
	return *this;
}

inline Array& Array::operator*=(double x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<double>(),x));
	return *this;
}

inline Array& Array::operator/=(const Array& v) {
	Require(n == v.n, "arrays with different sizes cannot be divided");
	std::transform(begin(),end(),v.begin(),begin(),std::divides<double>());
	return *this;
}

inline Array& Array::operator/=(double x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::divides<double>(),x));
	return *this;
}

inline double Array::operator[](int i) const {
	#ifdef QL_DEBUG
		Require(i>=0 && i<n, "array cannot be accessed out of range");
	#endif
	return pointer[i];
}

inline double& Array::operator[](int i) {
	#ifdef QL_DEBUG
		Require(i>=0 && i<n, "array cannot be accessed out of range");
	#endif
	return pointer[i];
}

inline int Array::size() const {
	return n;
}

inline void Array::resize(int size) {
	if (size != n) {
		if (size <= bufferSize) {
			n = size;
		} else {
			Array temp(size);
			std::copy(begin(),end(),temp.begin());
			allocate(size);
			copy(temp);
		}
	}
}

inline Array::const_iterator Array::begin() const {
	return pointer;
}

inline Array::iterator Array::begin() {
	return pointer;
}

inline Array::const_iterator Array::end() const {
	return pointer+n;
}

inline Array::iterator Array::end() {
	return pointer+n;
}

inline Array::const_reverse_iterator Array::rbegin() const {
	return const_reverse_iterator(end());
}

inline Array::reverse_iterator Array::rbegin() {
	return reverse_iterator(end());
}

inline Array::const_reverse_iterator Array::rend() const {
	return const_reverse_iterator(begin());
}

inline Array::reverse_iterator Array::rend() {
	return reverse_iterator(begin());
}

inline void Array::allocate(int size) {
	if (pointer != 0 && bufferSize != 0)
		delete[] pointer;
	if (size <= 0) {
		pointer = 0;
	} else {
		n = size;
		bufferSize = size+size/10+10;
		try {
			pointer = new double[bufferSize];
		}
		catch (...) {
			pointer = 0;
		}
		if (pointer == 0) {
			n = bufferSize = size;
			try {
				pointer = new double[bufferSize];
			}
			catch (...) {
				pointer = 0;
			}
			if (pointer == 0) {
				n = bufferSize = 0;
				throw OutOfMemoryError("Array");
			}
		}
	}
}

// overloaded operators

// unary

inline VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Plus> >
operator+(const Array& v) {
	typedef UnaryVectorialExpression<Array::const_iterator,Plus> Iter;
	return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Iter1> 
inline VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Plus> >
operator+(const VectorialExpression<Iter1>& e) {
	typedef UnaryVectorialExpression<VectorialExpression<Iter1>,Plus> Iter;
	return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
}

inline VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Minus> >
operator-(const Array& v) {
	typedef UnaryVectorialExpression<Array::const_iterator,Minus> Iter;
	return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Iter1> 
inline VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Minus> >
operator-(const VectorialExpression<Iter1>& e) {
	typedef UnaryVectorialExpression<VectorialExpression<Iter1>,Minus> Iter;
	return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
}


// binary operators

// addition

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Add> >
operator+(const Array& v1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Add> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Add> >
operator+(const Array& v1, double x) {
	typedef BinaryVectorialExpression<Array::const_iterator,Scalar,Add> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Add> >
operator+(double x, const Array& v2) {
	typedef BinaryVectorialExpression<Scalar,Array::const_iterator,Add> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
}

template <class Iter2> 
inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Add> >
operator+(const Array& v1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Add> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Iter1> 
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Add> >
operator+(const VectorialExpression<Iter1>& e1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Add> Iter;
	return VectorialExpression<Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Iter1, class Iter2>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Add> >
operator+(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Add> Iter;
	return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Iter1>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Add> >
operator+(const VectorialExpression<Iter1>& e1, double x) {
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Add> Iter;
	return VectorialExpression<Iter>(Iter(e1,Scalar(x),e1.size()),e1.size());
}

template <class Iter2>
inline VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Add> >
operator+(double x, const VectorialExpression<Iter2>& e2) {
	typedef BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Add> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),e2,e2.size()),e2.size());
}

// subtraction

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Subtract> >
operator-(const Array& v1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Subtract> >
operator-(const Array& v1, double x) {
	typedef BinaryVectorialExpression<Array::const_iterator,Scalar,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Subtract> >
operator-(double x, const Array& v2) {
	typedef BinaryVectorialExpression<Scalar,Array::const_iterator,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
}

template <class Iter2> 
inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Subtract> >
operator-(const Array& v1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Iter1> 
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Subtract> >
operator-(const VectorialExpression<Iter1>& e1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Iter1, class Iter2>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Subtract> >
operator-(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Iter1>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Subtract> >
operator-(const VectorialExpression<Iter1>& e1, double x) {
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(e1,Scalar(x),e1.size()),e1.size());
}

template <class Iter2>
inline VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Subtract> >
operator-(double x, const VectorialExpression<Iter2>& e2) {
	typedef BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Subtract> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),e2,e2.size()),e2.size());
}

// multiplication

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Multiply> >
operator*(const Array& v1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Multiply> >
operator*(const Array& v1, double x) {
	typedef BinaryVectorialExpression<Array::const_iterator,Scalar,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Multiply> >
operator*(double x, const Array& v2) {
	typedef BinaryVectorialExpression<Scalar,Array::const_iterator,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
}

template <class Iter2> 
inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Multiply> >
operator*(const Array& v1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Iter1> 
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Multiply> >
operator*(const VectorialExpression<Iter1>& e1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Iter1, class Iter2>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Multiply> >
operator*(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Iter1>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Multiply> >
operator*(const VectorialExpression<Iter1>& e1, double x) {
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(e1,Scalar(x),e1.size()),e1.size());
}

template <class Iter2>
inline VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Multiply> >
operator*(double x, const VectorialExpression<Iter2>& e2) {
	typedef BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Multiply> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),e2,e2.size()),e2.size());
}

// division

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Divide> >
operator/(const Array& v1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,Array::const_iterator,Divide> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,Scalar,Divide> >
operator/(const Array& v1, double x) {
	typedef BinaryVectorialExpression<Array::const_iterator,Scalar,Divide> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),Scalar(x),v1.size()),v1.size());
}

inline VectorialExpression<BinaryVectorialExpression<Scalar,Array::const_iterator,Divide> >
operator/(double x, const Array& v2) {
	typedef BinaryVectorialExpression<Scalar,Array::const_iterator,Divide> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),v2.begin(),v2.size()),v2.size());
}

template <class Iter2> 
inline VectorialExpression<BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Divide> >
operator/(const Array& v1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Array::const_iterator,VectorialExpression<Iter2>,Divide> Iter;
	return VectorialExpression<Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Iter1> 
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Divide> >
operator/(const VectorialExpression<Iter1>& e1, const Array& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Array::const_iterator,Divide> Iter;
	return VectorialExpression<Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Iter1, class Iter2>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Divide> >
operator/(const VectorialExpression<Iter1>& e1, const VectorialExpression<Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,VectorialExpression<Iter2>,Divide> Iter;
	return VectorialExpression<Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Iter1>
inline VectorialExpression<BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Divide> >
operator/(const VectorialExpression<Iter1>& e1, double x) {
	typedef BinaryVectorialExpression<VectorialExpression<Iter1>,Scalar,Divide> Iter;
	return VectorialExpression<Iter>(Iter(e1,Scalar(x),e1.size()),e1.size());
}

template <class Iter2>
inline VectorialExpression<BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Divide> >
operator/(double x, const VectorialExpression<Iter2>& e2) {
	typedef BinaryVectorialExpression<Scalar,VectorialExpression<Iter2>,Divide> Iter;
	return VectorialExpression<Iter>(Iter(Scalar(x),e2,e2.size()),e2.size());
}

// dot product

inline double DotProduct(const Array& v1, const Array& v2) {
	Require(v1.size() == v2.size(), "arrays with different sizes cannot be multiplied");
	return std::inner_product(v1.begin(),v1.end(),v2.begin(),0.0);
}

// functions

// Abs()
 
inline VectorialExpression<UnaryVectorialExpression<Array::const_iterator,AbsoluteValue> >
Abs(const Array& v) {
	typedef UnaryVectorialExpression<Array::const_iterator,AbsoluteValue> Iter;
	return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Iter1> 
inline VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,AbsoluteValue> >
Abs(const VectorialExpression<Iter1>& e) {
	typedef UnaryVectorialExpression<VectorialExpression<Iter1>,AbsoluteValue> Iter;
	return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
}

// Sqrt()
 
inline VectorialExpression<UnaryVectorialExpression<Array::const_iterator,SquareRoot> >
Sqrt(const Array& v) {
	typedef UnaryVectorialExpression<Array::const_iterator,SquareRoot> Iter;
	return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Iter1> 
inline VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,SquareRoot> >
Sqrt(const VectorialExpression<Iter1>& e) {
	typedef UnaryVectorialExpression<VectorialExpression<Iter1>,SquareRoot> Iter;
	return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
}

// Log()
 
inline VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Logarithm> >
Log(const Array& v) {
	typedef UnaryVectorialExpression<Array::const_iterator,Logarithm> Iter;
	return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Iter1> 
inline VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Logarithm> >
Log(const VectorialExpression<Iter1>& e) {
	typedef UnaryVectorialExpression<VectorialExpression<Iter1>,Logarithm> Iter;
	return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
}

// Exp()
 
inline VectorialExpression<UnaryVectorialExpression<Array::const_iterator,Exponential> >
Exp(const Array& v) {
	typedef UnaryVectorialExpression<Array::const_iterator,Exponential> Iter;
	return VectorialExpression<Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Iter1> 
inline VectorialExpression<UnaryVectorialExpression<VectorialExpression<Iter1>,Exponential> >
Exp(const VectorialExpression<Iter1>& e) {
	typedef UnaryVectorialExpression<VectorialExpression<Iter1>,Exponential> Iter;
	return VectorialExpression<Iter>(Iter(e,e.size()),e.size());
}

QL_END_NAMESPACE(QuantLib)


#endif
