
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

#ifndef ql_location_h
#define ql_location_h

namespace QuantLib{

	template <class Iterator, class Type>
	int location(Iterator begin, Iterator end, const Type& x) {
		if (begin == end) {
			throw Error("location: begin == end");
		} else if (x < *begin) {
			return -1;
		} else if (x > *(end-1)) {
			return end-begin;
		} else {
			Iterator i=begin, j=end, k;
			while (j-i > 1) {
				k = i+(j-i)/2;
				if (x < *k)
					j = k;
				else
					i = k;
			}
			return (i-begin);
		}
	}

	template <class Iterator, class Type>
	int location(Iterator begin, Iterator end, const Type& x, int guess) {
	  int n = end-begin;
		if (n == 0) {
			throw Error("location: begin == end");
		} else if (guess >=0 && guess < n-1) {
			if (x >= *(begin+guess) && x < *(begin+guess+1))
				return guess;
			else if (guess < n-2 && x >= *(begin+guess+1) && x < *(begin+guess+2))
				return guess+1;
			else
				return location(begin, end, x);
		} else if (guess == n-1 && x == *(begin+guess)) {
			return guess;
		} else {
			return location(begin, end, x);
		}
	}

}
#endif
