/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2012 Klaus Spanderen

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

/*! \file richardsonextrapolation.hpp
*/

#ifndef quantlib_richardson_extrapolation_hpp
#define quantlib_richardson_extrapolation_hpp

#include <ql/types.hpp>

#include <boost/function.hpp>

namespace QuantLib {

    //! Richardson Extrapolatio
    /*! References:

        http://en.wikipedia.org/wiki/Richardson_extrapolation
     */

    class RichardsonExtrapolation {
      public:
        RichardsonExtrapolation(Real h,
                                const boost::function<Real (Real)>& f);

        /*! Extrapolation for known order of convergence
            \param  step size for extrapolation. Is often 2
            \param k order of convergence
        */
        Real formula(Real t, Real k) const;

        /*! Extrapolation for unknown order of convergence
            \param t first step size for the extrapolation.
            \param s second step size for the extrapolation
        */
        Real formulaUnknownConvergence(Real t, Real s) const;

      private:
        const Real h_;
        const Real fh_;
        const boost::function<Real (Real)> f_;
    };
}

#endif
