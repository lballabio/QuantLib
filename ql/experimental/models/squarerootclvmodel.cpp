/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Klaus Spanderen

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

/*! \file squarerootclvmodel.cpp
    \brief CLV model with a square root kernel process
*/

#include <ql/processes/blackscholesprocess.hpp>
#include <ql/processes/squarerootprocess.hpp>
#include <ql/math/integrals/gaussianquadratures.hpp>

#include <ql/experimental/models/squarerootclvmodel.hpp>
#include <ql/methods/finitedifferences/utilities/gbsmrndcalculator.hpp>

#include <boost/math/distributions/non_central_chi_squared.hpp>

#include <utility>

namespace QuantLib {
    SquareRootCLVModel::SquareRootCLVModel(
        const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess,
        ext::shared_ptr<SquareRootProcess> sqrtProcess,
        std::vector<Date> maturityDates,
        Size lagrangeOrder,
        Real pMax,
        Real pMin)
    : pMax_(pMax), pMin_(pMin), bsProcess_(bsProcess), sqrtProcess_(std::move(sqrtProcess)),
      maturityDates_(std::move(maturityDates)), lagrangeOrder_(lagrangeOrder),
      rndCalculator_(ext::make_shared<GBSMRNDCalculator>(bsProcess)) {}

    Real SquareRootCLVModel::cdf(const Date& d, Real k) const {
        return rndCalculator_->cdf(k, bsProcess_->time(d));
    }


    Real SquareRootCLVModel::invCDF(const Date& d, Real q) const {
        return rndCalculator_->invcdf(q, bsProcess_->time(d));
    }

    std::pair<Real, Real> SquareRootCLVModel::nonCentralChiSquaredParams(
        const Date& d) const {

        const Time t = bsProcess_->time(d);

        const Real kappa = sqrtProcess_->a();
        const Real theta = sqrtProcess_->b();
        const Real sigma = sqrtProcess_->sigma();

        const Real df  = 4*theta*kappa/(sigma*sigma);
        const Real ncp = 4*kappa*std::exp(-kappa*t)
            / (sigma*sigma*(1-std::exp(-kappa*t)))*sqrtProcess_->x0();

        return std::make_pair(df, ncp);
    }


    Array SquareRootCLVModel::collocationPointsX(const Date& d) const {

        const std::pair<Real, Real> p = nonCentralChiSquaredParams(d);

        Array x = GaussianQuadrature(lagrangeOrder_,
            GaussNonCentralChiSquaredPolynomial(p.first, p.second))
             .x();

        std::sort(x.begin(), x.end());

        const boost::math::non_central_chi_squared_distribution<Real>
            dist(p.first, p.second);

        const Real xMin = std::max(x.front(),
            (pMin_ == Null<Real>())
                ? 0.0 : boost::math::quantile(dist, pMin_));

        const Real xMax = std::min(x.back(),
            (pMax_ == Null<Real>())
            ? QL_MAX_REAL : boost::math::quantile(dist, pMax_));

        const Real b = xMin - x.front();
        const Real a = (xMax - xMin)/(x.back() - x.front());

        for (Real& i : x) {
            i = a * i + b;
        }

        return x;
    }

    Array SquareRootCLVModel::collocationPointsY(const Date& d) const {

        const Array x = collocationPointsX(d);
        const std::pair<Real, Real> params = nonCentralChiSquaredParams(d);
        const boost::math::non_central_chi_squared_distribution<Real>
            dist(params.first, params.second);

        Array s(x.size());
        for (Size i=0, n=s.size(); i < n; ++i) {
            const Real q = boost::math::cdf(dist, x[i]);

            s[i] = invCDF(d, q);
        }

        return s;
    }

    std::function<Real(Time, Real)> SquareRootCLVModel::g() const {
        calculate();
        return g_;
    }

    void SquareRootCLVModel::performCalculations() const {
        g_ = std::function<Real(Time, Real)>(MappingFunction(*this));
    }

    SquareRootCLVModel::MappingFunction::MappingFunction(
        const SquareRootCLVModel& model)
    : s_(ext::make_shared<Matrix>(
         model.maturityDates_.size(), model.lagrangeOrder_)),
      x_(ext::make_shared<Matrix>(
         model.maturityDates_.size(), model.lagrangeOrder_)) {

        std::vector<Date> maturityDates = model.maturityDates_;
        std::sort(maturityDates.begin(), maturityDates.end());

        const ext::shared_ptr<GeneralizedBlackScholesProcess>&
            bsProcess = model.bsProcess_;

        for (Size i=0, n = maturityDates.size(); i < n; ++i) {
            const Date maturityDate = maturityDates[i];

            const Array x = model.collocationPointsX(maturityDate);
            const Array y = model.collocationPointsY(maturityDate);

            std::copy(x.begin(), x.end(), x_->row_begin(i));
            std::copy(y.begin(), y.end(), s_->row_begin(i));

            const Time maturity = bsProcess->time(maturityDate);

            interpl.insert(
                std::make_pair(maturity,
                    ext::make_shared<LagrangeInterpolation>(
                        x_->row_begin(i), x_->row_end(i),
                        s_->row_begin(i))));
        }
    }

    Real SquareRootCLVModel::MappingFunction::operator()(Time t,Real x) const {
        const interpl_type::const_iterator ge = interpl.lower_bound(t);

        if (close_enough(ge->first, t)) {
            return (*ge->second)(x, true);
        }

        QL_REQUIRE(ge != interpl.end() && ge != interpl.begin(),
             "extrapolation to large or small t is not allowed");

        const Time t1 = ge->first;
        const Real y1 = (*ge->second)(x, true);

        interpl_type::const_iterator lt = ge;
        std::advance(lt, -1);

        const Time t0 = lt->first;
        const Real y0 = (*lt->second)(x, true);

        return y0 + (y1 - y0)/(t1 - t0)*(t - t0);
    }
}
