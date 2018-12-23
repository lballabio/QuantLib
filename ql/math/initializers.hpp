/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2018 Peter Caspers

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

/*! \file initializers.hpp
    \brief array and matrix initializers
*/

#ifndef quantlib_initializers_hpp
#define quantlib_initializers_hpp

#include <ql/math/array.hpp>
#include <ql/math/matrix.hpp>

namespace QuantLib {

namespace initializers {

class ArrayProxy {
public:
    ArrayProxy& operator,(const Real x) {
        QL_REQUIRE(a_.size() > idx_,
                   "ArrayProxy: too many initializers, array has size "
                       << a_.size());
        a_[idx_++] = x;
        return *this;
    }

private:
    ArrayProxy(Array& a, const Real x) : a_(a) {
        QL_REQUIRE(a_.size() > 0, "ArrayProxy: array has size 0");
        a_[0] = x;
        idx_ = 1;
    }
    friend ArrayProxy operator<<(Array&, const Real);
    Size idx_;
    Array& a_;
};

class MatrixProxy {
public:
    MatrixProxy& operator,(const Real x) {
        QL_REQUIRE(m_.rows() * m_.columns() > idx_,
                   "MatrixProxy: too many initializers, matrix has size "
                       << m_.rows() << "x" << m_.columns());
        *(m_.begin() + idx_++) = x;
        return *this;
    }

private:
    MatrixProxy(Matrix& m, const Real x) : m_(m) {
        QL_REQUIRE(m_.rows() * m_.columns() > 0,
                   "MatrixProxy: matrix has size 0");
        *m_.begin() = x;
        idx_ = 1;
    }
    friend MatrixProxy operator<<(Matrix&, const Real);
    Size idx_;
    Matrix& m_;
};

inline ArrayProxy operator<<(Array& a, const Real x) {
    return ArrayProxy(a, x);
}

inline MatrixProxy operator<<(Matrix& m, const Real x) {
    return MatrixProxy(m, x);
}

} // namespace initializers

using initializers::operator<<;

} // namespace QuantLib

#endif
