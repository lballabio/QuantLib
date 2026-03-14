/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
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

/*! \file conjugategradient.hpp
    \brief Conjugate gradient optimization method
*/

#ifndef quantlib_optimization_conjugate_gradient_h
#define quantlib_optimization_conjugate_gradient_h

#include <ql/math/optimization/linesearchbasedmethod.hpp>

namespace QuantLib {

    //! Multi-dimensional Conjugate Gradient class.
    /*! Fletcher-Reeves-Polak-Ribiere algorithm
        adapted from Numerical Recipes in C, 2nd edition.

        User has to provide line-search method and optimization end criteria.
        Search direction \f$ d_i = - f'(x_i) + c_i*d_{i-1} \f$
        where \f$ c_i = ||f'(x_i)||^2/||f'(x_{i-1})||^2 \f$
        and \f$ d_1 = - f'(x_1) \f$

        This optimization method requires the knowledge of
        the gradient of the cost function.

        \ingroup optimizers
    */
    class ConjugateGradient: public LineSearchBasedMethod {
      public:
        ConjugateGradient(const ext::shared_ptr<LineSearch>& lineSearch =
                                            ext::shared_ptr<LineSearch>())
        : LineSearchBasedMethod(lineSearch) {}
      private:
        //! \name LineSearchBasedMethod interface
        //@{
        Array getUpdatedDirection(const Problem& P, Real gold2, const Array& oldGradient) override;
        //@}
    };

}

#endif
