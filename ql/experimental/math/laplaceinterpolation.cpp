/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015, 2024 Peter Caspers

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

/*! \file laplaceinterpolation.hpp
    \brief Laplace interpolation of missing values
*/

#include <ql/experimental/math/laplaceinterpolation.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/matrixutilities/sparsematrix.hpp>
#include <ql/methods/finitedifferences/meshers/fdm1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearopcomposite.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>
#include <ql/methods/finitedifferences/operators/triplebandlinearop.hpp>

namespace QuantLib {

    LaplaceInterpolation::LaplaceInterpolation(std::function<Real(const std::vector<Size>&)> y,
                                               std::vector<std::vector<Real>> x,
                                               const Real relTol)
    : y_(std::move(y)), x_(std::move(x)), relTol_(relTol) {

        // set up the mesher

        std::vector<Size> dim;
        coordinateIncluded_.resize(x_.size());
        for (Size i = 0; i < x_.size(); ++i) {
            coordinateIncluded_[i] = x_[i].size() > 1;
            if (coordinateIncluded_[i])
                dim.push_back(x_[i].size());
        }

        numberOfCoordinatesIncluded_ = dim.size();

        if (numberOfCoordinatesIncluded_ == 0) {
            return;
        }

        QL_REQUIRE(!dim.empty(), "LaplaceInterpolation: singular point or no points given");

        layout_ = ext::make_shared<FdmLinearOpLayout>(dim);

        std::vector<ext::shared_ptr<Fdm1dMesher>> meshers;
        for (Size i = 0; i < x_.size(); ++i) {
            if (x_[i].size() > 1)
                meshers.push_back(ext::make_shared<Predefined1dMesher>(x_[i]));
        }

        auto mesher = ext::make_shared<FdmMesherComposite>(layout_, meshers);

        // set up the Laplace operator and convert it to matrix

        struct LaplaceOp : public FdmLinearOpComposite {
            explicit LaplaceOp(const ext::shared_ptr<FdmMesher>& mesher) {
                for (Size direction = 0; direction < mesher->layout()->dim().size(); ++direction) {
                    if (mesher->layout()->dim()[direction] > 1)
                        map_.push_back(SecondDerivativeOp(direction, mesher));
                }
            }
            std::vector<TripleBandLinearOp> map_;

            Size size() const override { QL_FAIL("no impl"); }
            void setTime(Time t1, Time t2) override { QL_FAIL("no impl"); }
            Array apply(const array_type& r) const override { QL_FAIL("no impl"); }
            Array apply_mixed(const Array& r) const override { QL_FAIL("no impl"); }
            Array apply_direction(Size direction, const Array& r) const override {
                QL_FAIL("no impl");
            }
            Array solve_splitting(Size direction, const Array& r, Real s) const override {
                QL_FAIL("no impl");
            }
            Array preconditioner(const Array& r, Real s) const override { QL_FAIL("no impl"); }
            std::vector<SparseMatrix> toMatrixDecomp() const override {
                std::vector<SparseMatrix> decomp;
                for (auto const& m : map_)
                    decomp.push_back(m.toMatrix());
                return decomp;
            }
        };

        SparseMatrix op = LaplaceOp(mesher).toMatrix();

        // set up the linear system to solve

        Size N = layout_->size();

        SparseMatrix g(N, N, 5 * N);
        Array rhs(N, 0.0), guess(N, 0.0);
        Real guessTmp = 0.0;

        struct f_A {
            const SparseMatrix& g;
            explicit f_A(const SparseMatrix& g) : g(g) {}
            Array operator()(const Array& x) const { return prod(g, x); }
        };

        auto rowit = op.begin1();
        Size count = 0;
        std::vector<Real> corner_h(dim.size());
        std::vector<Size> corner_neighbour_index(dim.size());
        for (auto const& pos : *layout_) {
            auto coord = pos.coordinates();
            Real val =
                y_(numberOfCoordinatesIncluded_ == x_.size() ? coord : fullCoordinates(coord));
            QL_REQUIRE(rowit != op.end1() && rowit.index1() == count,
                       "LaplaceInterpolation: op matrix row iterator ("
                           << (rowit != op.end1() ? std::to_string(rowit.index1()) : "na")
                           << ") does not match expected row count (" << count << ")");
            if (val == Null<Real>()) {
                bool isCorner = true;
                for (Size d = 0; d < dim.size() && isCorner; ++d) {
                    if (coord[d] == 0) {
                        corner_h[d] = meshers[d]->dplus(0);
                        corner_neighbour_index[d] = 1;
                    } else if (coord[d] == layout_->dim()[d] - 1) {
                        corner_h[d] = meshers[d]->dminus(dim[d] - 1);
                        corner_neighbour_index[d] = dim[d] - 2;
                    } else {
                        isCorner = false;
                    }
                }
                if (isCorner) {
                    // handling of the "corners", all second derivs are zero in the op
                    // this directly generalizes Numerical Recipes, 3rd ed, eq 3.8.6
                    Real sum_corner_h =
                        std::accumulate(corner_h.begin(), corner_h.end(), 0.0, std::plus<Real>());
                    for (Size j = 0; j < dim.size(); ++j) {
                        std::vector<Size> coord_j(coord);
                        coord_j[j] = corner_neighbour_index[j];
                        Real weight = 0.0;
                        for (Size i = 0; i < dim.size(); ++i) {
                            if (i != j)
                                weight += corner_h[i];
                        }
                        weight = dim.size() == 1 ? Real(1.0) : Real(weight / sum_corner_h);
                        g(count, layout_->index(coord_j)) = -weight;
                    }
                    g(count, count) = 1.0;
                } else {
                    // point with at least one dimension with non-trivial second derivative
                    for (auto colit = rowit.begin(); colit != rowit.end(); ++colit)
                        g(count, colit.index2()) = *colit;
                }
                rhs[count] = 0.0;
                guess[count] = guessTmp;
            } else {
                g(count, count) = 1;
                rhs[count] = val;
                guess[count] = guessTmp = val;
            }
            ++count;
            ++rowit;
        }

        interpolatedValues_ = BiCGstab(f_A(g), 10 * N, relTol_).solve(rhs, guess).x;
    }

