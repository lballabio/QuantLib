
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

/*! \file multifactormontecarlooption.hpp
    \fullpath Include/ql/MonteCarlo/%multifactormontecarlooption.hpp
    \brief basic Monte Carlo pricer based on a multi-factor model

    $Id$
*/

// $Source$
// $Log$
// Revision 1.11  2001/08/08 11:07:49  sigmud
// inserting \fullpath for doxygen
//
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
// Revision 1.9  2001/08/07 11:25:54  sigmud
// copyright header maintenance
//
// Revision 1.8  2001/07/25 15:47:28  sigmud
// Change from quantlib.sourceforge.net to quantlib.org
//
// Revision 1.7  2001/07/19 16:40:10  lballabio
// Improved docs a bit
//
// Revision 1.6  2001/06/22 16:38:15  lballabio
// Improved documentation
//
// Revision 1.5  2001/05/24 15:38:08  nando
// smoothing #include xx.hpp and cutting old Log messages
//

#ifndef quantlib_multi_factor_montecarlo_option_h
#define quantlib_multi_factor_montecarlo_option_h

#include "ql/Math/statistics.hpp"
#include "ql/MonteCarlo/multipathpricer.hpp"
#include "ql/MonteCarlo/gaussianmultipathgenerator.hpp"
#include "ql/MonteCarlo/montecarlomodel.hpp"

namespace QuantLib {

    namespace MonteCarlo {
        
        /*! \typedef MultiFactorMonteCarloOption
            This typedef builds a class that contains the basic features
            of a Monte Carlo pricer based on a multi-factor model.
        */
        typedef MonteCarloModel<Math::Statistics,
                    GaussianMultiPathGenerator, MultiPathPricer>
                                        MultiFactorMonteCarloOption;
    }

}


#endif
