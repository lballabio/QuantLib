
/*
 * Copyright (C) 2000-2001 QuantLib Group
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
 * The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.sourceforge.net/Authors.txt
*/

/*  $Source$
	$Log$
	Revision 1.6  2001/04/06 18:46:21  nando
	changed Authors, Contributors, Licence and copyright header

	Revision 1.5  2001/03/12 17:35:11  lballabio
	Removed global IsNull function - could have caused very vicious loops

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
		return (self->isNull() ? 0 : 1);
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
