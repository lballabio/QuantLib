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
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/lmdif.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/functional.hpp>

namespace QuantLib {

    LevenbergMarquardt::LevenbergMarquardt(Real epsfcn,
                                           Real xtol,
                                           Real gtol,
                                           bool useCostFunctionsJacobian)
        : info_(0), epsfcn_(epsfcn), xtol_(xtol), gtol_(gtol),
          useCostFunctionsJacobian_(useCostFunctionsJacobian) {}

    Integer LevenbergMarquardt::getInfo() const {
        return info_;
    }

    EndCriteria::Type LevenbergMarquardt::minimize(Problem& P,
                                                   const EndCriteria& endCriteria) {
        using namespace ext::placeholders;

        EndCriteria::Type ecType = EndCriteria::None;
        P.reset();
        Array x_ = P.currentValue();
        currentProblem_ = &P;
        initCostValues_ = P.costFunction().values(x_);
        int m = initCostValues_.size();
        int n = x_.size();
        if(useCostFunctionsJacobian_) {
            initJacobian_ = Matrix(m,n);
            P.costFunction().jacobian(initJacobian_, x_);
        }
        boost::scoped_array<Real> xx(new Real[n]);
        std::copy(x_.begin(), x_.end(), xx.get());
        boost::scoped_array<Real> fvec(new Real[m]);
        boost::scoped_array<Real> diag(new Real[n]);
        int mode = 1;
        Real factor = 1;
        int nprint = 0;
        int info = 0;
        int nfev =0;
        boost::scoped_array<Real> fjac(new Real[m*n]);
        int ldfjac = m;
        boost::scoped_array<int> ipvt(new int[n]);
        boost::scoped_array<Real> qtf(new Real[n]);
        boost::scoped_array<Real> wa1(new Real[n]);
        boost::scoped_array<Real> wa2(new Real[n]);
        boost::scoped_array<Real> wa3(new Real[n]);
        boost::scoped_array<Real> wa4(new Real[m]);
        // requirements; check here to get more detailed error messages.
        QL_REQUIRE(n > 0, "no variables given");
        QL_REQUIRE(m >= n,
                   "less functions (" << m <<
                   ") than available variables (" << n << ")");
        QL_REQUIRE(endCriteria.functionEpsilon() >= 0.0,
                   "negative f tolerance");
        QL_REQUIRE(xtol_ >= 0.0, "negative x tolerance");
        QL_REQUIRE(gtol_ >= 0.0, "negative g tolerance");
        QL_REQUIRE(endCriteria.maxIterations() > 0,
                   "null number of evaluations");

        // call lmdif to minimize the sum of the squares of m functions
        // in n variables by the Levenberg-Marquardt algorithm.
        MINPACK::LmdifCostFunction lmdifCostFunction =
            ext::bind(&LevenbergMarquardt::fcn, this, _1, _2, _3, _4, _5);
        MINPACK::LmdifCostFunction lmdifJacFunction =
            useCostFunctionsJacobian_
                ? ext::bind(&LevenbergMarquardt::jacFcn, this, _1, _2, _3,
                              _4, _5)
                : MINPACK::LmdifCostFunction();
        MINPACK::lmdif(m, n, xx.get(), fvec.get(),
                       endCriteria.functionEpsilon(),
                       xtol_,
                       gtol_,
                       endCriteria.maxIterations(),
                       epsfcn_,
                       diag.get(), mode, factor,
                       nprint, &info, &nfev, fjac.get(),
                       ldfjac, ipvt.get(), qtf.get(),
                       wa1.get(), wa2.get(), wa3.get(), wa4.get(),
                       lmdifCostFunction,
                       lmdifJacFunction);
        info_ = info;
        // check requirements & endCriteria evaluation
        QL_REQUIRE(info != 0, "MINPACK: improper input parameters");
        //QL_REQUIRE(info != 6, "MINPACK: ftol is too small. no further "
        //                               "reduction in the sum of squares "
        //                               "is possible.");
        if (info != 6) ecType = QuantLib::EndCriteria::StationaryFunctionValue;
        //QL_REQUIRE(info != 5, "MINPACK: number of calls to fcn has "
        //                               "reached or exceeded maxfev.");
        endCriteria.checkMaxIterations(nfev, ecType);
        QL_REQUIRE(info != 7, "MINPACK: xtol is too small. no further "
                                       "improvement in the approximate "
                                       "solution x is possible.");
        QL_REQUIRE(info != 8, "MINPACK: gtol is too small. fvec is "
                                       "orthogonal to the columns of the "
                                       "jacobian to machine precision.");
        // set problem
        std::copy(xx.get(), xx.get()+n, x_.begin());
        P.setCurrentValue(x_);
        P.setFunctionValue(P.costFunction().value(x_));

        return ecType;
    }

    void LevenbergMarquardt::fcn(int, int n, Real* x, Real* fvec, int*) {
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

    void LevenbergMarquardt::jacFcn(int m, int n, Real* x, Real* fjac, int*) {
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
