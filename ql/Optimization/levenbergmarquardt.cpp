/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include <ql/Optimization/lmdif.hpp>
#include <ql/Optimization/levenbergmarquardt.hpp>

namespace QuantLib {

    const Problem* LevenbergMarquardt::_thisP(0);
    Array LevenbergMarquardt::_initCostValues(0);

    LevenbergMarquardt::LevenbergMarquardt(Real epsfcn,
                                           Real ftol, Real xtol,
                                           Real gtol, Size maxfev)
    : info_(0), maxfev_(maxfev), epsfcn_(epsfcn),
      ftol_(ftol), xtol_(xtol), gtol_(gtol) {}

    Integer LevenbergMarquardt::getInfo() const {
        return info_;
    }

    void LevenbergMarquardt::minimize(const Problem& P) const {
        _thisP = &P;
        _initCostValues = P.costFunction().values(x());

        int m = _initCostValues.size();
        int n = x().size();
        boost::scoped_array<double> xx(new double[n]);
        std::copy(x_.begin(), x_.end(), xx.get());
        boost::scoped_array<double> fvec(new double[m]);
        double ftol = ftol_;
        double xtol = xtol_;
        double gtol = gtol_;
        int maxfev = maxfev_;
        double epsfcn = epsfcn_;
        boost::scoped_array<double> diag(new double[n]);
        int mode = 1;
        double factor = 1;
        int nprint = 0;
        int info = 0;
        int nfev =0;
        boost::scoped_array<double> fjac(new double[m*n]);
        int ldfjac = m;
        boost::scoped_array<int> ipvt(new int[n]);
        boost::scoped_array<double> qtf(new double[n]);
        boost::scoped_array<double> wa1(new double[n]);
        boost::scoped_array<double> wa2(new double[n]);
        boost::scoped_array<double> wa3(new double[n]);
        boost::scoped_array<double> wa4(new double[m]);

        QuantLib::MINPACK::lmdif(m, n, xx.get(), fvec.get(), ftol,
                                 xtol, gtol, maxfev, epsfcn,
                                 diag.get(), mode, factor,
                                 nprint, &info, &nfev, fjac.get(),
                                 ldfjac, ipvt.get(), qtf.get(),
                                 wa1.get(), wa2.get(), wa3.get(), wa4.get());

        info_ = info;

        QL_REQUIRE(info != 0, "MINPACK: improper input parameters");
        QL_REQUIRE(info != 5, "MINPACK: number of calls to fcn has "
                                       "reached or exceeded maxfev.");
        QL_REQUIRE(info != 6, "MINPACK: ftol is too small. no further "
                                       "reduction in the sum of squares "
                                       "is possible.");
        QL_REQUIRE(info != 7, "MINPACK: xtol is too small. no further "
                                       "improvement in the approximate "
                                       "solution x is possible.");
        QL_REQUIRE(info != 8, "MINPACK: gtol is too small. fvec is "
                                       "orthogonal to the columns of the "
                                       "jacobian to machine precision.");

        std::copy(xx.get(), xx.get()+n, x_.begin());
    }

    void LevenbergMarquardt::fcn(int m, int n,
                                 double* x, double* fvec, int* iflag) {
        Array xt(n);
        std::copy(x, x+n, xt.begin());

        // constraint handling needs some improvement in the future
        // starting point shouldn't be close to a constraint violation
        Array result;
        if (_thisP->constraint().test(xt)) {
            const Array& tmp = _thisP->values(xt);
            std::copy(tmp.begin(), tmp.end(), fvec);
        } else {
            std::copy(_initCostValues.begin(), _initCostValues.end(), fvec);
        }

    }

}

