
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
    Revision 1.9  2001/04/09 12:24:58  nando
    updated copyright notice header and improved CVS tags

*/

#ifndef shaft_Montecarlo_Tools_i
#define shaft_Montecarlo_Tools_i

%include QLArray.i
%include Vectors.i

%{
using QuantLib::MonteCarlo::GaussianArrayGenerator;
using QuantLib::MonteCarlo::StandardPathGenerator;
using QuantLib::MonteCarlo::StandardMultiPathGenerator;
using QuantLib::MonteCarlo::getCovariance;
%}

class GaussianArrayGenerator {
  public:
	GaussianArrayGenerator(const Array& average,
                           const Matrix& covariance,
                           long seed=0);
    ~GaussianArrayGenerator();
	Array next() const;
	double weight() const;
};


class StandardPathGenerator {
  public:
	StandardPathGenerator(int dimension, long seed=0);
	~StandardPathGenerator();
	Array next() const; // Note that currently Path and Array are equivalent
	double weight() const;
};


class StandardMultiPathGenerator {
  public:
	StandardMultiPathGenerator(const DoubleVector& timeDelays,
                               const Matrix& covariance,
	                           const Array& average,
                               long seed=0);
    ~StandardMultiPathGenerator();
	Matrix next() const;
	double weight() const;
};

Matrix getCovariance(const Array &volatilities, const Matrix &correlations);

#endif
