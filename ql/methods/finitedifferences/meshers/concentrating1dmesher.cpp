/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Ralph Schreyer
 Copyright (C) 2014 Johannes Göttker-Schnetmann
 Copyright (C) 2014 Klaus Spanderen
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

/*! \file concentrating1dmesher.cpp
    \brief One-dimensional grid mesher concentrating around critical points
*/ 

#include <ql/errors.hpp>
#include <ql/timegrid.hpp>
#include <ql/utilities/null.hpp>
#include <ql/math/array.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/ode/adaptiverungekutta.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/functional.hpp>
#include <cmath>

// asinh is missing in WIN32 (and possibly on other compilers)
#if !defined(QL_HAVE_ASINH)
#define asinh(x) std::log(x + std::sqrt(x * x + 1))
#endif

namespace QuantLib {

    Concentrating1dMesher::Concentrating1dMesher(
        Real start, Real end, Size size, const std::pair<Real, Real>& cPoints,
        const bool requireCPoint)
        : Fdm1dMesher(size) {

        QL_REQUIRE(end > start, "end must be larger than start");

        const Real cPoint = cPoints.first;
        const Real density = cPoints.second == Null<Real>() ?
            Null<Real>() : cPoints.second*(end - start);

        QL_REQUIRE(cPoint == Null<Real>() || (cPoint >= start && cPoint <= end),
            "cPoint must be between start and end");
        QL_REQUIRE(density == Null<Real>() || density > 0.0,
            "density > 0 required");
        QL_REQUIRE(cPoint == Null<Real>() || density != Null<Real>(),
            "density must be given if cPoint is given");
        QL_REQUIRE(!requireCPoint || cPoint != Null<Real>(),
            "cPoint is required in grid but not given");

        const Real dx = 1.0 / (size - 1);

        if (cPoint != Null<Real>()) {
            std::vector<Real> u, z;
            ext::shared_ptr<Interpolation> transform;
            const Real c1 = asinh((start - cPoint) / density);
            const Real c2 = asinh((end - cPoint) / density);
            if (requireCPoint) {
                u.push_back(0.0);
                z.push_back(0.0);
                if (!close(cPoint, start) && !close(cPoint, end)) {
                    const Real z0 = -c1 / (c2 - c1);
                    const Real u0 =
                        std::max(
                            std::min(static_cast<int>(z0 * (size - 1) + 0.5),
                                static_cast<int>(size) - 2),
                            1) /
                        ((Real)(size - 1));
                    u.push_back(u0);
                    z.push_back(z0);
                }
                u.push_back(1.0);
                z.push_back(1.0);
                transform = ext::shared_ptr<Interpolation>(
                    new LinearInterpolation(u.begin(), u.end(), z.begin()));
            }

            for (Size i = 1; i < size - 1; ++i) {
                const Real li = requireCPoint ? (*transform)(i*dx) : i*dx;
                locations_[i] = cPoint
                    + density*std::sinh(c1*(1.0 - li) + c2*li);
            }
        }
        else {
            for (Size i = 1; i < size - 1; ++i) {
                locations_[i] = start + i*dx*(end - start);
            }
        }

        locations_.front() = start;
        locations_.back() = end;

        for (Size i = 0; i < size - 1; ++i) {
            dplus_[i] = dminus_[i + 1] = locations_[i + 1] - locations_[i];
        }
        dplus_.back() = dminus_.front() = Null<Real>();
    }

    namespace {
        class OdeIntegrationFct {
          public:
            OdeIntegrationFct(const std::vector<Real>& points,
                              const std::vector<Real>& betas,
                              Real tol)
          : rk_(tol), points_(points), betas_(betas) {}

            Real solve(Real a, Real y0, Real x0, Real x1) {
                using namespace ext::placeholders;
                AdaptiveRungeKutta<>::OdeFct1d odeFct(
                    ext::bind(&OdeIntegrationFct::jac, this, a, _1, _2));

                return rk_(odeFct, y0, x0, x1);
            }

