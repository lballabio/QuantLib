/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Frédéric Degraeve

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file bfgs.hpp
    \brief Broyden-Fletcher-Goldfarb-Shanno optimization method
*/

#ifndef quantlib_optimization_bfgs_hpp
#define quantlib_optimization_bfgs_hpp

#include <ql/math/optimization/linesearchbasedmethod.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib {

    //! Broyden-Fletcher-Goldfarb-Shanno algorithm
    /*! See <http://en.wikipedia.org/wiki/BFGS_method>.

        Adapted from Numerical Recipes in C, 2nd edition.

        User has to provide line-search method and optimization end criteria.
    */
    class BFGS: public LineSearchBasedMethod {
      public:
        BFGS(const ext::shared_ptr<LineSearch>& lineSearch =
                                              ext::shared_ptr<LineSearch>())
        : LineSearchBasedMethod(lineSearch) {}
      private:
        //! \name LineSearchBasedMethod interface
        //@{
        Array getUpdatedDirection(const Problem& P, Real gold2, const Array& oldGradient) override;
        //@}
        //! inverse of hessian matrix
        Matrix inverseHessian_;
    };

}

#endif
