
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_array_h
#define quantlib_array_h

#include "qldefines.h"
#include "expressiontemplates.h"

/* The concepts implemented here with regard to vector algebra are from 
   T. L. Veldhuizen, "Expression templates", C++ Report, 7(5):26-31, June 1995
   http://extreme.indiana.edu/~tveldhui/papers/
*/

QL_BEGIN_NAMESPACE(QuantLib)

// array

template <class Type>
class Array {
  public:
	// constructors
	explicit Array(int size = 0);
	Array(int size, Type value);
	Array(int size, Type value, Type increment);
	Array(const Array<Type>& from)
	  : pointer(0), n(0), bufferSize(0) { allocate(from.size()); copy(from); }
	template <class Iter> Array(VectorialExpression<Type,Iter>& e)
	  : pointer(0), n(0), bufferSize(0) { allocate(e.size()); copy(e); }
	// destructor
	~Array();
	// assignment
	Array& operator=(const Array<Type>& from) {
		if (this != &from) { resize(from.size()); copy(from); } return *this; }
	template <class Iter> Array& operator=(VectorialExpression<Type,Iter>& e) {
		resize(e.size()); copy(e); return *this;
	}
	// computed assignment
	Array& operator+=(const Array&);
	Array& operator+=(Type);
	template <class Iter> Array& operator+=(VectorialExpression<Type,Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "adding arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i += *e; ++i; ++e; }
		return *this;
	}
	Array& operator-=(const Array&);
	Array& operator-=(Type);
	template <class Iter> Array& operator-=(VectorialExpression<Type,Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "subtracting arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i -= *e; ++i; ++e; }
		return *this;
	}
	Array& operator*=(const Array&);
	Array& operator*=(double);
	template <class Iter> Array& operator*=(VectorialExpression<Type,Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "multiplying arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i *= *e; ++i; ++e; }
		return *this;
	}
	Array& operator/=(const Array&);
	Array& operator/=(double);
	template <class Iter> Array& operator/=(VectorialExpression<Type,Iter>& e) {
		#ifdef QL_DEBUG
			Require(size() == e.size(), "dividing arrays with different sizes");
		#endif
		iterator i = begin(), j = end();
		while (i != j) { *i /= *e; ++i; ++e; }
		return *this;
	}
	// element access
	const Type& operator[](int) const;
	Type& operator[](int);
	// info
	int size() const;
	// iterators
	typedef Type* iterator;
	typedef const Type* const_iterator;
	#if defined(__MWERKS__) || defined(__BORLANDC__)
	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
	#else
	typedef std::reverse_iterator<iterator,Type> reverse_iterator;
	typedef std::reverse_iterator<const_iterator,Type> const_reverse_iterator;
	#endif
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
	template <class Type2> void copy(const Array<Type2>& from) {
		std::copy(from.begin(),from.end(),begin());
	}
	void copy(const Array<Type>& from) {
		std::copy(from.begin(),from.end(),begin());
	}
	template <class Iter> void copy(VectorialExpression<Type,Iter>& e) {
		iterator i = begin(), j = end();
		while (i != j) {
			*i = *e;
			++i; ++e;
		}
	}
  private:
	Type* pointer;
	int n, bufferSize;
};

// vectorial products
template <class Type> Type DotProduct(const Array<Type>&, const Array<Type>&);

// unary operators
template <class Type> 
VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Plus<Type> > >
operator+(const Array<Type>& v);
template <class Type, class Iter1> 
VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Plus<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e);

template <class Type> 
VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Minus<Type> > >
operator-(const Array<Type>& v);
template <class Type, class Iter1> 
VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Minus<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e);

