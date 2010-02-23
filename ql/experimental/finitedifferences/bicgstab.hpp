/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2009 Klaus Spanderen

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

/*! \file bicgstab.hpp
    \brief bi-conjugated gradient stableized algorithm
*/

#ifndef quantlib_bicgstab_hpp
#define quantlib_bicgstab_hpp

#include <ql/math/array.hpp>
#include <boost/function.hpp>

namespace QuantLib {

    struct BiCGStabResult {
        Size iterations;
        Real error;
        Array x;
    };

    class BiCGstab  {
      public:
        typedef boost::function1<Disposable<Array> , const Array& > MatrixMult;
        
        BiCGstab(const MatrixMult& A, Size maxIter, Real relTol,
                 const MatrixMult& preConditioner = MatrixMult());
        
        BiCGStabResult solve(const Array& b, const Array& x0 = Array()) const;
        
      protected:
        Real norm2(const Array& a) const;
        
        const MatrixMult A_, M_;
        const Size maxIter_;
        const Real relTol_;  
    };
}

#endif
