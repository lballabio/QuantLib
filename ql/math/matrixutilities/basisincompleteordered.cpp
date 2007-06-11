/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Mark Joshi

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

#include <ql/math/matrixutilities/basisincompleteordered.hpp>
#include <algorithm>

namespace QuantLib {

    BasisIncompleteOrdered::BasisIncompleteOrdered(Size euclideanDimension)
    : euclideanDimension_(euclideanDimension) {}

    bool BasisIncompleteOrdered::addVector(const Array& newVector1) {

        QL_REQUIRE(newVector1.size() == euclideanDimension_,
                   "missized vector passed to "
                   "BasisIncompleteOrdered::addVector");

        newVector_ = newVector1;

        if (currentBasis_.size()==euclideanDimension_)
            return false;

        for (Size j=0; j<currentBasis_.size(); ++j) {
            Real innerProd = std::inner_product(newVector_.begin(),
                                                newVector_.end(),
                                                currentBasis_[j].begin(), 0.0);

            for (Size k=0; k<euclideanDimension_; ++k)
                newVector_[k] -=innerProd*currentBasis_[j][k];
        }

        Real norm = sqrt(std::inner_product(newVector_.begin(),
                                            newVector_.end(),
                                            newVector_.begin(), 0.0));

        if (norm==0.0) // maybe this should be a tolerance
            return false;

        for (Size l=0; l<euclideanDimension_; ++l)
            newVector_[l]/=norm;

        currentBasis_.push_back(newVector_);

        return true;
    }

    Size BasisIncompleteOrdered::basisSize() const {
        return currentBasis_.size();
    }

    Size BasisIncompleteOrdered::euclideanDimension() const {
        return euclideanDimension_;
    }


    Matrix BasisIncompleteOrdered::getBasisAsRowsInMatrix() const {
        Matrix basis(currentBasis_.size(), euclideanDimension_);
        for (Size i=0; i<basis.rows(); ++i)
            for (Size j=0; j<basis.columns(); ++j)
                basis[i][j] = currentBasis_[i][j];

        return basis;
    }

}
