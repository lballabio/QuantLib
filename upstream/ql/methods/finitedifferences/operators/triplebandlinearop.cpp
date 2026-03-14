/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen
 Copyright (C) 2014 Johannes Göttker-Schnetmann

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/tridiagonaloperator.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>

namespace QuantLib {

    TripleBandLinearOp::TripleBandLinearOp(
        Size direction,
        const ext::shared_ptr<FdmMesher>& mesher)
    : direction_(direction),
      i0_       (new Size[mesher->layout()->size()]),
      i2_       (new Size[mesher->layout()->size()]),
      reverseIndex_ (new Size[mesher->layout()->size()]),
      lower_    (new Real[mesher->layout()->size()]),
      diag_     (new Real[mesher->layout()->size()]),
      upper_    (new Real[mesher->layout()->size()]),
      mesher_(mesher) {

        std::vector<Size> newDim(mesher->layout()->dim());
        std::iter_swap(newDim.begin(), newDim.begin()+direction_);
        std::vector<Size> newSpacing = FdmLinearOpLayout(newDim).spacing();
        std::iter_swap(newSpacing.begin(), newSpacing.begin()+direction_);

        for (const auto& iter : *mesher->layout()) {
            const Size i = iter.index();

            i0_[i] = mesher->layout()->neighbourhood(iter, direction, -1);
            i2_[i] = mesher->layout()->neighbourhood(iter, direction,  1);

            const std::vector<Size>& coordinates = iter.coordinates();
            const Size newIndex =
                  std::inner_product(coordinates.begin(), coordinates.end(),
                                     newSpacing.begin(), Size(0));
            reverseIndex_[newIndex] = i;
        }
    }

    TripleBandLinearOp::TripleBandLinearOp(const TripleBandLinearOp& m)
    : direction_(m.direction_),
      i0_   (new Size[m.mesher_->layout()->size()]),
      i2_   (new Size[m.mesher_->layout()->size()]),
      reverseIndex_(new Size[m.mesher_->layout()->size()]),
      lower_(new Real[m.mesher_->layout()->size()]),
      diag_ (new Real[m.mesher_->layout()->size()]),
      upper_(new Real[m.mesher_->layout()->size()]),
      mesher_(m.mesher_) {
        const Size len = m.mesher_->layout()->size();
        std::copy(m.i0_.get(), m.i0_.get() + len, i0_.get());
        std::copy(m.i2_.get(), m.i2_.get() + len, i2_.get());
        std::copy(m.reverseIndex_.get(), m.reverseIndex_.get()+len,
                  reverseIndex_.get());
        std::copy(m.lower_.get(), m.lower_.get() + len, lower_.get());
        std::copy(m.diag_.get(),  m.diag_.get() + len,  diag_.get());
        std::copy(m.upper_.get(), m.upper_.get() + len, upper_.get());
    }

    void TripleBandLinearOp::swap(TripleBandLinearOp& m) noexcept {
        mesher_.swap(m.mesher_);
        std::swap(direction_, m.direction_);

        i0_.swap(m.i0_); i2_.swap(m.i2_);
        reverseIndex_.swap(m.reverseIndex_);
        lower_.swap(m.lower_); diag_.swap(m.diag_); upper_.swap(m.upper_);
    }

    void TripleBandLinearOp::axpyb(const Array& a,
                                   const TripleBandLinearOp& x,
                                   const TripleBandLinearOp& y,
                                   const Array& b) {
        const Size size = mesher_->layout()->size();

        Real *diag(diag_.get());
        Real *lower(lower_.get());
        Real *upper(upper_.get());

        const Real *y_diag (y.diag_.get());
        const Real *y_lower(y.lower_.get());
        const Real *y_upper(y.upper_.get());

        if (a.empty()) {
            if (b.empty()) {
                //#pragma omp parallel for
                for (Size i=0; i < size; ++i) {
                    diag[i]  = y_diag[i];
                    lower[i] = y_lower[i];
                    upper[i] = y_upper[i];
                }
            }
            else {
                Array::const_iterator bptr(b.begin());
                const Size binc = (b.size() > 1) ? 1 : 0;
                //#pragma omp parallel for
                for (Size i=0; i < size; ++i) {
                    diag[i]  = y_diag[i] + bptr[i*binc];
                    lower[i] = y_lower[i];
                    upper[i] = y_upper[i];
                }
            }
        }
        else if (b.empty()) {
            Array::const_iterator aptr(a.begin());
            const Size ainc = (a.size() > 1) ? 1 : 0;

            const Real *x_diag (x.diag_.get());
            const Real *x_lower(x.lower_.get());
            const Real *x_upper(x.upper_.get());

            //#pragma omp parallel for
            for (Size i=0; i < size; ++i) {
                const Real s = aptr[i*ainc];
                diag[i]  = y_diag[i]  + s*x_diag[i];
                lower[i] = y_lower[i] + s*x_lower[i];
                upper[i] = y_upper[i] + s*x_upper[i];
            }
        }
        else {
            Array::const_iterator bptr(b.begin());
            const Size binc = (b.size() > 1) ? 1 : 0;

            Array::const_iterator aptr(a.begin());
            const Size ainc = (a.size() > 1) ? 1 : 0;

            const Real *x_diag (x.diag_.get());
            const Real *x_lower(x.lower_.get());
            const Real *x_upper(x.upper_.get());

            //#pragma omp parallel for
            for (Size i=0; i < size; ++i) {
                const Real s = aptr[i*ainc];
                diag[i]  = y_diag[i]  + s*x_diag[i] + bptr[i*binc];
                lower[i] = y_lower[i] + s*x_lower[i];
                upper[i] = y_upper[i] + s*x_upper[i];
            }
        }
    }

