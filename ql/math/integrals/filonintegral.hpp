/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2014 Klaus Spanderen

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

/*! \file filonintegral.hpp
    \brief Filon's formulae for sine and cosine Integrals
*/

#ifndef quantlib_filon_integral_h
#define quantlib_filon_integral_h

#include <ql/math/integrals/integral.hpp>

namespace QuantLib {

    //! Integral of a one-dimensional function
    /*! Given a number \f$ N \f$ of intervals, the integral of
        a function \f$ f \f$ between \f$ a \f$ and \f$ b \f$ is
        calculated by means of Filon's sine and cosine integrals
    */

    /*! References:
        Abramowitz, M. and Stegun, I. A. (Eds.).
        Handbook of Mathematical Functions with Formulas, Graphs,
        and Mathematical Tables, 9th printing. New York: Dover,
        pp. 890-891, 1972.

        \test the correctness of the result is tested by checking it
              against known good values.
    */
    class FilonIntegral : public Integrator {
      public:
        enum Type { Sine, Cosine };
        FilonIntegral(Type type, Real t, Size intervals);

      protected:
        Real integrate(const ext::function<Real(Real)>& f, Real a, Real b) const override;

      private:
        const Type type_;
        const Real t_;
        const Size intervals_, n_;
    };
}

#endif


#ifndef id_88d7ceca68b67d96741f1d9e67256949
#define id_88d7ceca68b67d96741f1d9e67256949
inline bool test_88d7ceca68b67d96741f1d9e67256949(int* i) { return i != 0; }
#endif
