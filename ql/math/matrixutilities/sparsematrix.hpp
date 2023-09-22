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

/*! \file sparsematrix.hpp
    \brief typedef for boost sparse matrix class
*/

#ifndef quantlib_sparse_matrix_hpp
#define quantlib_sparse_matrix_hpp

#include <ql/qldefines.hpp>
#include <ql/errors.hpp>
#include <ql/math/array.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4180)
#pragma warning(disable:4127)
#endif

#if BOOST_VERSION == 106400
#include <boost/serialization/array_wrapper.hpp>
#endif

#include <boost/numeric/ublas/matrix_sparse.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

namespace QuantLib {

    typedef boost::numeric::ublas::compressed_matrix<Real> SparseMatrix;
    typedef boost::numeric::ublas::matrix_reference<SparseMatrix> SparseMatrixReference;

    inline Array prod(const SparseMatrix& A, const Array& x) {
        QL_REQUIRE(x.size() == A.size2(),
                   "vectors and sparse matrices with different sizes ("
                   << x.size() << ", " << A.size1() << "x" << A.size2() <<
                   ") cannot be multiplied");

        Array b(x.size(), 0.0);

        for (Size i=0; i < A.filled1()-1; ++i) {
            const Size begin = A.index1_data()[i];
            const Size end   = A.index1_data()[i+1];
            Real t=0;
            for (Size j=begin; j < end; ++j) {
                t += A.value_data()[j]*x[A.index2_data()[j]];
            }

            b[i]=t;
        }
        return b;
    }
}

#endif
