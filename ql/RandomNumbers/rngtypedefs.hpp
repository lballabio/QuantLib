

/*
 Copyright (C) 2002 Ferdinando Ametrano
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

#include <ql/RandomNumbers/lecuyeruniformrng.hpp>
#include <ql/RandomNumbers/knuthuniformrng.hpp>
#include <ql/RandomNumbers/boxmullergaussianrng.hpp>
#include <ql/RandomNumbers/centrallimitgaussianrng.hpp>
#include <ql/RandomNumbers/inversecumgaussianrng.hpp>
#include <ql/RandomNumbers/randomarraygenerator.hpp>

namespace QuantLib {

    namespace RandomNumbers {

        // Gaussian random number generators based on
        // Lecuyer uniform random number generator
        typedef BoxMullerGaussianRng<LecuyerUniformRng>
			BoxMullerLecuyerGaussianRng;
        typedef CLGaussianRng<LecuyerUniformRng>
			CentralLimitLecuyerGaussianRng;
        typedef ICGaussianRng<LecuyerUniformRng,
            QuantLib::Math::InvCumulativeNormalDistribution>
			InvCumulativeLecuyerGaussianRng;

        // Gaussian random number generators based on
        // Knuth uniform random number generator
        typedef BoxMullerGaussianRng<KnuthUniformRng>
			BoxMullerKnuthGaussianRng;
        typedef CLGaussianRng<KnuthUniformRng>
			CentralLimitKnuthGaussianRng;
        typedef ICGaussianRng<KnuthUniformRng,
            QuantLib::Math::InvCumulativeNormalDistribution>
			InvCumulativeKnuthGaussianRng;

/*      // looking forward to low-discrepancy sequences

        // default choice for Gaussian low discrepancy sequence generator.
        typedef ICGaussianRng<SobolUniformLds,
            QuantLib::Math::InvCumulativeNormalDistribution>
			InvCumulativeSobolGaussianLds;
*/

        //! default choice for uniform random number generator.
        typedef LecuyerUniformRng UniformRandomGenerator;

        //! default choice for Gaussian random number generator.
        typedef BoxMullerGaussianRng<UniformRandomGenerator>
			GaussianRandomGenerator;
        // It might be substitued by InvCumulativeLecuyerGaussianRng

        //! default choice for Gaussian array generator.
        typedef RandomArrayGenerator<GaussianRandomGenerator>
			GaussianArrayGenerator;

/*      //looking forward to low-discrepancy sequences

        // default choice for uniform low discrepancy sequence generator.
        typedef SobolUniformLds UniformLowDiscrepancy;
*/


    }

}


#endif
