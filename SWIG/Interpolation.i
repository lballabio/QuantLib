
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

/*
    $Id$
    $Source$
    $Log$
    Revision 1.8  2001/04/12 15:52:33  lballabio
    Rubified interpolation (and reworked it in Python too)

    Revision 1.7  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_interpolation_i
#define quantlib_interpolation_i

%include QLArray.i

%{
using QuantLib::Handle;
using QuantLib::Math::Interpolation;
using QuantLib::Math::LinearInterpolation;
using QuantLib::Math::CubicSpline;
using QuantLib::Array;
typedef Handle<Interpolation<Array::const_iterator, Array::const_iterator> > 
	InterpolationHandle;
typedef Handle<Interpolation<Array::const_iterator, Array::const_iterator> > 
	LinearInterpolationHandle;
typedef Handle<Interpolation<Array::const_iterator, Array::const_iterator> > 
	CubicSplineInterpolationHandle;
%}

// export Handle<Interpolation>
%name(Interpolation) class InterpolationHandle {
  public:
	// no constructor - forbid explicit construction
	~InterpolationHandle();
};

// replicate the Interpolation interface
%addmethods InterpolationHandle {
    #if defined(SWIGRUBY)
        void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
	double __call__(double x) {
		return (**self)(x);
	}
    #endif
    #if defined(SWIGPYTHON)
	int __nonzero__() {
		return (self->isNull() ? 0 : 1);
	}
    #endif
};


// actual interpolations

%name(LinearInterpolation)
class LinearInterpolationHandle : public InterpolationHandle {
  public:
    // constructor redefined below
    ~LinearInterpolationHandle();
};

%addmethods LinearInterpolationHandle {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    LinearInterpolationHandle(const Array& x, const Array& y) {
        return new LinearInterpolationHandle(
        new LinearInterpolation<Array::const_iterator,Array::const_iterator>(
            x.begin(),x.end(),y.begin())); 
    }
    #endif
}

%name(CubicSpline)
class CubicSplineInterpolationHandle : public InterpolationHandle {
  public:
    // constructor redefined below
    ~CubicSplineInterpolationHandle();
};

%addmethods CubicSplineInterpolationHandle {
    #if defined(SWIGRUBY)
    void crash() {}
    #endif
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    CubicSplineInterpolationHandle(const Array& x, const Array& y) {
        return new CubicSplineInterpolationHandle(
        new CubicSpline<Array::const_iterator,Array::const_iterator>(
            x.begin(),x.end(),y.begin()));
    }
    #endif
}


#endif
