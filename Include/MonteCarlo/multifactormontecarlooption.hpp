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

/*! \file multifactormontecarlooption.hpp
    $Source$
    $Name$
    $Log$
    Revision 1.2  2001/04/04 12:13:23  nando
    Headers policy part 2:
    The Include directory is added to the compiler's include search path.
    Then both your code and user code specifies the sub-directory in
    #include directives, as in
    #include <Solvers1d/newton.hpp>

    Revision 1.1  2001/04/04 11:07:22  nando
    Headers policy part 1:
    Headers should have a .hpp (lowercase) filename extension
    All *.h renamed to *.hpp

    Revision 1.2  2001/02/02 10:47:00  marmar
    Specialization of PathMonteCarlo useful for pricing option
    depending on  multiple factors

    Revision 1.1  2001/01/30 15:46:32  marmar
    Special cases of a PathMonteCarlo defined for convenience in
    single- and multi-factor Monte Carlo option-pricing

*/

#ifndef quantlib_multi_factor_montecarlo_option_h
#define quantlib_multi_factor_montecarlo_option_h

#include "qldefines.hpp"
#include "Math/statistics.hpp"
#include "multipathpricer.hpp"
#include "standardmultipathgenerator.hpp"
#include "pathmontecarlo.hpp"

namespace QuantLib {

    namespace MonteCarlo {
    /*! \typedef MultiFactorMonteCarloOption
    This typedef builds a class that contains the basic features
    of a Monte Carlo pricer based on a multi-factor model.
    See the corresponding classes for more documentation.
    */
        typedef PathMonteCarlo<Math::Statistics,
                    StandardMultiPathGenerator, MultiPathPricer>
                                        MultiFactorMonteCarloOption;

    }

}

#endif
