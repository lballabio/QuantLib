
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

/*! \file stepiterator.h
	\brief Iterator advancing with constant step
	
	$Source$
	$Name$
	$Log$
	Revision 1.1  2001/01/09 11:51:10  lballabio
	Added a couple of smart iterators

*/

#ifndef quantlib_step_iterator_h
#define quantlib_step_iterator_h

#include <iterator>

namespace QuantLib {
	
	namespace Utilities {
	
		//! Iterator advancing with constant step
		/*! This iterator advances an underlying random access
			iterator in steps of \f$ n \f$ positions, where 
			\f$ n \f$ is an integer given upon construction.
		*/
		template <class RandomAccessIterator>
		class StepIterator {
		  public:
			typedef std::iterator_traits<RandomAccessIterator> traits;
			typedef std::random_access_iterator_tag   iterator_category;
			typedef typename traits::value_type       value_type;
			typedef typename traits::difference_type  difference_type;
			typedef typename traits::pointer          pointer;
			typedef typename traits::reference        reference;
			StepIterator(difference_type step, const RandomAccessIterator&);
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
			StepIterator& operator++();
			StepIterator  operator++(int);
			StepIterator& operator--();
			StepIterator  operator--(int);
			StepIterator& operator+=(difference_type);
			StepIterator& operator-=(difference_type);
			StepIterator<RandomAccessIterator> operator+(difference_type);
			StepIterator<RandomAccessIterator> operator-(difference_type);
			//@}
			//! \name Difference
			//@{
			difference_type operator-(
			    const StepIterator<RandomAccessIterator>&);
			//@}
			//! \name Comparisons
			//@{
			bool operator==(const StepIterator<RandomAccessIterator>&);
			bool operator!=(const StepIterator<RandomAccessIterator>&);
			bool operator< (const StepIterator<RandomAccessIterator>&);
			bool operator> (const StepIterator<RandomAccessIterator>&);
			bool operator<=(const StepIterator<RandomAccessIterator>&);
			bool operator>=(const StepIterator<RandomAccessIterator>&);
			//@}
		  private:
			difference_type dn_;
			RandomAccessIterator it_;
		};
		

		// inline definitions
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>::StepIterator(
		  StepIterator<RandomAccessIterator>::difference_type step, 
		  const RandomAccessIterator& it)
		: dn_(step), it_(it) {}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>& 
		StepIterator<RandomAccessIterator>::operator++() {
			it_ += dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator> 
		StepIterator<RandomAccessIterator>::operator++(int) {
			StepIterator<RandomAccessIterator> temp = *this;
			it_ += dn_;
			return temp;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>& 
		StepIterator<RandomAccessIterator>::operator--() {
			it_ -= dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator> 
		StepIterator<RandomAccessIterator>::operator--(int) {
			StepIterator<RandomAccessIterator> temp = *this;
			it_ -= dn_;
			return temp;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>& 
		StepIterator<RandomAccessIterator>::operator+=(
		  StepIterator<RandomAccessIterator>::difference_type i) {
			it_ += i*dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>& 
		StepIterator<RandomAccessIterator>::operator-=(
		  StepIterator<RandomAccessIterator>::difference_type i) {
			it_ -= i*dn_;
			return *this;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>::reference
		StepIterator<RandomAccessIterator>::operator*() const {
			return *it_;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>::pointer 
		StepIterator<RandomAccessIterator>::operator->() const {
			return it_;
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>::reference
		StepIterator<RandomAccessIterator>::operator[](int i) const {
			return it_[i*dn_];
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator> 
		StepIterator<RandomAccessIterator>::operator+(
		  StepIterator<RandomAccessIterator>::difference_type i) {
			return StepIterator<RandomAccessIterator>(dn_,it_+dn_*i);
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator> 
		StepIterator<RandomAccessIterator>::operator-(
		  StepIterator<RandomAccessIterator>::difference_type i) {
			return StepIterator<RandomAccessIterator>(dn_,it_-dn_*i);
		}
		
		template<class RandomAccessIterator>
		inline StepIterator<RandomAccessIterator>::difference_type 
		StepIterator<RandomAccessIterator>::operator-(
		  const StepIterator<RandomAccessIterator>& i) {
		    #ifdef QL_DEBUG
		    	QL_REQUIRE((it_-i.it_)%dn_ == 0,
		    	  "Cannot subtract step iterators not reachable "
		    	  "from each other")
			#endif
			return (it_-i.it_)/dn_;
		}
		
		template<class RandomAccessIterator>
		inline bool StepIterator<RandomAccessIterator>::operator==(
		  const StepIterator<RandomAccessIterator>& i) {
			return (it_ == i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool StepIterator<RandomAccessIterator>::operator!=(
		  const StepIterator<RandomAccessIterator>& i) {
			return (it_ != i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool StepIterator<RandomAccessIterator>::operator<(
		  const StepIterator<RandomAccessIterator>& i) {
			return (it_ < i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool StepIterator<RandomAccessIterator>::operator>(
		  const StepIterator<RandomAccessIterator>& i) {
			return (it_ > i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool StepIterator<RandomAccessIterator>::operator<=(
		  const StepIterator<RandomAccessIterator>& i) {
			return (it_ <= i.it_);
		}
		
		template<class RandomAccessIterator>
		inline bool StepIterator<RandomAccessIterator>::operator>=(
		  const StepIterator<RandomAccessIterator>& i) {
			return (it_ >= i.it_);
		}
		
	}

}


#endif
