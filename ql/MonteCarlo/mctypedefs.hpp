
/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software developed by the QuantLib Group; you can
 redistribute it and/or modify it under the terms of the QuantLib License;
 either version 1.0, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 QuantLib License for more details.

 You should have received a copy of the QuantLib License along with this
 program; if not, please email ferdinando@ametrano.net

 The QuantLib License is also available at http://quantlib.org/license.html
 The members of the QuantLib Group are listed in the QuantLib License
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
