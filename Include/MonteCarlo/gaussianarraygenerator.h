/*
 * Copyright (C) 2001
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
 * QuantLib license is also available at 
 * http://quantlib.sourceforge.net/LICENSE.TXT
*/

/*! \file gaussianarraygenerator.h
    \brief The best Gaussian random-array generator available in QuantLib

    $Source$
    $Name$
    $Log$
    Revision 1.1  2001/02/02 10:24:38  marmar
    The default gaussian array generator

*/

#ifndef quantlib_gaussian_random_array_generator_h
#define quantlib_gaussian_random_array_generator_h

#include "qldefines.h"
#include "gaussianrandomgenerator.h"
#include "randomarraygenerator.h"

namespace QuantLib {

    namespace MonteCarlo {

    /*! \typedef GaussianArrayGenerator
        Default choice for the gaussian random array generator. 
        See the corresponding classes for more documentation.
    */
        typedef RandomArrayGenerator<GaussianRandomGenerator> 
                                            GaussianArrayGenerator;

    }

}

#endif
