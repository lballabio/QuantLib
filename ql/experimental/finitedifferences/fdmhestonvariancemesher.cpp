/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008 Klaus Spanderen

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

#include <ql/math/functional.hpp>
#include <ql/math/distributions/chisquaredistribution.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/experimental/finitedifferences/fdmhestonvariancemesher.hpp>

namespace QuantLib {

    FdmHestonVarianceMesher::FdmHestonVarianceMesher(
        Size size,
        const boost::shared_ptr<HestonProcess> & process,
        Time maturity)
        : Fdm1dMesher(size) {

        const Size avgSteps = 10;
        const Real epsilon=0.0001;

        std::vector<Real> vGrid(size, 0.0), pGrid(size, 0.0);
        const Real df  = 4*process->theta()*process->kappa()/
            square<Real>()(process->sigma());
        try {
            for (Size l=1; l<=avgSteps; ++l) {
                const Real t = (maturity*l)/avgSteps;
                const Real ncp = 4*process->kappa()*std::exp(-process->kappa()*t)
                    /(square<Real>()(process->sigma())
                    *(1-std::exp(-process->kappa()*t)))*process->v0();
                const Real k = square<Real>()(process->sigma())
                    *(1-std::exp(-process->kappa()*t))/(4*process->kappa());

                const Real qMin = std::min(process->v0(),
                    k*InverseNonCentralChiSquareDistribution(
                    df, ncp, 100, 1e-16)(epsilon));
                const Real qMax = std::max(process->v0(),
                    k*InverseNonCentralChiSquareDistribution(
                    df, ncp, 100,  1e-16)(1-epsilon));

                const Real minVStep=(qMax-qMin)/(50*size);
                Real ps,p = epsilon;

                std::vector<Real> vTmp(size);
                vTmp[0]=qMin;
                pGrid[0]+=epsilon;
                for (Size i=1; i < size; ++i) {
                    ps = (1 - epsilon - p)/(size-i);
                    p += ps;
                    const Real tmp = k*InverseNonCentralChiSquareDistribution(
                        df, ncp, 100, 1e-16)(p);

                    const Real vx = std::max(vTmp[i-1]+minVStep, tmp);
                    p = NonCentralChiSquareDistribution(df, ncp)(vx/k);
                    vTmp[i]= vx;
                    pGrid[i]+= p;
                }
                std::transform(vTmp.begin(), vTmp.end(), vGrid.begin(),
                    vGrid.begin(), std::plus<Real>());
            }
            QL_REQUIRE(vGrid.size() == size, "somethings wrong with the vgrid");

            std::transform(vGrid.begin(), vGrid.end(), vGrid.begin(),
                std::bind2nd(std::divides<Real>(), Real(avgSteps)));
            std::transform(pGrid.begin(), pGrid.end(), pGrid.begin(),
                std::bind2nd(std::divides<Real>(), Real(avgSteps)));
        } catch (const QuantLib::Error&) {
            // use default mesh
            const Real vol = process->sigma()*
                std::sqrt(process->theta()/(2*process->kappa()));

            const Real mean = process->theta();
            const Real upperBound = std::max(process->v0()+4*vol, mean+4*vol);
            const Real lowerBound
                = std::max(0.0, std::min(process->v0()-4*vol, mean-4*vol));

            for (Size i=0; i < size; ++i) {
                pGrid[i] = i/(size-1.0);
                vGrid[i] = lowerBound + i*(upperBound-lowerBound)/(size-1.0);
            }
        }

        volaEstimate_ = GaussLobattoIntegral(1000, 1e-5)(
            boost::function1<Real, Real>(
                compose(std::ptr_fun<Real, Real>(std::sqrt),
                        LinearInterpolation(pGrid.begin(), pGrid.end(),
                        vGrid.begin()))),
            pGrid.front(), pGrid.back());

        const Real v0 = process->v0();
        for (Size i=1; i<vGrid.size(); ++i) {
            if (vGrid[i-1] <= v0 && vGrid[i] >= v0) {
                if (std::fabs(vGrid[i-1] - v0) < std::fabs(vGrid[i] - v0))
                    vGrid[i-1] = v0;
                else
                    vGrid[i] = v0;
            }
        }

        std::copy(vGrid.begin(), vGrid.end(), locations_.begin());

        for (Size i=0; i < size-1; ++i) {
            dminus_[i+1] = dplus_[i] = vGrid[i+1] - vGrid[i];
        }
        dplus_.back() = dminus_.front() = Null<Real>();
    }
}
