

/*
 Copyright (C) 2000, 2001, 2002 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
/*! \file rngtypedefs.hpp
    \brief Default choices for template instantiations

    \fullpath
    ql/RandomNumbers/%rngtypedefs.hpp
*/

// $Id$

#ifndef quantlib_rng_typedefs_h
#define quantlib_rng_typedefs_h

//#include <ql/RandomNumbers/brodasoboluniformlds.hpp>
#include <ql/RandomNumbers/lecuyeruniformrng.hpp>
#include <ql/RandomNumbers/boxmullergaussianrng.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        //! default choice for uniform random number generator.
        typedef LecuyerUniformRng UniformRandomGenerator;

        //! default choice for Gaussian random number generator.
        typedef BoxMullerGaussianRng<UniformRandomGenerator> GaussianRandomGenerator;

        //! default choice for Gaussian array generator.
        typedef RandomArrayGenerator<GaussianRandomGenerator> GaussianArrayGenerator;

        // default choice for uniform low discepancy sequence generator.
        // typedef BrodaSobolUniformLds UniformLowDiscrepancy;

        // default choice for Gaussian low discepancy sequence generator.
	// typedef BoxMullerGaussianRng<UniformLowDiscrepancy> GaussianLowDiscrepancy;
    }

}


#endif
