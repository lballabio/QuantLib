
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

/*! \file newcubicspline.cpp
	\brief cubic spline interpolation
	
	$Source$
	$Name$
	$Log$
	Revision 1.5  2000/12/27 14:05:57  lballabio
	Turned Require and Ensure functions into QL_REQUIRE and QL_ENSURE macros

	Revision 1.4  2000/12/14 12:32:31  lballabio
	Added CVS tags in Doxygen file documentation blocks
	
*/

#include "newcubicspline.h"

namespace QuantLib{

	NewCubicSpline::NewCubicSpline( Array xvalues,  Array yvalues, double dy1, double dyn)
	  :xValues(xvalues), yValues(yvalues),coeffs(yvalues){
	   QL_REQUIRE(xValues.size()==yValues.size(),"NewCubicSpline, dimension of x and y arrays must match");

	  int i, n = xValues.size();	
	   QL_REQUIRE(n >= 3,"cubic spline: less than three data points");
	   QL_REQUIRE(isStrictlyAscending(xValues),"interpolation: x values must be strictly ascending");
	   Array u(n-1);
	  if (dy1 > 0.99e+30) {
		  coeffs[0] = u[0] = 0.0;
	  } else {
		  coeffs[0] = -0.5;
		  u[0] = (3.0/double(xValues[1]-xValues[0]))*((yValues[1]-yValues[0])/double(xValues[1]-xValues[0])-dy1);
	  }
	  for (i=1; i<=n-2; ++i) {
		  double sig = double(xValues[i]-xValues[i-1])/double(xValues[i+1]-xValues[i-1]);
		  double p = sig*coeffs[i-1]+2;
		  coeffs[i] = (sig-1)/p;
		  u[i] = (yValues[i+1]-yValues[i])/double(xValues[i+1]-xValues[i]) 
			  - (yValues[i]-yValues[i-1])/double(xValues[i]-xValues[i-1]);
		  u[i] = (6*u[i]/double(xValues[i+1]-xValues[i-1])-sig*u[i-1])/p;

	  }
	  double qn, un;	
	  if (dyn > 0.99e+30) {
		  qn = un = 0.0;
	  } else {
		  qn = 0.5;
		  un = (3.0/double(xValues[n-1]-xValues[n-2]))*(dyn-(yValues[n-1]-yValues[n-2])/double(xValues[n-1]-xValues[n-2]));
	  }
	  coeffs[n-1] = (un-qn*u[n-2])/(qn*coeffs[n-2]+1);
	  for (i=n-2; i>=0; --i)
		  coeffs[i] = coeffs[i]*coeffs[i+1] + u[i];
	}



	double 	NewCubicSpline::value(double x, int guess) const{

		int n = xValues.size();
		int i = (guess == -1 ? location(xValues.begin(), xValues.end(), x) :
			location(xValues.begin(), xValues.end(), x, guess));
		if (i == -1)
			i = 0;
		if (i >= n-1)
			i = n-2;
		double h = double(xValues[i+1]-xValues[i]);
		 QL_REQUIRE(h != 0.0,"interpolation: data points must have different x values");
		double a = double(xValues[i+1]-x)/h, b = 1.0-a;
		return a*yValues[i]+b*yValues[i+1]+((a*a*a-a)*coeffs[i]+(b*b*b-b)*coeffs[i+1])*(h*h)/6;
	}


	double NewCubicSpline::firstDerivative(double x, int guess) const{

		int n = xValues.size();
		int i = (guess == -1 ? location(xValues.begin(), xValues.end(), x) :
			location(xValues.begin(), xValues.end(), x, guess));
		if (i == -1)
			i = 0;
		if (i >= n-1)
			i = n-2;
		double h = double(xValues[i+1]-xValues[i]);
		 QL_REQUIRE(h != 0.0,"interpolation: data points must have different x values");
		double a = double(xValues[i+1]-x)/h, b = 1.0-a;
		return (yValues[i+1]-yValues[i])/h - ((3*a*a-1)*coeffs[i] - (3*b*b-1)*coeffs[i+1])*h/6;
	}


	bool NewCubicSpline::isStrictlyAscending( Array v){
		bool answer = true;
		for(int i=1;i<v.size();i++){
			if(v[i-1]>=v[i])
				answer = false;
		}
		return answer;
	}

}
