
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
   Revision 1.6  2001/04/06 18:46:21  nando
   changed Authors, Contributors, Licence and copyright header

   Revision 1.5  2001/03/09 12:40:41  lballabio
   Spring cleaning for SWIG interfaces

*/

#ifndef quantlib_random_generators_i
#define quantlib_random_generators_i

%{
using QuantLib::MonteCarlo::UniformRandomGenerator;
using QuantLib::MonteCarlo::GaussianRandomGenerator;
%}

class UniformRandomGenerator {
  public:
    UniformRandomGenerator(long seed=0);
    ~UniformRandomGenerator();
    double next() const;
    double weight() const;
};

class GaussianRandomGenerator {
  public:
    GaussianRandomGenerator(long seed=0);
    ~GaussianRandomGenerator();
    double next() const;
    double weight() const;
};


#endif
