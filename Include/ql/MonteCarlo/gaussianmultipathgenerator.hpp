
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

    \fullpath
    Include/ql/MonteCarlo/%gaussianmultipathgenerator.hpp
    \brief Generates multi paths from random points

*/

// $Id$
// $Log$
// Revision 1.5  2001/08/31 15:23:45  sigmud
// refining fullpath entries for doxygen documentation
//
// Revision 1.4  2001/08/22 10:57:10  nando
// removed useless gaussianarraygenerator.hpp
// It was a clone of gaussianpathgenerator.hpp
//
// Revision 1.3  2001/08/09 14:59:46  sigmud
// header modification
//
// Revision 1.2  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
// Revision 1.1  2001/08/07 17:33:03  nando
// 1) StandardPathGenerator now is GaussianPathGenerator;
// 2) StandardMultiPathGenerator now is GaussianMultiPathGenerator;
// 3) PathMonteCarlo now is MonteCarloModel;
// 4) added ICGaussian, a Gaussian distribution that use
//    QuantLib::Math::InvCumulativeNormalDistribution to convert uniform
//    distribution extractions into gaussian distribution extractions;
// 5) added a few trailing underscore to private members
// 6) style enforced here and there ....
//
// Revision 1.7  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.6  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.5  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.4  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

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

        typedef MultiPathGenerator<GaussianPathGenerator>
                                              GaussianMultiPathGenerator;
    }
}


#endif
