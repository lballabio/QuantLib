
/*
 Copyright (C) 2002, 2003 Ferdinando Ametrano
 Copyright (C) 2000, 2001, 2002, 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file rngtypedefs.hpp
    \brief Default choices for template instantiations
*/

#ifndef quantlib_rng_typedefs_h
#define quantlib_rng_typedefs_h

#include <ql/RandomNumbers/rngtraits.hpp>

namespace QuantLib {

    /* Some default choices. Kind of redundant with the default
       choices made in mctraits---and in fact, defined depending
       on the former. Personally, I'd dump these ones and keep 
       the traits only. It's not as we're preventing the user 
       from creating his own typedefs (and with his own names) 
       if he feels the itch.
                                             LB
    */

    //! default choice for uniform random number generator.
    typedef PseudoRandom::urng_type UniformRandomGenerator;

    //! default choice for Gaussian random number generator.
    typedef PseudoRandom::rng_type GaussianRandomGenerator;

    //! default choice for Gaussian array generator.
    typedef RandomArrayGenerator<GaussianRandomGenerator>
                                                      GaussianArrayGenerator;

    //! default choice for uniform random sequence generator.
    typedef PseudoRandom::ursg_type UniformRandomSequenceGenerator;

    //! default choice for Gaussian random sequence generator.
    typedef PseudoRandom::rsg_type GaussianRandomSequenceGenerator;

    typedef LowDiscrepancy::ursg_type UniformLowDiscrepancySequenceGenerator;

    typedef LowDiscrepancy::rsg_type GaussianLowDiscrepancySequenceGenerator;

}


#endif
