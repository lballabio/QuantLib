
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

/*! \file steppingiterator.h
	\brief Iterator advancing in constant steps
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2001/01/11 11:43:52  lballabio
	Renamed StepIterator to SteppingIterator

	Revision 1.1  2001/01/09 11:51:10  lballabio
	Added a couple of smart iterators
	
*/

#ifndef quantlib_stepping_iterator_h
#define quantlib_stepping_iterator_h

#include <iterator>

namespace QuantLib {
	
	namespace Utilities {
	
		//! Iterator advancing in constant steps
		/*! This iterator advances an underlying random access
			iterator in steps of \f$ n \f$ positions, where 
			\f$ n \f$ is an integer given upon construction.
		*/
		template <class RandomAccessIterator>
		class SteppingIterator {
		  public:
			typedef std::iterator_traits<RandomAccessIterator> traits;
			typedef std::random_access_iterator_tag   iterator_category;
			typedef typename traits::value_type       value_type;
			typedef typename traits::difference_type  difference_type;
			typedef typename traits::pointer          pointer;
			typedef typename traits::reference        reference;
			SteppingIterator(const RandomAccessIterator&, difference_type step);
			//! \name Dereferencing
			//@{
			reference operator*()  const;
			pointer   operator->() const;
			//@}
			//! \name Random access
			//@{
			reference operator[](int) const;
			//@}
			//! \name Increment and decrement
			//@{
			SteppingIterator& operator++();
			SteppingIterator  operator++(int);
			SteppingIterator& operator--();
			SteppingIterator  operator--(int);
			SteppingIterator& operator+=(difference_type);
			SteppingIterator& operator-=(difference_type);
			SteppingIterator<RandomAccessIterator> operator+(difference_type);
			SteppingIterator<RandomAccessIterator> operator-(difference_type);
			//@}
			//! \name Difference
			//@{
			difference_type operator-(
			    const SteppingIterator<RandomAccessIterator>&);
			//@}
			//! \name Comparisons
			//@{
			bool operator==(const SteppingIterator<RandomAccessIterator>&);
			bool operator!=(const SteppingIterator<RandomAccessIterator>&);
			bool operator< (const SteppingIterator<RandomAccessIterator>&);
			bool operator> (const SteppingIterator<RandomAccessIterator>&);
			bool operator<=(const SteppingIterator<RandomAccessIterator>&);
			bool operator>=(const SteppingIterator<RandomAccessIterator>&);
			//@}
		  private:
			difference_type dn_;
			RandomAccessIterator it_;
		};
		

		// inline definitions
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>::SteppingIterator(
		  const RandomAccessIterator& it, 
		  SteppingIterator<RandomAccessIterator>::difference_type step)
		: dn_(step), it_(it) {}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>& 
		SteppingIterator<RandomAccessIterator>::operator++() {
			it_ += dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator> 
		SteppingIterator<RandomAccessIterator>::operator++(int) {
			SteppingIterator<RandomAccessIterator> temp = *this;
			it_ += dn_;
			return temp;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>& 
		SteppingIterator<RandomAccessIterator>::operator--() {
			it_ -= dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator> 
		SteppingIterator<RandomAccessIterator>::operator--(int) {
			SteppingIterator<RandomAccessIterator> temp = *this;
			it_ -= dn_;
			return temp;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>& 
		SteppingIterator<RandomAccessIterator>::operator+=(
		  SteppingIterator<RandomAccessIterator>::difference_type i) {
			it_ += i*dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>& 
		SteppingIterator<RandomAccessIterator>::operator-=(
		  SteppingIterator<RandomAccessIterator>::difference_type i) {
			it_ -= i*dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>::reference
		SteppingIterator<RandomAccessIterator>::operator*() const {
			return *it_;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>::pointer 
		SteppingIterator<RandomAccessIterator>::operator->() const {
			return it_;
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>::reference
		SteppingIterator<RandomAccessIterator>::operator[](int i) const {
			return it_[i*dn_];
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator> 
		SteppingIterator<RandomAccessIterator>::operator+(
		  SteppingIterator<RandomAccessIterator>::difference_type i) {
			return SteppingIterator<RandomAccessIterator>(dn_,it_+dn_*i);
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator> 
		SteppingIterator<RandomAccessIterator>::operator-(
		  SteppingIterator<RandomAccessIterator>::difference_type i) {
			return SteppingIterator<RandomAccessIterator>(dn_,it_-dn_*i);
		}
		
		template<class RandomAccessIterator>
		inline SteppingIterator<RandomAccessIterator>::difference_type 
		SteppingIterator<RandomAccessIterator>::operator-(
		  const SteppingIterator<RandomAccessIterator>& i) {
		    #ifdef QL_DEBUG
		    	QL_REQUIRE((it_-i.it_)%dn_ == 0,
		    	  "Cannot subtract stepping iterators not reachable "
		    	  "from each other")
			#endif
			return (it_-i.it_)/dn_;
		}
		
		template<class RandomAccessIterator>
		inline bool SteppingIterator<RandomAccessIterator>::operator==(
		  const SteppingIterator<RandomAccessIterator>& i) {
			return (it_ == i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool SteppingIterator<RandomAccessIterator>::operator!=(
		  const SteppingIterator<RandomAccessIterator>& i) {
			return (it_ != i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool SteppingIterator<RandomAccessIterator>::operator<(
		  const SteppingIterator<RandomAccessIterator>& i) {
			return (it_ < i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool SteppingIterator<RandomAccessIterator>::operator>(
		  const SteppingIterator<RandomAccessIterator>& i) {
			return (it_ > i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool SteppingIterator<RandomAccessIterator>::operator<=(
		  const SteppingIterator<RandomAccessIterator>& i) {
			return (it_ <= i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool SteppingIterator<RandomAccessIterator>::operator>=(
		  const SteppingIterator<RandomAccessIterator>& i) {
			return (it_ >= i.it_);
		}
		
	}

}


#endif
