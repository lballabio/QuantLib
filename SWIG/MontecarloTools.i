
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

/* $Source$
   $Log$
   Revision 1.8  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.7  2001/03/28 13:40:12  marmar
   MultiPathGenerator now has a default for mean

   Revision 1.6  2001/03/15 13:50:26  marmar
   getCovariance function added

   Revision 1.5  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

	Revision 1.4  2001/03/06 17:00:36  marmar
	First, simplified version, of everest option introduced

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
