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
/*! \file standardpathgenerator.h
    \brief Generates path from random points

    $Source$
    $Name$
    $Log$
    Revision 1.3  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.2  2001/01/17 11:54:02  marmar
    Some documentation added and 80 columns format enforced.

    Revision 1.1  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/


#ifndef quantlib_montecarlo_standard_path_generator_h
#define quantlib_montecarlo_standard_path_generator_h

#include "gaussianrandomgenerator.h"
#include "pathgenerator.h"

namespace QuantLib {

    namespace MonteCarlo {

    /*! \typedef StandardPathGenerator
        The following is the default choice for the standard path generator
        See the corresponding class for documentation.
    */

        typedef PathGenerator<GaussianRandomGenerator > StandardPathGenerator;

    }
}

#endif
