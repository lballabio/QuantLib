
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

/*! \file gaussianmultipathgenerator.hpp
    \brief Generates multi paths from random points

    \fullpath
    ql/MonteCarlo/%gaussianmultipathgenerator.hpp
*/

// $Id$

#ifndef quantlib_montecarlo_gaussian_multi_path_generator_h
#define quantlib_montecarlo_gaussian_multi_path_generator_h

#include "ql/MonteCarlo/gaussianpathgenerator.hpp"
#include "ql/MonteCarlo/multipathgenerator.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        /*! \typedef GaussianMultiPathGenerator
            The following is the default choice for the gaussian multi-path
            generator. See the corresponding class for documentation.
        */

        typedef 
        MultiPathGenerator<RandomArrayGenerator<GaussianRandomGenerator> >
            GaussianMultiPathGenerator;
    }

}


#endif
