
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

/*! \file mctypedefs.hpp
    \brief Default choices for template instantiations

    \fullpath
    ql/MonteCarlo/%mctypedefs.hpp
*/

// $Id$

#ifndef quantlib_mc_typedefs_h
#define quantlib_mc_typedefs_h

#include <ql/RandomNumbers/rngtypedefs.hpp>
#include <ql/MonteCarlo/pathgenerator.hpp>
#include <ql/MonteCarlo/multipathgenerator.hpp>
#include <ql/MonteCarlo/pathpricer.hpp>
#include <ql/MonteCarlo/montecarlomodel.hpp>
#include <ql/Math/statistics.hpp>

namespace QuantLib {

    namespace MonteCarlo {

        //! default choice for Gaussian path generator.
        typedef PathGenerator<RandomNumbers::GaussianRandomGenerator>
            GaussianPathGenerator;

        //! default choice for Gaussian multi-path generator.
        typedef
        MultiPathGenerator<RandomNumbers::RandomArrayGenerator<
            RandomNumbers::GaussianRandomGenerator> >
            GaussianMultiPathGenerator;

        //! default choice for one-factor Monte Carlo model.
        typedef MonteCarloModel<Math::Statistics,
                                GaussianPathGenerator,
                                PathPricer<Path> >
                                    OneFactorMonteCarloOption;

        //! default choice for multi-factor Monte Carlo model.
        typedef MonteCarloModel<Math::Statistics,
                                GaussianMultiPathGenerator,
                                PathPricer<MultiPath> >
                                    MultiFactorMonteCarloOption;

    }

}


#endif
