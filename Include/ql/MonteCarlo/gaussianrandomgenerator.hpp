
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

/*! \file gaussianrandomgenerator.hpp
    \brief Default choice for Gaussian random number generator

    $Id$
*/

// $Source$
// $Log$
// Revision 1.10  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.9  2001/08/07 13:54:16  marmar
// Remarks by Nando
//

#ifndef ql_gaussian_random_generator_h
#define ql_gaussian_random_generator_h

#include "ql/MonteCarlo/uniformrandomgenerator.hpp"
#include "ql/MonteCarlo/boxmuller.hpp"
#include "ql/MonteCarlo/boxmuller.hpp"
#include "ql/MonteCarlo/centrallimitgaussian.hpp"
#include "ql/MonteCarlo/inversecumulativegaussian.hpp"

namespace QuantLib {

    namespace MonteCarlo {

        /*! \typedef GaussianRandomGenerator
            Box-Muller algorithm is the default choice for the gaussian random number
            generator.
            It could have been
                typedef CLGaussian<UniformRandomGenerator> GaussianRandomGenerator;
            or even
                typedef ICGaussian<UniformRandomGenerator> GaussianRandomGenerator;
            that is, respectively, central-limit or inverse-cumulative algorithms.
        */

        typedef BoxMuller<UniformRandomGenerator> GaussianRandomGenerator;
//        typedef CLGaussian<UniformRandomGenerator> GaussianRandomGenerator;
//        typedef ICGaussian<UniformRandomGenerator> GaussianRandomGenerator;

    }

}


#endif