// binary operators
// addition
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Add<Type> > >
operator+(const Array<Type>& v1, const Array<Type>& v2);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Add<Type> > >
operator+(const Array<Type>& v1, const Type& x);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Add<Type> > >
operator+(const Type& x, const Array<Type>& v2);
template <class Type, class Iter2> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Add<Type> > >
operator+(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1> 
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Add<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2);
template <class Type, class Iter1>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Add<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e1, const Type& x);
template <class Type, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Add<Type> > >
operator+(const Type& x, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Add<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2);
// subtraction
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Subtract<Type> > >
operator-(const Array<Type>& v1, const Array<Type>& v2);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Subtract<Type> > >
operator-(const Array<Type>& v1, const Type& x);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Subtract<Type> > >
operator-(const Type& x, const Array<Type>& v2);
template <class Type, class Iter2> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Subtract<Type> > >
operator-(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1> 
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Subtract<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2);
template <class Type, class Iter1>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Subtract<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e1, const Type& x);
template <class Type, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Subtract<Type> > >
operator-(const Type& x, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Subtract<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2);
// multiplication
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Multiply<Type> > >
operator*(const Array<Type>& v1, const Array<Type>& v2);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Multiply<Type> > >
operator*(const Array<Type>& v1, const Type& x);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Multiply<Type> > >
operator*(const Type& x, const Array<Type>& v2);
template <class Type, class Iter2> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Multiply<Type> > >
operator*(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1> 
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Multiply<Type> > >
operator*(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2);
template <class Type, class Iter1>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Multiply<Type> > >
operator*(const VectorialExpression<Type,Iter1>& e1, const Type& x);
template <class Type, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Multiply<Type> > >
operator*(const Type& x, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Multiply<Type> > >
operator*(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2);
// division
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Divide<Type> > >
operator/(const Array<Type>& v1, const Array<Type>& v2);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Divide<Type> > >
operator/(const Array<Type>& v1, const Type& x);
template <class Type> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Divide<Type> > >
operator/(const Type& x, const Array<Type>& v2);
template <class Type, class Iter2> 
VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Divide<Type> > >
operator/(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1> 
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Divide<Type> > >
operator/(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2);
template <class Type, class Iter1>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Divide<Type> > >
operator/(const VectorialExpression<Type,Iter1>& e1, const Type& x);
template <class Type, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Divide<Type> > >
operator/(const Type& x, const VectorialExpression<Type,Iter2>& e2);
template <class Type, class Iter1, class Iter2>
VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Divide<Type> > >
operator/(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2);


// math functions

template <class Type> 
VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,AbsoluteValue<Type> > >
Abs(const Array<Type>& v);
template <class Type, class Iter1> 
VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,AbsoluteValue<Type> > >
Abs(const VectorialExpression<Type,Iter1>& e);

template <class Type> 
VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,SquareRoot<Type> > >
Sqrt(const Array<Type>& v);
template <class Type, class Iter1> 
VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,SquareRoot<Type> > >
Sqrt(const VectorialExpression<Type,Iter1>& e);

template <class Type> 
VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Logarithm<Type> > >
Log(const Array<Type>& v);
template <class Type, class Iter1> 
VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Logarithm<Type> > >
Log(const VectorialExpression<Type,Iter1>& e);

template <class Type> 
VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Exponential<Type> > >
Exp(const Array<Type>& v);
template <class Type, class Iter1> 
VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Exponential<Type> > >
Exp(const VectorialExpression<Type,Iter1>& e);

// inline definitions

template <class Type>
inline Array<Type>::Array(int size)
: pointer(0), n(0), bufferSize(0) {
	if (size > 0)
		allocate(size);
}

template <class Type>
inline Array<Type>::Array(int size, Type value)
: pointer(0), n(0), bufferSize(0) {
	if (size > 0)
		allocate(size);
	std::fill(begin(),end(),value);
}

template <class Type>
inline Array<Type>::Array(int size, Type value, Type increment)
: pointer(0), n(0), bufferSize(0) {
	if (size > 0)
		allocate(size);
	for (iterator i=begin(); i!=end(); i++,value+=increment)
		*i = value;
}

template <class Type>
inline Array<Type>::~Array() {
	if (pointer != 0 && bufferSize != 0)
		delete[] pointer;
	pointer = 0;
	n = bufferSize = 0;
}

template <class Type>
inline Array<Type>& Array<Type>::operator+=(const Array<Type>& v) {
	Require(n == v.n, "arrays with different sizes cannot be added");
	std::transform(begin(),end(),v.begin(),begin(),std::plus<Type>());
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator+=(Type x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::plus<Type>(),x));
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator-=(const Array<Type>& v) {
	Require(n == v.n, "arrays with different sizes cannot be subtracted");
	std::transform(begin(),end(),v.begin(),begin(),std::minus<Type>());
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator-=(Type x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::minus<Type>(),x));
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator*=(const Array<Type>& v) {
	Require(n == v.n, "arrays with different sizes cannot be multiplied");
	std::transform(begin(),end(),v.begin(),begin(),std::multiplies<Type>());
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator*=(double x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::multiplies<double>(),x));
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator/=(const Array<Type>& v) {
	Require(n == v.n, "arrays with different sizes cannot be divided");
	std::transform(begin(),end(),v.begin(),begin(),std::divides<Type>());
	return *this;
}

template <class Type>
inline Array<Type>& Array<Type>::operator/=(double x) {
	std::transform(begin(),end(),begin(),std::bind2nd(std::divides<double>(),x));
	return *this;
}

template <class Type>
inline const Type& Array<Type>::operator[](int i) const {
	#ifdef QL_DEBUG
		Require(i>=0 && i<n, "array cannot be accessed out of range");
	#endif
	return pointer[i];
}

template <class Type>
inline Type& Array<Type>::operator[](int i) {
	#ifdef QL_DEBUG
		Require(i>=0 && i<n, "array cannot be accessed out of range");
	#endif
	return pointer[i];
}

template <class Type>
inline int Array<Type>::size() const {
	return n;
}

template <class Type>
inline void Array<Type>::resize(int size) {
	if (size != n) {
		if (size <= bufferSize) {
			n = size;
		} else {
			Array<Type> temp(size);
			std::copy(begin(),end(),temp.begin());
//			swap(*this,temp);
			allocate(size);
			copy(temp);
		}
	}
}

template <class Type>
inline Array<Type>::const_iterator Array<Type>::begin() const {
	return pointer;
}

template <class Type>
inline Array<Type>::iterator Array<Type>::begin() {
	return pointer;
}

template <class Type>
inline Array<Type>::const_iterator Array<Type>::end() const {
	return pointer+n;
}

template <class Type>
inline Array<Type>::iterator Array<Type>::end() {
	return pointer+n;
}

template <class Type>
inline Array<Type>::const_reverse_iterator Array<Type>::rbegin() const {
	return const_reverse_iterator(end());
}

template <class Type>
inline Array<Type>::reverse_iterator Array<Type>::rbegin() {
	return reverse_iterator(end());
}

template <class Type>
inline Array<Type>::const_reverse_iterator Array<Type>::rend() const {
	return const_reverse_iterator(begin());
}

template <class Type>
inline Array<Type>::reverse_iterator Array<Type>::rend() {
	return reverse_iterator(begin());
}

template <class Type>
inline void Array<Type>::allocate(int size) {
	if (pointer != 0 && bufferSize != 0)
		delete[] pointer;
	if (size <= 0) {
		pointer = 0;
	} else {
		n = size;
		bufferSize = size+size/10+10;
		try {
			pointer = new Type[bufferSize];
		}
		catch (...) {
			pointer = 0;
		}
		if (pointer == 0) {
			n = bufferSize = size;
			try {
				pointer = new Type[bufferSize];
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

template <class Type> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Plus<Type> > >
operator+(const Array<Type>& v) {
	typedef UnaryVectorialExpression<Type,Array<Type>::const_iterator,Plus<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Plus<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e) {
	typedef UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Plus<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e,e.size()),e.size());
}

template <class Type> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Minus<Type> > >
operator-(const Array<Type>& v) {
	typedef UnaryVectorialExpression<Type,Array<Type>::const_iterator,Minus<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Minus<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e) {
	typedef UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Minus<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e,e.size()),e.size());
}


// binary operators

// addition

template <class Type>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Add<Type> > >
operator+(const Array<Type>& v1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Add<Type> > >
operator+(const Array<Type>& v1, const Type& x) {
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),Scalar<Type>(x),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Add<Type> > >
operator+(const Type& x, const Array<Type>& v2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter2> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Add<Type> > >
operator+(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Add<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter1, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Add<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Type, class Iter1>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Add<Type> > >
operator+(const VectorialExpression<Type,Iter1>& e1, const Type& x) {
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,Scalar<Type>(x),e1.size()),e1.size());
}

template <class Type, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Add<Type> > >
operator+(const Type& x, const VectorialExpression<Type,Iter2>& e2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Add<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),e2,e2.size()),e2.size());
}

// subtraction

template <class Type>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Subtract<Type> > >
operator-(const Array<Type>& v1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Subtract<Type> > >
operator-(const Array<Type>& v1, const Type& x) {
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),Scalar<Type>(x),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Subtract<Type> > >
operator-(const Type& x, const Array<Type>& v2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter2> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Subtract<Type> > >
operator-(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Subtract<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter1, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Subtract<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Type, class Iter1>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Subtract<Type> > >
operator-(const VectorialExpression<Type,Iter1>& e1, const Type& x) {
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,Scalar<Type>(x),e1.size()),e1.size());
}

template <class Type, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Subtract<Type> > >
operator-(const Type& x, const VectorialExpression<Type,Iter2>& e2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Subtract<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),e2,e2.size()),e2.size());
}

