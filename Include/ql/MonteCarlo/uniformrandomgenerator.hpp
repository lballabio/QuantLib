
/*
 * Copyright (C) 2000-2001 QuantLib Group
 *
 * This file is part of QuantLib.
 * QuantLib is a C++ open source library for financial quantitative
 * analysts and developers --- http://quantlib.org/
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
 * if not, please email quantlib-users@lists.sourceforge.net
 * The license is also available at http://quantlib.org/LICENSE.TXT
 *
 * The members of the QuantLib Group are listed in the Authors.txt file, also
 * available at http://quantlib.org/group.html
*/

/*! \file uniformrandomgenerator.hpp
    \brief Default choice for uniform random number generator

    \fullpath
    Include/ql/MonteCarlo/%uniformrandomgenerator.hpp
*/

// $Id$

#ifndef quantlib_uniform_random_generator_h
#define quantlib_uniform_random_generator_h

#include "ql/MonteCarlo/lecuyerrandomgenerator.hpp"
#include "ql/MonteCarlo/knuthrandomgenerator.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        /*! \typedef UniformRandomGenerator
            The following is the default choice for the uniform random number
            generator. See the corresponding class for documentation.

            It could have been
               typedef KnuthRandomGenerator UniformRandomGenerator;
            using Knuth algorithms.
        */
        typedef LecuyerRandomGenerator UniformRandomGenerator;
//        typedef KnuthRandomGenerator UniformRandomGenerator;

    }

}


#endif