          private:
            Real jac(Real a, Real, Real y) const {
                Real s=0.0;
                for (Size i=0; i < points_.size(); ++i) {
                    s+=1.0/(betas_[i] + square<Real>()(y - points_[i]));
                }
                return a/std::sqrt(s);
            }

            AdaptiveRungeKutta<> rk_;
            const std::vector<Real> &points_, &betas_;
        };

        bool equal_on_first(const std::pair<Real, Real>& p1,
                            const std::pair<Real, Real>& p2) {
            return close_enough(p1.first, p2.first, 1000);
        }
    }


    Concentrating1dMesher::Concentrating1dMesher(
        Real start, Real end, Size size,
        const std::vector<boost::tuple<Real, Real, bool> >& cPoints,
        Real tol)
    : Fdm1dMesher(size) {
        using namespace ext::placeholders;

        QL_REQUIRE(end > start, "end must be larger than start");

        std::vector<Real> points, betas;
        for (std::vector<boost::tuple<Real, Real, bool> >::const_iterator
                iter = cPoints.begin(); iter != cPoints.end(); ++iter) {
            points.push_back(iter->get<0>());
            betas.push_back(square<Real>()(iter->get<1>()*(end-start)));
        }

        // get scaling factor a so that y(1) = end
        Real aInit = 0.0;
        for (Size i=0; i < points.size(); ++i) {
            const Real c1 = asinh((start-points[i])/betas[i]);
            const Real c2 = asinh((end-points[i])/betas[i]);
            aInit+=(c2-c1)/points.size();
        }

        OdeIntegrationFct fct(points, betas, tol);
        const Real a = Brent().solve(
            ext::bind(std::minus<Real>(),
                ext::bind(&OdeIntegrationFct::solve,
                            &fct, _1, start, 0.0, 1.0), end),
            tol, aInit, 0.1*aInit);

        // solve ODE for all grid points
        Array x(size), y(size);
        x[0] = 0.0; y[0] = start;
        const Real dx = 1.0/(size-1);
        for (Size i=1; i < size; ++i) {
            x[i] = i*dx;
            y[i] = fct.solve(a, y[i-1], x[i-1], x[i]);
        }

        // eliminate numerical noise and ensure y(1) = end
        const Real dy = y.back() - end;
        for (Size i=1; i < size; ++i) {
            y[i] -= i*dx*dy;
        }

        LinearInterpolation odeSolution(x.begin(), x.end(), y.begin());

        // ensure required points are part of the grid
        std::vector<std::pair<Real, Real> > w(1, std::make_pair(0.0, 0.0));

        for (Size i=0; i < points.size(); ++i) {
            if (cPoints[i].get<2>() && points[i] > start && points[i] < end) {

                const Size j = std::distance(y.begin(),
                        std::lower_bound(y.begin(), y.end(), points[i]));

                const Real e = Brent().solve(
                    ext::bind(std::minus<Real>(),
                        ext::bind(&LinearInterpolation::operator(),
                                    odeSolution, _1, true), points[i]),
                    QL_EPSILON, x[j], 0.5/size);

                w.push_back(std::make_pair(std::min(x[size-2], x[j]), e));
            }
        }
        w.push_back(std::make_pair(1.0, 1.0));
        std::sort(w.begin(), w.end());
        w.erase(std::unique(w.begin(), w.end(), equal_on_first), w.end());

        std::vector<Real> u(w.size()), z(w.size());
        for (Size i=0; i < w.size(); ++i) {
            u[i] = w[i].first;
            z[i] = w[i].second;
        }
        LinearInterpolation transform(u.begin(), u.end(), z.begin());

        for (Size i=0; i < size; ++i) {
            locations_[i] = odeSolution(transform(i*dx));
        }

        for (Size i=0; i < size-1; ++i) {
            dplus_[i] = dminus_[i+1] = locations_[i+1] - locations_[i];
        }
        dplus_.back() = dminus_.front() = Null<Real>();
    }
}
