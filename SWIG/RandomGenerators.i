
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

#ifndef quantlib_random_generators_i
#define quantlib_random_generators_i

%module RandomGenerators

%{
#include "quantlib.h"
%}

#if !defined(SWIGPYTHON)
#if !defined(PYTHON_WARNING_ISSUED)
#define PYTHON_WARNING_ISSUED
%echo "Warning: this is a Python module!!"
%echo "Exporting it to any other language is not advised as it could lead to unpredicted results."
#endif
#endif

%{
using QuantLib::MonteCarlo::UniformRandomGenerator;
using QuantLib::MonteCarlo::GaussianRandomGenerator;
%}

class UniformRandomGenerator {
  public:
    UniformRandomGenerator(long seed=0);
    double next() const;
    double weight() const;
};

class GaussianRandomGenerator {
  public:
    GaussianRandomGenerator(long seed=0);
    double next() const;
    double weight() const;
};

%{
using QuantLib::MonteCarlo::UniformRandomGenerator;
using QuantLib::MonteCarlo::BoxMuller;
typedef BoxMuller<UniformRandomGenerator> BM;
%}

class BM {
  public:
    BM(long seed=0);
    double next() const;
    double weight() const;
};

#endif
