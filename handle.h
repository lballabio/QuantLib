
/* 
Copyright (C) 2000 F.Ametrano, L.Ballabio, A.Benin, M.Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_handle_h
#define quantlib_handle_h

#include "qldefines.h"

// The implementation of this class is taken from 
// "The C++ Programming Language", 3rd edition, B.Stroustrup

QL_BEGIN_NAMESPACE(QuantLib)

template <class Type>
class Handle {
	friend class HandleCopier;
  public:
	// constructors and destructor
	Handle()
	: ptr(0), n(new int(1)) {}
	explicit Handle(Type* ptr)
	: ptr(ptr), n(new int(1)) {}
	template <class Type2> explicit Handle(const Handle<Type2>& from)
	: ptr(0), n(new int(1)) { HandleCopier().copy(*this,from); }
	Handle(const Handle& from)
	: ptr(from.ptr), n(from.n) { (*n)++; }
	~Handle();
	// assignment
	Handle& operator=(const Handle& from);
	// dereferencing
	Type& operator*() const;
	Type* operator->() const;
	// pointer access
	const Type const * pointer() const;
	Type * pointer();
  private:
	Type* ptr;
	int* n;
};

template <class Type> bool operator==(const Handle<Type>&, const Handle<Type>&);
template <class Type> bool operator!=(const Handle<Type>&, const Handle<Type>&);

class HandleCopier {
  public:
	HandleCopier() {}
	template <class Type1, class Type2> void copy(Handle<Type1>& to, Handle<Type2> from) const {
		if (to.ptr != from.ptr) {
			if (--(*(to.n)) == 0) {
				if (to.ptr != 0)
					delete to.ptr;
				delete to.n;
			}
			to.ptr = from.ptr;
			to.n = from.n;
			(*(to.n))++;
		}
	}
};


// inline definitions

template <class Type>
inline Handle<Type>::~Handle() {
	if (--(*n) == 0) {
		if (ptr != 0)
			delete ptr;
		delete n;
	}
}

template <class Type>
inline Handle<Type>& Handle<Type>::operator=(const Handle& from) {
	if (ptr != from.ptr) {
		if (--(*n) == 0) {
			if (ptr != 0)
				delete ptr;
			delete n;
		}
		ptr = from.ptr;
		n = from.n;
		(*n)++;
	}
	return *this;
}

template <class Type>
inline Type& Handle<Type>::operator*() const {
	return *ptr;
}

template <class Type>
inline Type* Handle<Type>::operator->() const {
	return ptr;
}

template <class Type>
inline const Type const * Handle<Type>::pointer() const {
	return ptr;
}

template <class Type>
inline Type* Handle<Type>::pointer() {
	return ptr;
}

template <class Type>
inline bool operator==(const Handle<Type>& h1, const Handle<Type>& h2) {
	return (h1.pointer() == h2.pointer());
}

template <class Type>
inline bool operator!=(const Handle<Type>& h1, const Handle<Type>& h2) {
	return !(h1 == h2);
}


QL_END_NAMESPACE(QuantLib)


#endif
