
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
    Revision 1.20  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef quantlib_statistics_i
#define quantlib_statistics_i

#if defined (SWIGPYTHON)
%include QLArray.i
#endif
%include Vectors.i

%{
using QuantLib::Math::Statistics;
%}

class Statistics {
  public:
    Statistics();
    ~Statistics();
    // Accessors
    int samples() const;
    double weightSum() const;
    double mean() const;
    double variance() const;
    double standardDeviation() const;
    double errorEstimate() const;
    double skewness() const;
    double kurtosis() const;
    double min() const;
    double max() const;
    // Modifiers
    void add(double value, double weight = 1.0);
    void reset();
};

%addmethods Statistics {
    #if defined(SWIGPYTHON) || defined(SWIGRUBY)
    void addSequence(DoubleVector values) {
        self->addSequence(values.begin(), values.end());
    }
    void addWeightedSequence(DoubleVector values, DoubleVector weights) {
        self->addSequence(values.begin(), values.end(), weights.begin());
    }
    #endif
}

#if defined(SWIGPYTHON)
%include Matrix.i

%{
using QuantLib::Math::MultivariateAccumulator;
%}

class MultivariateAccumulator {
  public:
    MultivariateAccumulator();
    int size() const;
    double samples() const;
    Array mean() const;
    Matrix covariance() const;
    void add(const Array& a, double weight = 1.0);
    void reset();
};
#endif


#endif

