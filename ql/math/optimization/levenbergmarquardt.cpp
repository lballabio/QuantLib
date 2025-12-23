/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen
 Copyright (C) 2015 Peter Caspers

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

#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/lmdif.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <functional>
#include <memory>

namespace QuantLib {

    LevenbergMarquardt::LevenbergMarquardt(Real epsfcn,
                                           Real xtol,
                                           Real gtol,
                                           bool useCostFunctionsJacobian)
    : epsfcn_(epsfcn), xtol_(xtol), gtol_(gtol),
      useCostFunctionsJacobian_(useCostFunctionsJacobian) {}

    EndCriteria::Type LevenbergMarquardt::minimize(Problem& P,
                                                   const EndCriteria& endCriteria) {
        P.reset();
        const Array& initX = P.currentValue();
        currentProblem_ = &P;
        initCostValues_ = P.costFunction().values(initX);
        int m = initCostValues_.size();
        int n = initX.size();
        if (useCostFunctionsJacobian_) {
            initJacobian_ = Matrix(m,n);
            P.costFunction().jacobian(initJacobian_, initX);
        }
        Array xx = initX;
        std::unique_ptr<Real[]> fvec(new Real[m]);
        std::unique_ptr<Real[]> diag(new Real[n]);
        int mode = 1;
        // magic number recommended by the documentation
        Real factor = 100;
        // lmdif() evaluates cost function n+1 times for each iteration (technically, 2n+1
        // times if useCostFunctionsJacobian is true, but lmdif() doesn't account for that)
        int maxfev = endCriteria.maxIterations() * (n + 1);
        int nprint = 0;
        int info = 0;
        int nfev = 0;
        std::unique_ptr<Real[]> fjac(new Real[m*n]);
        int ldfjac = m;
        std::unique_ptr<int[]> ipvt(new int[n]);
        std::unique_ptr<Real[]> qtf(new Real[n]);
        std::unique_ptr<Real[]> wa1(new Real[n]);
        std::unique_ptr<Real[]> wa2(new Real[n]);
        std::unique_ptr<Real[]> wa3(new Real[n]);
        std::unique_ptr<Real[]> wa4(new Real[m]);
        // requirements; check here to get more detailed error messages.
        QL_REQUIRE(n > 0, "no variables given");
        QL_REQUIRE(m >= n,
                   "less functions (" << m <<
                   ") than available variables (" << n << ")");
        QL_REQUIRE(endCriteria.functionEpsilon() >= 0.0,
                   "negative f tolerance");
        QL_REQUIRE(xtol_ >= 0.0, "negative x tolerance");
        QL_REQUIRE(gtol_ >= 0.0, "negative g tolerance");
        QL_REQUIRE(maxfev > 0, "null number of evaluations");

        // call lmdif to minimize the sum of the squares of m functions
        // in n variables by the Levenberg-Marquardt algorithm.
        MINPACK::LmdifCostFunction lmdifCostFunction =
            [this](const auto m, const auto n, const auto x, const auto fvec, const auto iflag) {
                this->fcn(m, n, x, fvec);
            };
        MINPACK::LmdifCostFunction lmdifJacFunction =
            useCostFunctionsJacobian_
                ? [this](const auto m, const auto n, const auto x, const auto fjac, const auto iflag) {
                    this->jacFcn(m, n, x, fjac);
                }
                : MINPACK::LmdifCostFunction();
        MINPACK::lmdif(m, n, xx.begin(), fvec.get(),
                       endCriteria.functionEpsilon(),
                       xtol_,
                       gtol_,
                       maxfev,
                       epsfcn_,
                       diag.get(), mode, factor,
                       nprint, &info, &nfev, fjac.get(),
                       ldfjac, ipvt.get(), qtf.get(),
                       wa1.get(), wa2.get(), wa3.get(), wa4.get(),
                       lmdifCostFunction,
                       lmdifJacFunction);
        // for the time being
        info_ = info;
        // check requirements & endCriteria evaluation
        QL_REQUIRE(info != 0, "MINPACK: improper input parameters");
        QL_REQUIRE(info != 7, "MINPACK: xtol is too small. no further "
                                       "improvement in the approximate "
                                       "solution x is possible.");
        QL_REQUIRE(info != 8, "MINPACK: gtol is too small. fvec is "
                                       "orthogonal to the columns of the "
                                       "jacobian to machine precision.");

        EndCriteria::Type ecType = EndCriteria::None;
        switch (info) {
          case 1:
          case 2:
          case 3:
          case 4:
            // 2 and 3 should be StationaryPoint, 4 a new gradient-related value,
            // but we keep StationaryFunctionValue for backwards compatibility.
            ecType = EndCriteria::StationaryFunctionValue;
            break;
          case 5:
            ecType = EndCriteria::MaxIterations;
            break;
          case 6:
            ecType = EndCriteria::FunctionEpsilonTooSmall;
            break;
          default:
            QL_FAIL("unknown MINPACK result: " << info);
        }
        // set problem
        P.setCurrentValue(std::move(xx));
        P.setFunctionValue(P.costFunction().value(P.currentValue()));

        return ecType;
    }

    void LevenbergMarquardt::fcn(int, int n, Real* x, Real* fvec) {
        Array xt(n);
        std::copy(x, x+n, xt.begin());
        // constraint handling needs some improvement in the future:
        // starting point should not be close to a constraint violation
        if (currentProblem_->constraint().test(xt)) {
            const Array& tmp = currentProblem_->values(xt);
            std::copy(tmp.begin(), tmp.end(), fvec);
        } else {
            std::copy(initCostValues_.begin(), initCostValues_.end(), fvec);
        }
    }

    void LevenbergMarquardt::jacFcn(int m, int n, Real* x, Real* fjac) {
        Array xt(n);
        std::copy(x, x+n, xt.begin());
        // constraint handling needs some improvement in the future:
        // starting point should not be close to a constraint violation
        if (currentProblem_->constraint().test(xt)) {
            Matrix tmp(m,n);
            currentProblem_->costFunction().jacobian(tmp, xt);
            Matrix tmpT = transpose(tmp);
            std::copy(tmpT.begin(), tmpT.end(), fjac);
        } else {
            Matrix tmpT = transpose(initJacobian_);
            std::copy(tmpT.begin(), tmpT.end(), fjac);
        }
    }

}
