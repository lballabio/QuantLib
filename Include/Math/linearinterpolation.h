
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

/*! \file linearinterpolation.h
    \brief linear interpolation between discrete points
    
    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/01/15 13:36:01  lballabio
    Added interpolation classes

*/

#ifndef quantlib_linear_interpolation_h
#define quantlib_linear_interpolation_h

#include "qldefines.h"
#include "qlerrors.h"
#include "interpolation.h"

namespace QuantLib {

    namespace Math {
        
    	//! linear interpolation between discrete points
    	template <class RandomAccessIterator1, class RandomAccessIterator2>
    	class LinearInterpolation
    	: public Interpolation<RandomAccessIterator1,RandomAccessIterator2> {
    	  public:
            /*  these typedefs are repeated because Borland C++ won't inherit
                them from Interpolation - they shouldn't hurt, though.
            */
            typedef 
              typename std::iterator_traits<RandomAccessIterator1>::value_type
                argument_type;
            typedef 
              typename std::iterator_traits<RandomAccessIterator2>::value_type
                result_type;
    	 	LinearInterpolation(const RandomAccessIterator1& xBegin, 
    	 	    const RandomAccessIterator1& xEnd, 
    	 	    const RandomAccessIterator2& yBegin)
    	 	: Interpolation<RandomAccessIterator1,RandomAccessIterator2>(
    	 	    xBegin,xEnd,yBegin) {}
    		result_type operator()(const argument_type& x) const;
    	};


        // inline definitions
        
        template <class I1, class I2>
        inline LinearInterpolation<I1,I2>::result_type
        LinearInterpolation<I1,I2>::operator()(
            const LinearInterpolation<I1,I2>::argument_type& x) const {
                I1 i;
                if (x < *xBegin_)
                    i = xBegin_;
                else if (x > *(xEnd_-1))
                    i = xEnd_-2;
                else
                    i = Location(xBegin_,xEnd_,x);
                I2 j = yBegin_+(i-xBegin_);
                return *j + (x-*i)*double(*(j+1)-*j)/double(*(i+1)-*i);
        }

    }
	
}


#endif
