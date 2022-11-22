/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Andreas Gaida
 Copyright (C) 2008 Ralph Schreyer
 Copyright (C) 2008, 2019 Klaus Spanderen

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
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/distributions/chisquaredistribution.hpp>
#include <ql/math/interpolations/linearinterpolation.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/termstructures/volatility/equityfx/localvoltermstructure.hpp>
#include <ql/methods/finitedifferences/meshers/fdmhestonvariancemesher.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <set>
#include <algorithm>

namespace QuantLib {

    namespace {
        struct interpolated_volatility {
            interpolated_volatility(const std::vector<Real>& pGrid,
                                    const std::vector<Real>& vGrid)
            : variance(pGrid.begin(), pGrid.end(), vGrid.begin()) {}
            Real operator()(Real x) const {
                return std::sqrt(variance(x, true));
            }
            LinearInterpolation variance;
        };
    }

    FdmHestonVarianceMesher::FdmHestonVarianceMesher(
        Size size,
        const ext::shared_ptr<HestonProcess> & process,
        Time maturity, Size tAvgSteps, Real epsilon,
        Real mixingFactor)
        : Fdm1dMesher(size) {

        std::vector<Real> vGrid(size, 0.0), pGrid(size, 0.0);
        const Real mixedSigma = process->sigma()*mixingFactor;
        const Real df  = 4*process->theta()*process->kappa()/squared(mixedSigma);
        try {
            std::multiset<std::pair<Real, Real> > grid;
            
            for (Size l=1; l<=tAvgSteps; ++l) {
                const Real t = (maturity*l)/tAvgSteps;
                const Real ncp = 4*process->kappa()*std::exp(-process->kappa()*t)/(squared(mixedSigma)
                    *(1-std::exp(-process->kappa()*t)))*process->v0();
                const Real k = squared(mixedSigma)
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
            
            const std::vector<std::pair<Real, Real> > tp(grid.begin(), grid.end());

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
            const Real vol = mixedSigma*
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
                ? Real(std::max(1.0, mixedSigma/process->kappa())) : 1.0);

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


    FdmHestonLocalVolatilityVarianceMesher::FdmHestonLocalVolatilityVarianceMesher(
        Size size,
        const ext::shared_ptr<HestonProcess>& process,
        const ext::shared_ptr<LocalVolTermStructure>& leverageFct,
        Time maturity, Size tAvgSteps, Real epsilon,
        Real mixingFactor)
     : Fdm1dMesher(size) {

        const FdmHestonVarianceMesher mesher(
            size, process, maturity, tAvgSteps, epsilon, mixingFactor);

        for (Size i=0; i < size; ++i) {
            dplus_[i] = mesher.dplus(i);
            dminus_[i] = mesher.dminus(i);
            locations_[i] = mesher.location(i);
        }

        volaEstimate_ = mesher.volaEstimate();

        if (leverageFct != nullptr) {
            typedef boost::accumulators::accumulator_set<
                Real, boost::accumulators::stats<
                    boost::accumulators::tag::mean> >
                accumulator_set;

            accumulator_set acc;

            const Real s0 = process->s0()->value();

            acc(leverageFct->localVol(0, s0, true));

            const Handle<YieldTermStructure> rTS = process->riskFreeRate();
            const Handle<YieldTermStructure> qTS = process->dividendYield();

            for (Size l=1; l <= tAvgSteps; ++l) {
                const Real t = (maturity*l)/tAvgSteps;
                const Real vol = volaEstimate_ * boost::accumulators::mean(acc);

                const Real fwd = s0*qTS->discount(t)/rTS->discount(t);

                const Size sAvgSteps = 50;

                std::vector<Real> u(sAvgSteps), sig(sAvgSteps);

                for (Size i=0; i < sAvgSteps; ++i) {
                    u[i] = epsilon + ((1-2*epsilon)/(sAvgSteps-1))*i;
                    const Real x = InverseCumulativeNormal()(u[i]);

                    const Real gf = x*vol*std::sqrt(t);
                    const Real f = fwd*std::exp(gf);

                    sig[i] = squared(leverageFct->localVol(t, f, true));
                }

                const Real leverageAvg =
                    GaussLobattoIntegral(10000, 1e-4)(
                        interpolated_volatility(u, sig), u.front(), u.back())
                    / (1-2*epsilon);

                acc(leverageAvg);
            }
            volaEstimate_ *= boost::accumulators::mean(acc);
        }
    }
}
