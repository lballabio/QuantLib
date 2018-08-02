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
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>

#include <set>
#include <algorithm>

namespace QuantLib {

    namespace {

        struct interpolated_volatility {
            interpolated_volatility(const std::vector<Real>& pGrid,
                                    const std::vector<Real>& vGrid)
            : variance(pGrid.begin(), pGrid.end(), vGrid.begin()) {}
            Real operator()(Real x) const {
                return std::sqrt(variance(x));
            }
            LinearInterpolation variance;
        };

    }

    FdmHestonVarianceMesher::FdmHestonVarianceMesher(
        Size size,
        const ext::shared_ptr<HestonProcess> & process,
        Time maturity, Size tAvgSteps, Real epsilon)
        : Fdm1dMesher(size) {

        std::vector<Real> vGrid(size, 0.0), pGrid(size, 0.0);
        const Real df  = 4*process->theta()*process->kappa()/
                            square<Real>()(process->sigma());
        try {
            std::multiset<std::pair<Real, Real> > grid;
            
            for (Size l=1; l<=tAvgSteps; ++l) {
                const Real t = (maturity*l)/tAvgSteps;
                const Real ncp = 4*process->kappa()*std::exp(-process->kappa()*t)
                    /(square<Real>()(process->sigma())
                    *(1-std::exp(-process->kappa()*t)))*process->v0();
                const Real k = square<Real>()(process->sigma())
                    *(1-std::exp(-process->kappa()*t))/(4*process->kappa());

                const Real qMin = 0.0; // v_min = 0.0;
                const Real qMax = std::max(process->v0(),
                    k*InverseNonCentralCumulativeChiSquareDistribution(
                                            df, ncp, 100, 1e-8)(1-epsilon));

                const Real minVStep=(qMax-qMin)/(50*size);
                Real ps,p = 0.0;

                Real vTmp = qMin;
                grid.insert(std::pair<Real, Real>(qMin, epsilon));
                
                for (Size i=1; i < size; ++i) {
                    ps = (1 - epsilon - p)/(size-i);
                    p += ps;
                    const Real tmp = k*InverseNonCentralCumulativeChiSquareDistribution(
                        df, ncp, 100, 1e-8)(p);

                    const Real vx = std::max(vTmp+minVStep, tmp);
                    p = NonCentralCumulativeChiSquareDistribution(df, ncp)(vx/k);
                    vTmp=vx;
                    grid.insert(std::pair<Real, Real>(vx, p));
                }
            }
            QL_REQUIRE(grid.size() == size*tAvgSteps, 
                       "something wrong with the grid size");
            
            std::vector<std::pair<Real, Real> > tp(grid.begin(), grid.end());

            for (Size i=0; i < size; ++i) {
                const Size b = (i*tp.size())/size;
                const Size e = ((i+1)*tp.size())/size;
                for (Size j=b; j < e; ++j) {
                    vGrid[i]+=tp[j].first/(e-b);
                    pGrid[i]+=tp[j].second/(e-b);
                }
            }
        } 
        catch (const Error&) {
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

        Real skewHint = ((process->kappa() != 0.0) 
                ? std::max(1.0, process->sigma()/process->kappa()) : 1.0);

        std::sort(pGrid.begin(), pGrid.end());
        volaEstimate_ = GaussLobattoIntegral(100000, 1e-4)(
            interpolated_volatility(pGrid, vGrid),
            pGrid.front(), pGrid.back())*std::pow(skewHint, 1.5);

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