    std::vector<Size>
    LaplaceInterpolation::projectedCoordinates(const std::vector<Size>& coordinates) const {
        std::vector<Size> tmp;
        for (Size i = 0; i < coordinates.size(); ++i) {
            if (coordinateIncluded_[i])
                tmp.push_back(coordinates[i]);
        }
        return tmp;
    }

    std::vector<Size>
    LaplaceInterpolation::fullCoordinates(const std::vector<Size>& projectedCoordinates) const {
        std::vector<Size> tmp(coordinateIncluded_.size(), 0);
        for (Size i = 0, count = 0; i < coordinateIncluded_.size(); ++i) {
            if (coordinateIncluded_[i])
                tmp[i] = projectedCoordinates[count++];
        }
        return tmp;
    }

    Real LaplaceInterpolation::operator()(const std::vector<Size>& coordinates) const {
        QL_REQUIRE(coordinates.size() == x_.size(), "LaplaceInterpolation::operator(): expected "
                                                        << x_.size() << " coordinates, got "
                                                        << coordinates.size());
        if (numberOfCoordinatesIncluded_ == 0) {
            Real val = y_(coordinates);
            return val == Null<Real>() ? 0.0 : val;
        } else {
            return interpolatedValues_[layout_->index(numberOfCoordinatesIncluded_ == x_.size() ?
                                                          coordinates :
                                                          projectedCoordinates(coordinates))];
        }
    }

    void laplaceInterpolation(Matrix& A,
                              const std::vector<Real>& x,
                              const std::vector<Real>& y,
                              Real relTol) {

        std::vector<std::vector<Real>> tmp;
        tmp.push_back(y);
        tmp.push_back(x);

        if (y.empty()) {
            tmp[0].resize(A.rows());
            std::iota(tmp[0].begin(), tmp[0].end(), 0.0);
        }

        if (x.empty()) {
            tmp[1].resize(A.columns());
            std::iota(tmp[1].begin(), tmp[1].end(), 0.0);
        }

        LaplaceInterpolation interpolation(
            [&A](const std::vector<Size>& coordinates) {
                return A(coordinates[0], coordinates[1]);
            },
            tmp, relTol);

        for (Size i = 0; i < A.rows(); ++i) {
            for (Size j = 0; j < A.columns(); ++j) {
                if (A(i, j) == Null<Real>())
                    A(i, j) = interpolation({i, j});
            }
        }
    }

} // namespace QuantLib
