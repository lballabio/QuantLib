/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Nicolas Di Césaré
 Copyright (C) 2009 Frédéric Degraeve

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

/*! \file steepestdescent.hpp
    \brief Steepest descent optimization method
*/

#ifndef quantlib_optimization_steepest_descent_h
#define quantlib_optimization_steepest_descent_h

#include <ql/math/optimization/linesearchbasedmethod.hpp>

namespace QuantLib {

    //! Multi-dimensional steepest-descent class
    /*! User has to provide line-search method and optimization end criteria

        search direction \f$ = - f'(x) \f$
    */
    class SteepestDescent : public LineSearchBasedMethod {
      public:
        SteepestDescent(const std::shared_ptr<LineSearch>& lineSearch =
                                            std::shared_ptr<LineSearch>())
        : LineSearchBasedMethod(lineSearch) {}
      private:
        //! \name LineSearchBasedMethod interface
        //@{
        Array getUpdatedDirection(const Problem& P, Real gold2, const Array& oldGradient) override;
        //@}
    };

}

#endif
