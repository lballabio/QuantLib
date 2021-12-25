/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 François du Vignaud
 Copyright (C) 2007 Giorgio Facchinetti
 Copyright (C) 2013 Peter Caspers

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file projection.hpp
    \brief Parameter projection
*/

#ifndef quantlib_math_projection_h
#define quantlib_math_projection_h

#include <ql/math/array.hpp>

namespace QuantLib {

    class Projection {
      public:
        Projection(const Array& parameterValues,
                   std::vector<bool> fixParameters = std::vector<bool>());

        //! returns the subset of free parameters corresponding
        // to set of parameters
        virtual Disposable<Array> project(const Array &parameters) const;

        //! returns whole set of parameters corresponding to the set
        // of projected parameters
        virtual Disposable<Array>
        include(const Array &projectedParameters) const;
        virtual ~Projection() = default;

      protected:
        void mapFreeParameters(const Array &parameterValues) const;
        Size numberOfFreeParameters_;
        const Array fixedParameters_;
        mutable Array actualParameters_;
        std::vector<bool> fixParameters_;
    };

}

#endif
