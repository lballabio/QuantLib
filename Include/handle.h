
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

/*! \file handle.h
	\brief Reference-counted pointer
*/

#ifndef quantlib_handle_h
#define quantlib_handle_h

#include "qldefines.h"

// The implementation of this class is taken from 
// "The C++ Programming Language", 3rd edition, B.Stroustrup

namespace QuantLib {

	//! Reference-counted pointer
	/*! This class acts as a proxy to a pointer contained in it. Such pointer is
		owned by the handle, i.e., the handle will be responsible for its deletion.
		A count of the references to the contained pointer is incremented every
		time a handle is copied, and decremented every time a handle is deleted or 
		goes out of scope. This mechanism ensures on one hand, that the pointer will
		not be deallocated as long as a handle refers to it, and on the other hand, that 
		it will be deallocated when no more handles do.
		
		\note The implementation of this class was originally taken from 
		"The C++ Programming Language", 3rd ed., B.Stroustrup, Addison-Wesley, 1997.

		\warning This mechanism will broke and result in untimely deallocation of the 
		pointer (and very possible death of your executable) if two handles are explicitly 
		initialized with the same pointer, as in
		\code
		SomeObj* so = new SomeObj;
		Handle<SomeObj> h1(so);
		Handle<SomeObj> h2 = h1;	// this is safe.
		Handle<SomeObj> h3(so);		// this is definitely not.
		\endcode
		It is good practice to create the pointer and immediately pass it to the handle,
		as in
		\code
		Handle<SomeObj> h1(new SomeObj);	// this is as safe as can be.
		\endcode
	*/
	template <class Type>
	class Handle {
	  public:
		//! \name constructors, destructor, and assignment
		//@{
		//! Default constructor returning a null handle.
		Handle()
		: ptr(0), n(new int(1)) {}
		explicit Handle(Type* ptr)
		: ptr(ptr), n(new int(1)) {}
		//! Copy from a handle to a different but compatible type
		template <class Type2> explicit Handle(const Handle<Type2>& from)
		: ptr(0), n(new int(1)) { HandleCopier().copy(*this,from); }
		Handle(const Handle& from)
		: ptr(from.ptr), n(from.n) { (*n)++; }
		~Handle();
		Handle& operator=(const Handle& from);
		//@}
		
		//! \name Proxy to pointer dereferencing
		//@{
		Type& operator*() const;
		Type* operator->() const;
		//@}
		
		// \name Pointer access
		//@{
		//! Read-only access
		const Type QL_PTR_CONST pointer() const;
		//! Read-write access - <b>use with care</b>.
		Type * pointer();
		//@}
	  private:
		Type* ptr;
		int* n;
		// used to convert handles to different but compatible types
		class HandleCopier;
		friend class HandleCopier;
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
	};
	
	template <class Type> bool operator==(const Handle<Type>&, const Handle<Type>&);
	template <class Type> bool operator!=(const Handle<Type>&, const Handle<Type>&);
	
	
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
	inline const Type QL_PTR_CONST Handle<Type>::pointer() const {
		return ptr;
	}
	
	template <class Type>
	inline Type* Handle<Type>::pointer() {
		return ptr;
	}
	
	/*! \relates Handle
		Returns <tt>true</tt> iff the two handles contain the same pointer.
	*/
	template <class Type>
	inline bool operator==(const Handle<Type>& h1, const Handle<Type>& h2) {
		return (h1.pointer() == h2.pointer());
	}
	
	/*! \relates Handle
		Returns <tt>true</tt> iff the two handles contain different pointers.
	*/
	template <class Type>
	inline bool operator!=(const Handle<Type>& h1, const Handle<Type>& h2) {
		return !(h1 == h2);
	}

}


#endif
