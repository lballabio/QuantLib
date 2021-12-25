/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2017 Klaus Spanderen

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

/*! \file gmres.hpp
    \brief generalized minimal residual method
*/

#ifndef quantlib_gmres_hpp
#define quantlib_gmres_hpp

#include <ql/math/array.hpp>
#include <ql/functional.hpp>

#include <list>

namespace QuantLib {

    /*! References:
        Saad, Yousef. 1996, Iterative methods for sparse linear systems,
        http://www-users.cs.umn.edu/~saad/books.html

        Dongarra et al. 1994,
        Templates for the Solution of Linear Systems: Building Blocks
        for Iterative Methods, 2nd Edition, SIAM, Philadelphia
        http://www.netlib.org/templates/templates.pdf

        Christian Kanzow
        Numerik linearer Gleichungssysteme (German)
        Chapter 6: GMRES und verwandte Verfahren
        http://bilder.buecher.de/zusatz/12/12950/12950560_lese_1.pdf
    */

    struct GMRESResult {
        std::list<Real> errors;
        Array x;
    };

    class GMRES  {
      public:
        typedef ext::function<Disposable<Array>(const Array&)> MatrixMult;

        GMRES(MatrixMult A, Size maxIter, Real relTol, MatrixMult preConditioner = MatrixMult());

        GMRESResult solve(const Array& b, const Array& x0 = Array()) const;
        GMRESResult solveWithRestart(
            Size restart, const Array& b, const Array& x0 = Array()) const;

      protected:
        GMRESResult solveImpl(const Array& b, const Array& x0) const;

        const MatrixMult A_, M_;
        const Size maxIter_;
        const Real relTol_;
    };

}

#endif
