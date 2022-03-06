/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007, 2008 Mark Joshi

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


#ifndef quantlib_basis_incomplete_ordered_hpp
#define quantlib_basis_incomplete_ordered_hpp

#include <ql/math/matrix.hpp>
#include <valarray>

namespace QuantLib {

    class BasisIncompleteOrdered {
      public:
        BasisIncompleteOrdered(Size euclideanDimension);
        //! return value indicates if the vector was linearly independent
        bool addVector(const Array& newVector);
        Size basisSize() const;
        Size euclideanDimension() const;
        Matrix getBasisAsRowsInMatrix() const;
      private:
        std::vector<Array> currentBasis_;
        Size euclideanDimension_;
        Array newVector_;
    };

/*! Given a collection of vectors, w_i, find a collection of vectors x_i such that
x_i is orthogonal to w_j for i != j, and <x_i, w_i> = <w_i, w_i>

This is done by performing GramSchmidt on the other vectors and then projecting onto
the orthogonal space.

This class is tested in

    MatricesTest::testOrthogonalProjection();
*/

    class OrthogonalProjections
    {
    public:
        OrthogonalProjections(const Matrix& originalVectors,
                              Real multiplierCutOff,
                               Real tolerance  );

        const std::valarray<bool>& validVectors() const;
        const std::vector<Real>& GetVector(Size index) const;

        Size numberValidVectors() const;


    private:

        //! inputs
        Matrix originalVectors_;
        Real multiplierCutoff_;
        Size numberVectors_;
        Size numberValidVectors_;
        Size dimension_;

        //!outputs
        std::valarray<bool> validVectors_;
        std::vector<std::vector<Real> > projectedVectors_;

        //!workspace
        Matrix orthoNormalizedVectors_;


    };

}

#endif


#ifndef id_32bbe94c33374934fe1af1fc0ed489f4
#define id_32bbe94c33374934fe1af1fc0ed489f4
inline bool test_32bbe94c33374934fe1af1fc0ed489f4(const int* i) {
    return i != nullptr;
}
#endif
