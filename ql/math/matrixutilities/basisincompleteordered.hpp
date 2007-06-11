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


#ifndef quantlib_basis_incomplete_ordered_hpp
#define quantlib_basis_incomplete_ordered_hpp

#include <ql/math/matrix.hpp>

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

}

#endif
