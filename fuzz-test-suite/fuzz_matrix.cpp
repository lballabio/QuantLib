/*
 Copyright (C) 2026 OSS-Fuzz coverage pipeline

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the license
 for more details.
*/

//! Fuzz target: QuantLib matrix decompositions and iterative solvers
//! (ql/math/matrixutilities/*).
//!
//! Threat model / rationale: these kernels consume numeric arrays that
//! arrive from data feeds and calibration results (correlation matrices,
//! covariance inputs, calibration residuals). In the 20261021 coverage
//! report ~1,627 lines were dark because no harness reaches them
//! (svd.cpp 372, pseudosqrt.cpp 353, tqreigendecomposition.cpp 89,
//! gmres.cpp 76, bicgstab.cpp 48, householder.cpp 42, cholesky 41,
//! factorreduction.cpp 40, expm.cpp 23). Degenerate input matrices
//! (near-singular, indefinite, huge dynamic range) are exactly the
//! attacker-influenced case: divisions by tiny pivots, iterative
//! refinement loops and work-array indexing are the expected bug
//! classes. Sizes are bounded (n <= 8) to keep memory and runtime in
//! check; entry magnitude is bounded to [-1000, 1000].

#include <fuzzer/FuzzedDataProvider.h>

#include <ql/math/array.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/matrixutilities/bicgstab.hpp>
#include <ql/math/matrixutilities/choleskydecomposition.hpp>
#include <ql/math/matrixutilities/expm.hpp>
#include <ql/math/matrixutilities/factorreduction.hpp>
#include <ql/math/matrixutilities/getcovariance.hpp>
#include <ql/math/matrixutilities/gmres.hpp>
#include <ql/math/matrixutilities/householder.hpp>
#include <ql/math/matrixutilities/pseudosqrt.hpp>
#include <ql/math/matrixutilities/svd.hpp>
#include <ql/math/matrixutilities/tqreigendecomposition.hpp>
#include <vector>

using namespace QuantLib;

