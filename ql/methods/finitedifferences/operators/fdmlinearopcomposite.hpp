/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2012 Klaus Spanderen

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

/*! \file fdmlinearopcomposite.hpp
    \brief composite pattern for linear operators
*/

#ifndef quantlib_fdm_affine_map_composite_hpp
#define quantlib_fdm_affine_map_composite_hpp

#include <ql/math/matrixutilities/sparsematrix.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearop.hpp>
#include <numeric>

namespace QuantLib {

    class FdmLinearOpComposite : public FdmLinearOp {
      public:
        virtual Size size() const = 0;

        //! Time \f$t1 <= t2\f$ is required
        virtual void setTime(Time t1, Time t2) = 0;

        virtual Disposable<Array> apply_mixed(const Array& r) const = 0;
        
        virtual Disposable<Array> 
            apply_direction(Size direction, const Array& r) const = 0;
        virtual Disposable<Array> 
            solve_splitting(Size direction, const Array& r, Real s) const = 0;
        virtual Disposable<Array> 
            preconditioner(const Array& r, Real s) const = 0;

        virtual Disposable<std::vector<SparseMatrix> > toMatrixDecomp() const {
            QL_FAIL(" ublas representation is not implemented");
        }

        Disposable<SparseMatrix> toMatrix() const override {
            const std::vector<SparseMatrix> dcmp = toMatrixDecomp();
            SparseMatrix retVal = std::accumulate(dcmp.begin()+1, dcmp.end(),
                                                  SparseMatrix(dcmp.front()));
            return retVal;
        }

    };
}

#endif


#ifndef id_5ec3734876d970e563b3d0c8a5d93330
#define id_5ec3734876d970e563b3d0c8a5d93330
inline bool test_5ec3734876d970e563b3d0c8a5d93330(int* i) { return i != 0; }
#endif
