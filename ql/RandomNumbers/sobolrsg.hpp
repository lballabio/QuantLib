
/*
 Copyright (C) 2003 Ferdinando Ametrano

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

/*! \file sobolrsg.hpp
    \brief Sobol low-discrepancy sequence generator

    \fullpath
    ql/RandomNumbers/%sobolrsg.hpp
*/

// $Id$

#ifndef quantlib_sobol_ld_rsg_h
#define quantlib_sobol_ld_rsg_h

#include <ql/array.hpp>
#include <ql/MonteCarlo/sample.hpp>
#include <vector>

namespace QuantLib {

    namespace RandomNumbers {
        //! Sobol low-discrepancy sequence generator
        /*! Sobol low-discrepancy sequence generator using the Gray code
            counter and bitwise operations for very fast sequence generation.

            The implementation relies on primitive polynomials modulo two and
            initialization numbers from the book "Monte Carlo Methods in
            Finance" by Peter Jäckel

            Primitive polynomials modulo two are in the separate library
            PrimitivePolynomialsModuloTwo of the QuantLib CVS. There are
            8 129 334 polynomials as provided by Jäckel, but they are compiled
            only up to PPMT_MAX_DIM. If you need higher dimension you must
            recompile the PrimitivePolynomialsModuloTwo library.

            The choice of initialization numbers is crucial for the homogeneity
            properties of the sequence. Jäckel's initialization numbers are
            superior to the "unit initialization" suggested in "Numerical
            Recipes in C" by Press, Teukolsky, Vetterling, and Flannery.

            For more info on Sobol sequences see "Monte Carlo Methods in
            Finance", by Peter Jäckel, section 8.3 and "Numerical Recipes in
            C", 2nd edition, by Press, Teukolsky, Vetterling, and Flannery,
            section 7.7
        */
        class SobolRsg {
          public:
            typedef MonteCarlo::Sample<Array> sample_type;
            // dimensionality must be <= PPMT_MAX_DIM
            SobolRsg(Size dimensionality,
                     unsigned long seed = 0);
            const sample_type& nextSequence() const;
            const sample_type& lastSequence() const {
                return sequence_;
            }
            Size dimension() const {return dimensionality_;}
          private:
            static const int bits_;
            static const double normalizationFactor_;
            unsigned long dimensionality_;
            mutable unsigned long sequenceCounter_;
            mutable sample_type sequence_;
            mutable std::vector<unsigned long> integerSequence_;
            std::vector<std::vector<unsigned long> > directionIntegers_;
        };

    }

}

#endif
