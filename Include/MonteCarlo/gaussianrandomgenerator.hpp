
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

/*! \file gaussianrandomgenerator.hpp
    \brief the best Gaussian random-number generator available in QuantLib

    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/06 18:46:20  nando
    changed Authors, Contributors, Licence and copyright header

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.4  2001/01/17 14:37:56  nando
    tabs removed

    Revision 1.3  2001/01/04 17:31:22  marmar
    Alpha version of the Monte Carlo tools.

*/

#ifndef ql_gaussian_random_generator_h
#define ql_gaussian_random_generator_h

#include "qldefines.hpp"
#include "uniformrandomgenerator.hpp"
#include "boxmuller.hpp"

namespace QuantLib {

    namespace MonteCarlo {

    /*! \typedef GaussianRandomGenerator
        Default choice for the gaussian random number
        generator. See the corresponding class for documentation.
    */
        typedef BoxMuller<UniformRandomGenerator> GaussianRandomGenerator;

    }

}

#endif