namespace {

constexpr int kNumOps = 11;

Matrix makeMatrix(FuzzedDataProvider& fdp, Size n) {
    Matrix m(n, n);
    for (Size i = 0; i < n; ++i)
        for (Size j = 0; j < n; ++j)
            m[i][j] = fdp.ConsumeFloatingPointInRange<Real>(-1000.0, 1000.0);
    return m;
}

Array makeVector(FuzzedDataProvider& fdp, Size n) {
    Array b(n);
    for (Size i = 0; i < n; ++i)
        b[i] = fdp.ConsumeFloatingPointInRange<Real>(-1000.0, 1000.0);
    return b;
}

} // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);

    try {
        const Size n = fdp.ConsumeIntegralInRange<Size>(1, 8);
        const int op = fdp.ConsumeIntegralInRange<int>(0, kNumOps - 1);
        const Matrix m = makeMatrix(fdp, n);

        switch (op) {
        case 0: {
            // Singular value decomposition and its derived quantities.
            SVD svd(m);
            (void)svd.U();
            (void)svd.V();
            (void)svd.S();
            (void)svd.singularValues();
            (void)svd.norm2();
            (void)svd.cond();
            (void)svd.rank();
            (void)svd.solveFor(makeVector(fdp, n));
            break;
        }
        case 1: {
            // Cholesky decomposition, strict and flexible variants.
            const bool flexible = fdp.ConsumeBool();
            (void)CholeskyDecomposition(m, flexible);
            break;
        }
        case 2: {
            // Pseudo square root with every salvaging algorithm.
            const auto salvage = static_cast<SalvagingAlgorithm::Type>(
                fdp.ConsumeIntegralInRange<int>(0, 5));
            (void)pseudoSqrt(m, salvage);
            break;
        }
        case 3: {
            // Rank-reduced pseudo square root.
            const auto salvage = static_cast<SalvagingAlgorithm::Type>(
                fdp.ConsumeIntegralInRange<int>(0, 5));
            const Size maxRank = fdp.ConsumeIntegralInRange<Size>(1, n);
            const Real pct =
                fdp.ConsumeFloatingPointInRange<Real>(0.0, 1.0);
            (void)rankReducedSqrt(m, maxRank, pct, salvage);
            break;
        }
        case 4: {
            // Tridiagonal QR eigen decomposition over a fuzzed pencil.
            Array diag(n), sub(n > 0 ? n - 1 : 0);
            for (Size i = 0; i < n; ++i)
                diag[i] = fdp.ConsumeFloatingPointInRange<Real>(-100.0, 100.0);
            for (Size i = 0; i < sub.size(); ++i)
                sub[i] = fdp.ConsumeFloatingPointInRange<Real>(-100.0, 100.0);
            const auto calc = static_cast<
                TqrEigenDecomposition::EigenVectorCalculation>(
                fdp.ConsumeIntegralInRange<int>(0, 2));
            const auto strategy = static_cast<
                TqrEigenDecomposition::ShiftStrategy>(
                fdp.ConsumeIntegralInRange<int>(0, 2));
            TqrEigenDecomposition tqr(diag, sub, calc, strategy);
            (void)tqr.eigenvalues();
            (void)tqr.eigenvectors();
            (void)tqr.iterations();
            break;
        }
        case 5: {
            // GMRES on a fuzzed linear system.
            const Size maxIter = fdp.ConsumeIntegralInRange<Size>(1, 50);
            const Real relTol = fdp.ConsumeFloatingPointInRange<Real>(
                1.0e-9, 1.0e-2);
            GMRES gmres([m](const Array& x) { return m * x; },
                        maxIter, relTol);
            const auto res = gmres.solve(makeVector(fdp, n));
            (void)res.errors;
            (void)res.x;
            break;
        }
        case 6: {
            // GMRES with restart.
            const Size maxIter = fdp.ConsumeIntegralInRange<Size>(1, 50);
            const Size restart = fdp.ConsumeIntegralInRange<Size>(1, 20);
            const Real relTol = fdp.ConsumeFloatingPointInRange<Real>(
                1.0e-9, 1.0e-2);
            GMRES gmres([m](const Array& x) { return m * x; },
                        maxIter, relTol);
            const auto res = gmres.solveWithRestart(
                restart, makeVector(fdp, n));
            (void)res.errors;
            (void)res.x;
            break;
        }
        case 7: {
            // BiCGstab on a fuzzed linear system.
            const Size maxIter = fdp.ConsumeIntegralInRange<Size>(1, 50);
            const Real relTol = fdp.ConsumeFloatingPointInRange<Real>(
                1.0e-9, 1.0e-2);
            BiCGstab bicg([m](const Array& x) { return m * x; },
                          maxIter, relTol);
            const auto res = bicg.solve(makeVector(fdp, n));
            (void)res.error;
            (void)res.iterations;
            (void)res.x;
            break;
        }
        case 8: {
            // Matrix exponential.
            const Real t = fdp.ConsumeFloatingPointInRange<Real>(-2.0, 2.0);
            (void)Expm(m, t);
            break;
        }
        case 9: {
            // Factor reduction (PCA-style eigenvalue aggregation).
            (void)factorReduction(m,
                fdp.ConsumeIntegralInRange<Size>(1, 25));
            break;
        }
        default: {
            // Covariance from standard deviations + correlation matrix.
            const Array stdDev = makeVector(fdp, n);
            Matrix corr(n, n);
            for (Size i = 0; i < n; ++i)
                for (Size j = 0; j < n; ++j)
                    corr[i][j] = (i == j) ? 1.0
                        : fdp.ConsumeFloatingPointInRange<Real>(-1.0, 1.0);
            (void)getCovariance(stdDev.begin(), stdDev.end(), corr);
            break;
        }
        }
    } catch (const std::exception&) {
        // QuantLib reports invalid input via exceptions; not a finding.
    }
    return 0;
}