// multiplication

template <class Type>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Multiply<Type> > >
operator*(const Array<Type>& v1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Multiply<Type> > >
operator*(const Array<Type>& v1, const Type& x) {
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),Scalar<Type>(x),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Multiply<Type> > >
operator*(const Type& x, const Array<Type>& v2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter2> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Multiply<Type> > >
operator*(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Multiply<Type> > >
operator*(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter1, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Multiply<Type> > >
operator*(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Type, class Iter1>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Multiply<Type> > >
operator*(const VectorialExpression<Type,Iter1>& e1, const Type& x) {
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,Scalar<Type>(x),e1.size()),e1.size());
}

template <class Type, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Multiply<Type> > >
operator*(const Type& x, const VectorialExpression<Type,Iter2>& e2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Multiply<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),e2,e2.size()),e2.size());
}

// division

template <class Type>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Divide<Type> > >
operator/(const Array<Type>& v1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(v1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Array<Type>::const_iterator,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),v2.begin(),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Divide<Type> > >
operator/(const Array<Type>& v1, const Type& x) {
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,Scalar<Type>,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),Scalar<Type>(x),v1.size()),v1.size());
}

template <class Type> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Divide<Type> > >
operator/(const Type& x, const Array<Type>& v2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,Array<Type>::const_iterator,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter2> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Divide<Type> > >
operator/(const Array<Type>& v1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(v1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,Array<Type>::const_iterator,VectorialExpression<Type,Iter2>,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v1.begin(),e2,v1.size()),v1.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Divide<Type> > >
operator/(const VectorialExpression<Type,Iter1>& e1, const Array<Type>& v2) {
	#ifdef QL_DEBUG
		Require(e1.size() == v2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Array<Type>::const_iterator,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,v2.begin(),v2.size()),v2.size());
}

template <class Type, class Iter1, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Divide<Type> > >
operator/(const VectorialExpression<Type,Iter1>& e1, const VectorialExpression<Type,Iter2>& e2) {
	#ifdef QL_DEBUG
		Require(e1.size() == e2.size(), "adding arrays with different sizes");
	#endif
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,VectorialExpression<Type,Iter2>,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,e2,e1.size()),e1.size());
}

