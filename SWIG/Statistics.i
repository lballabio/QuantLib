
/* 
Copyright (C) 2000 Ferdinando Ametrano, Luigi Ballabio, Adolfo Benin, Marco Marchioro
See the file LICENSE.TXT for information on usage and distribution
Contact ferdinando@ametrano.net if LICENSE.TXT was not distributed with this file
*/

#ifndef quantlib_statistics_i
#define quantlib_statistics_i

%module Statistics

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%include Vectors.i

%{
#include "statistics.h"
QL_USING(QuantLib,Statistics)
%}

class Statistics {
  public:
    Statistics();
    virtual ~Statistics();
    // Accessors
    int samples() const;
    double sampleWeight() const;
    double mean() const;
    // Variance, uses N-1 divisor
    double variance() const;
    // Standard deviation, uses N-1 divisor
    double standardDeviation() const;
    // Skewness (Excel definition).  Zero for normal distribution.
    double skewness() const;
    // Kurtosis (Excel definition). Zero for normal distribution.
    double kurtosis() const;
    // Minimum sample value
    double min() const;
    // Maximum sample value
    double max() const;

    // Modifiers
    void add(double value, double weight = 1.0);
    void reset();
};

%addmethods Statistics {
	void addSequence(DoubleVector values) {
	  self->addSequence(values.begin(), values.end());
	}
	void addWeightedSequence(DoubleVector values, DoubleVector weights) {
	  self->addSequence(values.begin(), values.end(), weights.begin());
	}
}

#endif
