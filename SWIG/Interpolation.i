
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

/*  $Source$
	$Log$
	Revision 1.4  2001/03/09 12:40:41  lballabio
	Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_interpolation_i
#define quantlib_interpolation_i

%include QLArray.i

%{
using QuantLib::Handle;
using QuantLib::Math::Interpolation;
using QuantLib::Array;
typedef Handle<Interpolation<Array::const_iterator, Array::const_iterator> > InterpolationHandle;
using QuantLib::IsNull;
%}

// export Handle<Interpolation>
%name(Interpolation) class InterpolationHandle {
  public:
	// no constructor - forbid explicit construction
	~InterpolationHandle();
};

// replicate the Interpolation interface
%addmethods InterpolationHandle {
    #if defined(SWIGPYTHON)
	double __call__(double x) {
		return (**self)(x);
	}
	int __nonzero__() {
		return (IsNull(*self) ? 0 : 1);
	}
    #endif
};


// actual interpolations

%{
using QuantLib::Math::LinearInterpolation;
using QuantLib::Math::CubicSpline;

InterpolationHandle NewLinearInterpolation(const Array& x, const Array& y) {
    return InterpolationHandle(
        new LinearInterpolation<Array::const_iterator,Array::const_iterator>(
            x.begin(),x.end(),y.begin())); }

InterpolationHandle newCubicSpline(const Array& x, const Array& y) {
    return InterpolationHandle(
        new CubicSpline<Array::const_iterator,Array::const_iterator>(
            x.begin(),x.end(),y.begin())); }
%}

%name(LinearInterpolation)	
InterpolationHandle NewLinearInterpolation(const Array& x, const Array& y);
%name(CubicSpline)	
InterpolationHandle newCubicSpline(const Array& x, const Array& y);


#endif