template <class Type, class Iter1>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Divide<Type> > >
operator/(const VectorialExpression<Type,Iter1>& e1, const Type& x) {
	typedef BinaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Scalar<Type>,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e1,Scalar<Type>(x),e1.size()),e1.size());
}

template <class Type, class Iter2>
inline VectorialExpression<Type,BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Divide<Type> > >
operator/(const Type& x, const VectorialExpression<Type,Iter2>& e2) {
	typedef BinaryVectorialExpression<Type,Scalar<Type>,VectorialExpression<Type,Iter2>,Divide<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(Scalar<Type>(x),e2,e2.size()),e2.size());
}

// dot product

template <class Type> 
inline Type DotProduct(const Array<Type>& v1, const Array<Type>& v2) {
	Require(v1.size() == v2.size(), "arrays with different sizes cannot be multiplied");
	return std::inner_product(v1.begin(),v1.end(),v2.begin(),Type());
}

// functions

// Abs()
template <class Type> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,AbsoluteValue<Type> > >
Abs(const Array<Type>& v) {
	typedef UnaryVectorialExpression<Type,Array<Type>::const_iterator,AbsoluteValue<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,AbsoluteValue<Type> > >
Abs(const VectorialExpression<Type,Iter1>& e) {
	typedef UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,AbsoluteValue<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e,e.size()),e.size());
}

// Sqrt()
template <class Type> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,SquareRoot<Type> > >
Sqrt(const Array<Type>& v) {
	typedef UnaryVectorialExpression<Type,Array<Type>::const_iterator,SquareRoot<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,SquareRoot<Type> > >
Sqrt(const VectorialExpression<Type,Iter1>& e) {
	typedef UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,SquareRoot<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e,e.size()),e.size());
}

// Log()
template <class Type> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Logarithm<Type> > >
Log(const Array<Type>& v) {
	typedef UnaryVectorialExpression<Type,Array<Type>::const_iterator,Logarithm<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Logarithm<Type> > >
Log(const VectorialExpression<Type,Iter1>& e) {
	typedef UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Logarithm<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e,e.size()),e.size());
}

// Exp()
template <class Type> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,Array<Type>::const_iterator,Exponential<Type> > >
Exp(const Array<Type>& v) {
	typedef UnaryVectorialExpression<Type,Array<Type>::const_iterator,Exponential<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(v.begin(),v.size()),v.size());
}

template <class Type, class Iter1> 
inline VectorialExpression<Type,UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Exponential<Type> > >
Exp(const VectorialExpression<Type,Iter1>& e) {
	typedef UnaryVectorialExpression<Type,VectorialExpression<Type,Iter1>,Exponential<Type> > Iter;
	return VectorialExpression<Type,Iter>(Iter(e,e.size()),e.size());
}


QL_END_NAMESPACE(QuantLib)

#endif
