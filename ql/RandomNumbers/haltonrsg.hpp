
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

/*! \file haltonrsg.hpp
    \brief Halton low-discrepancy sequence generator
*/

// $Id$

#ifndef quantlib_halton_ld_rsg_h
#define quantlib_halton_ld_rsg_h

#include <ql/array.hpp>
#include <ql/MonteCarlo/sample.hpp>
#include <cmath>

namespace QuantLib {

    namespace RandomNumbers {
        //! Halton low-discrepancy sequence generator
        /*! Halton algorithm for low-discrepancy sequence.
            For more details see chapter 8, paragraph 2 of
            "Monte Carlo Methods in Finance", by Peter Jäckel
        */
        class HaltonRsg {
          public:
            typedef MonteCarlo::Sample<Array> sample_type;
            HaltonRsg(Size dimensionality)
            : dimensionality_(dimensionality), sequenceCounter_(0),
              sequence_(Array(dimensionality), 1.0) {}

            const sample_type& nextSequence() const;
            const sample_type& lastSequence() const {
                return sequence_;
            }
            Size dimension() const {return dimensionality_;}
          private:
            Size dimensionality_;
            mutable Size sequenceCounter_;
            mutable sample_type sequence_;
        };
    }

}

#endif
