/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Klaus Spanderen

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

/*! \file matrix.hpp
    \brief matrix used in linear algebra.
*/

#include <ql/math/matrix.hpp>
#if defined(QL_PATCH_MSVC)
#pragma warning(push)
#pragma warning(disable:4180)
#pragma warning(disable:4127)
#endif

#if BOOST_VERSION == 106400
#include <boost/serialization/array_wrapper.hpp>
#endif
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <boost/numeric/ublas/triangular.hpp>
#include <boost/numeric/ublas/lu.hpp>

#if defined(QL_PATCH_MSVC)
#pragma warning(pop)
#endif

namespace QuantLib {

    Matrix inverse(const Matrix& m) {
        QL_REQUIRE(m.rows() == m.columns(), "matrix is not square");

        boost::numeric::ublas::matrix<Real> a(m.rows(), m.columns());

        std::copy(m.begin(), m.end(), a.data().begin());

        boost::numeric::ublas::permutation_matrix<Size> pert(m.rows());

        // lu decomposition
        Size singular = 1;
        try {
            singular = lu_factorize(a, pert);
        } catch (const boost::numeric::ublas::internal_logic& e) {
            QL_FAIL("lu_factorize error: " << e.what());
        } catch (const boost::numeric::ublas::external_logic& e) {
            QL_FAIL("lu_factorize error: " << e.what());
        }
        QL_REQUIRE(singular == 0, "singular matrix given");

        boost::numeric::ublas::matrix<Real>
            inverse = boost::numeric::ublas::identity_matrix<Real>(m.rows());

        // backsubstitution
        try {
            boost::numeric::ublas::lu_substitute(a, pert, inverse);
        } catch (const boost::numeric::ublas::internal_logic& e) {
            QL_FAIL("lu_substitute error: " << e.what());
        }

        Matrix retVal(m.rows(), m.columns());
        std::copy(inverse.data().begin(), inverse.data().end(),
                  retVal.begin());

        return retVal;
    }

    Real determinant(const Matrix& m) {
        QL_REQUIRE(m.rows() == m.columns(), "matrix is not square");

        boost::numeric::ublas::matrix<Real> a(m.rows(), m.columns());
        std::copy(m.begin(), m.end(), a.data().begin());


        // lu decomposition
        boost::numeric::ublas::permutation_matrix<Size> pert(m.rows());
        /* const Size singular = */ lu_factorize(a, pert);

        Real retVal = 1.0;

        for (Size i=0; i < m.rows(); ++i) {
            if (pert[i] != i)
                retVal *= -a(i,i);
            else
                retVal *=  a(i,i);
        }
        return retVal;
    }

}
