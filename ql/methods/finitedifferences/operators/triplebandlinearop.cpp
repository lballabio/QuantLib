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
      temp_     (mesher->layout()->size()),
      mesher_(mesher) {

        std::vector<Size> newDim(mesher->layout()->dim());
        std::iter_swap(newDim.begin(), newDim.begin()+direction_);
        std::vector<Size> newSpacing = FdmLinearOpLayout(newDim).spacing();
        std::iter_swap(newSpacing.begin(), newSpacing.begin()+direction_);

        for (const auto& iter : *mesher->layout()) {
            const auto i = iter.index();

            i0_[i] = mesher->layout()->neighbourhood(iter, direction, -1);
            i2_[i] = mesher->layout()->neighbourhood(iter, direction,  1);

            const auto& coordinates = iter.coordinates();
            const auto newIndex =
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
      temp_ (m.mesher_->layout()->size()),
      mesher_(m.mesher_) {
        const auto len = m.mesher_->layout()->size();
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
        temp_.swap(m.temp_);
    }

    void TripleBandLinearOp::axpyb(const Array& a,
                                   const TripleBandLinearOp& x,
                                   const TripleBandLinearOp& y,
                                   const Array& b) {
        const auto size = mesher_->layout()->size();

        Real *diag(diag_.get());
        Real *lower(lower_.get());
        Real *upper(upper_.get());

        const auto *y_diag (y.diag_.get());
        const auto *y_lower(y.lower_.get());
        const auto *y_upper(y.upper_.get());

        if (a.empty()) {
            if (b.empty()) {
                //#pragma omp parallel for
                for (auto i=0u; i < size; ++i) {
                    diag[i]  = y_diag[i];
                    lower[i] = y_lower[i];
                    upper[i] = y_upper[i];
                }
            }
            else {
                Array::const_iterator bptr(b.begin());
                const auto binc = (b.size() > 1) ? 1 : 0;
                //#pragma omp parallel for
                for (auto i=0u; i < size; ++i) {
                    diag[i]  = y_diag[i] + bptr[i*binc];
                    lower[i] = y_lower[i];
                    upper[i] = y_upper[i];
                }
            }
        }
        else if (b.empty()) {
            Array::const_iterator aptr(a.begin());
            const auto ainc = (a.size() > 1) ? 1 : 0;

            const auto *x_diag (x.diag_.get());
            const auto *x_lower(x.lower_.get());
            const auto *x_upper(x.upper_.get());

            //#pragma omp parallel for
            for (auto i=0u; i < size; ++i) {
                const auto s = aptr[i*ainc];
                diag[i]  = y_diag[i]  + s*x_diag[i];
                lower[i] = y_lower[i] + s*x_lower[i];
                upper[i] = y_upper[i] + s*x_upper[i];
            }
        }
        else {
            Array::const_iterator bptr(b.begin());
            const auto binc = (b.size() > 1) ? 1 : 0;

            Array::const_iterator aptr(a.begin());
            const auto ainc = (a.size() > 1) ? 1 : 0;

            const auto *x_diag (x.diag_.get());
            const auto *x_lower(x.lower_.get());
            const auto *x_upper(x.upper_.get());

            //#pragma omp parallel for
            for (auto i=0u; i < size; ++i) {
                const auto s = aptr[i*ainc];
                diag[i]  = y_diag[i]  + s*x_diag[i] + bptr[i*binc];
                lower[i] = y_lower[i] + s*x_lower[i];
                upper[i] = y_upper[i] + s*x_upper[i];
            }
        }
    }

    TripleBandLinearOp TripleBandLinearOp::add(const TripleBandLinearOp& m) const {

        TripleBandLinearOp retVal(direction_, mesher_);
        const auto size = mesher_->layout()->size();
        //#pragma omp parallel for
        for (auto i=0u; i < size; ++i) {
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
        for (auto i=0u; i < size; ++i) {
            const auto s = u[i];
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
        for (auto i=0u; i < size; ++i) {
            const auto sm1 = i > 0? u[i-1] : 1.0;
            const auto s0 = u[i];
            const auto sp1 = i < size-1? u[i+1] : 1.0;
            retVal.lower_[i]= lower_[i]*sm1;
            retVal.diag_[i] = diag_[i]*s0;
            retVal.upper_[i]= upper_[i]*sp1;
        }

        return retVal;
    }

    TripleBandLinearOp TripleBandLinearOp::add(const Array& u) const {

        TripleBandLinearOp retVal(direction_, mesher_);

        const auto size = mesher_->layout()->size();
        //#pragma omp parallel for
        for (auto i=0u; i < size; ++i) {
            retVal.lower_[i]= lower_[i];
            retVal.upper_[i]= upper_[i];
            retVal.diag_[i] = diag_[i]+u[i];
        }

        return retVal;
    }

    Array TripleBandLinearOp::apply(const Array& r) const {
        QL_REQUIRE(r.size() == mesher_->layout()->size(), "inconsistent length of r");

        const auto* lptr = lower_.get();
        const auto* dptr = diag_.get();
        const auto* uptr = upper_.get();
        const auto* i0ptr = i0_.get();
        const auto* i2ptr = i2_.get();

        array_type retVal(r.size());
        //#pragma omp parallel for
        for (auto i=0u; i < mesher_->layout()->size(); ++i) {
            retVal[i] = r[i0ptr[i]]*lptr[i]+r[i]*dptr[i]+r[i2ptr[i]]*uptr[i];
        }

        return retVal;
    }

    SparseMatrix TripleBandLinearOp::toMatrix() const {
        const auto n = mesher_->layout()->size();

        SparseMatrix retVal(n, n, 3*n);
        for (auto i=0u; i < n; ++i) {
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

        const auto* lptr = lower_.get();
        const auto* dptr = diag_.get();
        const auto* uptr = upper_.get();

        // Thomas algorithm to solve a tridiagonal system.
        const auto size = mesher_->layout()->size();
        const auto solve = [&](const auto& index) {
            Array result(size);

            auto previous = index(0);
            auto beta = a*dptr[previous] + b;
            QL_REQUIRE(beta != 0.0, "division by zero");
            beta = 1.0 / beta;
            result[previous] = r[previous] * beta;

            for (auto j=1u; j<size; ++j) {
                const auto current = index(j);
                temp_[j] = a * uptr[previous] * beta;

                beta = b + a * (dptr[current] - temp_[j] * lptr[current]);
                QL_ENSURE(beta != 0.0, "division by zero");
                beta = 1.0 / beta;

                result[current] =
                    (r[current] - a*lptr[current]*result[previous]) * beta;
                previous = current;
            }

            // j cannot be greater than or equal to zero when Size is unsigned.
            for (auto j=size-2; j>0; --j)
                result[index(j)] -= temp_[j+1] * result[index(j+1)];
            result[index(0)] -= temp_[1] * result[index(1)];

            return result;
        };

        // The first direction follows storage order and needs no index lookup.
        if (direction_ == 0)
            return solve([](Size i) { return i; });
        return solve([this](Size i) { return reverseIndex_[i]; });
    }
}
