
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_normal_distribution_h
#define quantlib_normal_distribution_h

#include "qldefines.h"
#include <functional>
#include <cmath>

QL_BEGIN_NAMESPACE(QuantLib)

class NormalDistribution : public QL_ADD_NAMESPACE(std,unary_function)<double,double> {
  public:
	NormalDistribution(double average = 0.0, double sigma = 1.0);
	// function
	double operator()(double x);
  private:
	static const double pi;
	double average, sigma, normalizationFactor, denominator;
};

typedef NormalDistribution GaussianDistribution;

class CumulativeNormalDistribution : public QL_ADD_NAMESPACE(std,unary_function)<double,double> {
  public:
	CumulativeNormalDistribution();
	// function
	double operator()(double x);
	double derivative(double x);
  private:
	static const double a1, a2, a3, a4, a5, gamma, precision;
	NormalDistribution gaussian;
};

// inline definitions

inline NormalDistribution::NormalDistribution(double average, double sigma)
: average(average), sigma(sigma) {
	normalizationFactor = 1.0/(sigma*QL_SQRT(2.0*pi));
	denominator = 2.0*sigma*sigma;
}

inline double NormalDistribution::operator()(double x) {
	double deltax = x-average;
	return normalizationFactor*QL_EXP(-deltax*deltax/denominator);
}

inline CumulativeNormalDistribution::CumulativeNormalDistribution() {}

inline double CumulativeNormalDistribution::derivative(double x) {
	return gaussian(x);
}

QL_END_NAMESPACE(QuantLib)


#endif