    TripleBandLinearOp TripleBandLinearOp::add(const TripleBandLinearOp& m) const {

        TripleBandLinearOp retVal(direction_, mesher_);
        const Size size = mesher_->layout()->size();
        //#pragma omp parallel for
        for (Size i=0; i < size; ++i) {
            retVal.lower_[i]= lower_[i] + m.lower_[i];
            retVal.diag_[i] = diag_[i]  + m.diag_[i];
            retVal.upper_[i]= upper_[i] + m.upper_[i];
        }

        return retVal;
    }


    TripleBandLinearOp TripleBandLinearOp::mult(const Array& u) const {

        TripleBandLinearOp retVal(direction_, mesher_);

        const Size size = mesher_->layout()->size();
        //#pragma omp parallel for
        for (Size i=0; i < size; ++i) {
            const Real s = u[i];
            retVal.lower_[i]= lower_[i]*s;
            retVal.diag_[i] = diag_[i]*s;
            retVal.upper_[i]= upper_[i]*s;
        }

        return retVal;
    }

    TripleBandLinearOp TripleBandLinearOp::multR(const Array& u) const {
        const Size size = mesher_->layout()->size();
        QL_REQUIRE(u.size() == size, "inconsistent size of rhs");
        TripleBandLinearOp retVal(direction_, mesher_);

        #pragma omp parallel for
        for (long i=0; i < (long)size; ++i) {
            const Real sm1 = i > 0? u[i-1] : 1.0;
            const Real s0 = u[i];
            const Real sp1 = i < (long)size-1? u[i+1] : 1.0;
            retVal.lower_[i]= lower_[i]*sm1;
            retVal.diag_[i] = diag_[i]*s0;
            retVal.upper_[i]= upper_[i]*sp1;
        }

        return retVal;
    }

    TripleBandLinearOp TripleBandLinearOp::add(const Array& u) const {

        TripleBandLinearOp retVal(direction_, mesher_);

        const Size size = mesher_->layout()->size();
        //#pragma omp parallel for
        for (Size i=0; i < size; ++i) {
            retVal.lower_[i]= lower_[i];
            retVal.upper_[i]= upper_[i];
            retVal.diag_[i] = diag_[i]+u[i];
        }

        return retVal;
    }

    Array TripleBandLinearOp::apply(const Array& r) const {
        QL_REQUIRE(r.size() == mesher_->layout()->size(), "inconsistent length of r");

        const Real* lptr = lower_.get();
        const Real* dptr = diag_.get();
        const Real* uptr = upper_.get();
        const Size* i0ptr = i0_.get();
        const Size* i2ptr = i2_.get();

        array_type retVal(r.size());
        //#pragma omp parallel for
        for (Size i=0; i < mesher_->layout()->size(); ++i) {
            retVal[i] = r[i0ptr[i]]*lptr[i]+r[i]*dptr[i]+r[i2ptr[i]]*uptr[i];
        }

        return retVal;
    }

    SparseMatrix TripleBandLinearOp::toMatrix() const {
        const Size n = mesher_->layout()->size();

        SparseMatrix retVal(n, n, 3*n);
        for (Size i=0; i < n; ++i) {
            retVal(i, i0_[i]) += lower_[i];
            retVal(i, i     ) += diag_[i];
            retVal(i, i2_[i]) += upper_[i];
        }

        return retVal;
    }


    Array TripleBandLinearOp::solve_splitting(const Array& r, Real a, Real b) const {
        QL_REQUIRE(r.size() == mesher_->layout()->size(), "inconsistent size of rhs");

#ifdef QL_EXTRA_SAFETY_CHECKS
        for (const auto& iter : *mesher_->layout()) {
            const std::vector<Size>& coordinates = iter.coordinates();
            QL_REQUIRE(   coordinates[direction_] != 0
                       || lower_[iter.index()] == 0,"removing non zero entry!");
            QL_REQUIRE(   coordinates[direction_] != mesher_->layout()->dim()[direction_]-1
                       || upper_[iter.index()] == 0,"removing non zero entry!");
        }
#endif

        Array retVal(r.size()), tmp(r.size());

        const Real* lptr = lower_.get();
        const Real* dptr = diag_.get();
        const Real* uptr = upper_.get();

        // Thomson algorithm to solve a tridiagonal system.
        // Example code taken from Tridiagonalopertor and
        // changed to fit for the triple band operator.
        Size rim1 = reverseIndex_[0];
        Real bet=1.0/(a*dptr[rim1]+b);
        QL_REQUIRE(bet != 0.0, "division by zero");
        retVal[reverseIndex_[0]] = r[rim1]*bet;

        for (Size j=1; j<=mesher_->layout()->size()-1; j++){
            const Size ri = reverseIndex_[j];
            tmp[j] = a*uptr[rim1]*bet;

            bet=b+a*(dptr[ri]-tmp[j]*lptr[ri]);
            QL_ENSURE(bet != 0.0, "division by zero");
            bet=1.0/bet;

            retVal[ri] = (r[ri]-a*lptr[ri]*retVal[rim1])*bet;
            rim1 = ri;
        }
        // cannot be j>=0 with Size j
        for (Size j=mesher_->layout()->size()-2; j>0; --j)
            retVal[reverseIndex_[j]] -= tmp[j+1]*retVal[reverseIndex_[j+1]];
        retVal[reverseIndex_[0]] -= tmp[1]*retVal[reverseIndex_[1]];

        return retVal;
    }
}
