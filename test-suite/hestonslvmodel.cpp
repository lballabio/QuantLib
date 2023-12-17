/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
  Copyright (C) 2014, 2015 Johannes Göttker-Schnetmann
  Copyright (C) 2014, 2015 Klaus Spanderen

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

#include "preconditions.hpp"
#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/exoticoptions/analyticpdfhestonengine.hpp>
#include <ql/instruments/barrieroption.hpp>
#include <ql/instruments/doublebarrieroption.hpp>
#include <ql/instruments/forwardvanillaoption.hpp>
#include <ql/instruments/impliedvolatility.hpp>
#include <ql/math/distributions/gammadistribution.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/integrals/discreteintegrals.hpp>
#include <ql/math/integrals/gausslobattointegral.hpp>
#include <ql/math/interpolations/bicubicsplineinterpolation.hpp>
#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
#include <ql/math/randomnumbers/rngtraits.hpp>
#include <ql/math/randomnumbers/sobolbrownianbridgersg.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/methods/finitedifferences/meshers/concentrating1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmblackscholesmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/meshers/uniform1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmblackscholesfwdop.hpp>
#include <ql/methods/finitedifferences/operators/fdmhestonfwdop.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/fdmlocalvolfwdop.hpp>
#include <ql/methods/finitedifferences/operators/fdmsquarerootfwdop.hpp>
#include <ql/methods/finitedifferences/schemes/craigsneydscheme.hpp>
#include <ql/methods/finitedifferences/schemes/douglasscheme.hpp>
#include <ql/methods/finitedifferences/schemes/hundsdorferscheme.hpp>
#include <ql/methods/finitedifferences/schemes/modifiedcraigsneydscheme.hpp>
#include <ql/methods/finitedifferences/solvers/fdmbackwardsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdmhestongreensfct.hpp>
#include <ql/methods/finitedifferences/utilities/fdmmesherintegral.hpp>
#include <ql/methods/finitedifferences/utilities/localvolrndcalculator.hpp>
#include <ql/methods/finitedifferences/utilities/squarerootprocessrndcalculator.hpp>
#include <ql/models/equity/hestonmodel.hpp>
#include <ql/models/equity/hestonmodelhelper.hpp>
#include <ql/models/equity/hestonslvfdmmodel.hpp>
#include <ql/models/equity/hestonslvmcmodel.hpp>
#include <ql/models/marketmodels/browniangenerators/mtbrowniangenerator.hpp>
#include <ql/models/marketmodels/browniangenerators/sobolbrowniangenerator.hpp>
#include <ql/pricingengines/barrier/analyticdoublebarrierbinaryengine.hpp>
#include <ql/pricingengines/barrier/fdblackscholesbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestonbarrierengine.hpp>
#include <ql/pricingengines/barrier/fdhestondoublebarrierengine.hpp>
#include <ql/pricingengines/forward/forwardengine.hpp>
#include <ql/pricingengines/vanilla/analyticeuropeanengine.hpp>
#include <ql/pricingengines/vanilla/analytichestonengine.hpp>
#include <ql/pricingengines/vanilla/fdblackscholesvanillaengine.hpp>
#include <ql/pricingengines/vanilla/fdhestonvanillaengine.hpp>
#include <ql/pricingengines/vanilla/mceuropeanhestonengine.hpp>
#include <ql/processes/hestonprocess.hpp>
#include <ql/processes/hestonslvprocess.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/equityfx/blackvariancesurface.hpp>
#include <ql/termstructures/volatility/equityfx/fixedlocalvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/gridmodellocalvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/hestonblackvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/localconstantvol.hpp>
#include <ql/termstructures/volatility/equityfx/localvolsurface.hpp>
#include <ql/termstructures/volatility/equityfx/noexceptlocalvolsurface.hpp>
#include <ql/termstructures/yield/zerocurve.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <boost/math/special_functions/gamma.hpp>
#include <boost/multi_array.hpp>
#include <iomanip>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(HestonSLVModelTests)

Real fokkerPlanckPrice1D(const ext::shared_ptr<FdmMesher>& mesher,
                         const ext::shared_ptr<FdmLinearOpComposite>& op,
                         const ext::shared_ptr<StrikedTypePayoff>& payoff,
                         Real x0, Time maturity, Size tGrid) {

    const Array x = mesher->locations(0);
    Array p(x.size(), 0.0);

    QL_REQUIRE(x.size() > 3 && x[1] <= x0 && x[x.size()-2] >= x0,
               "insufficient mesher");

    const Array::const_iterator upperb
        = std::upper_bound(x.begin(), x.end(), x0);
    const Array::const_iterator lowerb = upperb-1;

    if (close_enough(*upperb, x0)) {
        const Size idx = std::distance(x.begin(), upperb);
        const Real dx = (x[idx+1]-x[idx-1])/2.0;
        p[idx] = 1.0/dx;
    }
    else if (close_enough(*lowerb, x0)) {
        const Size idx = std::distance(x.begin(), lowerb);
        const Real dx = (x[idx+1]-x[idx-1])/2.0;
        p[idx] = 1.0/dx;
    } else {
        const Real dx = *upperb - *lowerb;
        const Real lowerP = (*upperb - x0)/dx;
        const Real upperP = (x0 - *lowerb)/dx;

        const Size lowerIdx = std::distance(x.begin(), lowerb);
        const Size upperIdx = std::distance(x.begin(), upperb);

        const Real lowerDx = (x[lowerIdx+1]-x[lowerIdx-1])/2.0;
        const Real upperDx = (x[upperIdx+1]-x[upperIdx-1])/2.0;

        p[lowerIdx] = lowerP/lowerDx;
        p[upperIdx] = upperP/upperDx;
    }

    DouglasScheme evolver(FdmSchemeDesc::Douglas().theta, op);
    const Time dt = maturity/tGrid;
    evolver.setStep(dt);

    for (Time t=dt; t <= maturity+20*QL_EPSILON; t+=dt) {
        evolver.step(p, t);
    }

    Array payoffTimesDensity(x.size());
    for (Size i=0; i < x.size(); ++i) {
        payoffTimesDensity[i] = (*payoff)(std::exp(x[i]))*p[i];
    }

    CubicNaturalSpline f(x.begin(), x.end(), payoffTimesDensity.begin());
    f.enableExtrapolation();
    return GaussLobattoIntegral(1000, 1e-6)(f, x.front(), x.back());
}

Real stationaryLogProbabilityFct(Real kappa, Real theta,
                                 Real sigma, Real z) {
    const Real alpha = 2*kappa*theta/(sigma*sigma);
    const Real beta = alpha/theta;

    return std::pow(beta, alpha)*std::exp(z*alpha)
               *std::exp(-beta*std::exp(z)-GammaFunction().logValue(alpha));
}

ext::shared_ptr<FdmMesher> createStationaryDistributionMesher(
        Real kappa, Real theta, Real sigma, Size vGrid) {

    const Real qMin = 0.01;
    const Real qMax = 0.99;
    const Real dq = (qMax-qMin)/(vGrid-1);

    const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);
    std::vector<Real> v(vGrid);
    for (Size i=0; i < vGrid; ++i) {
        v[i] = rnd.stationary_invcdf(qMin + i*dq);
    }

    return ext::shared_ptr<FdmMesher>(
            ext::make_shared<FdmMesherComposite>(
                ext::make_shared<Predefined1dMesher>(v)));
}

class q_fct {
  public:
    q_fct(const Array& v, const Array& p, const Real alpha)
    : v_(v), q_(Pow(v, alpha)*p), alpha_(alpha),
      spline_(ext::make_shared<CubicNaturalSpline>(v_.begin(), v_.end(), q_.begin())) {
    }

    Real operator()(Real v) const {
        return (*spline_)(v, true)*std::pow(v, -alpha_);
    }
  private:
    const Array v_, q_;
    const Real alpha_;
    const ext::shared_ptr<CubicNaturalSpline> spline_;
};

Real fokkerPlanckPrice2D(const Array& p,
                         const ext::shared_ptr<FdmMesherComposite>& mesher) {

    std::vector<Real> x, y;

    x.reserve(mesher->layout()->dim()[0]);
    y.reserve(mesher->layout()->dim()[1]);

    for (const auto& iter : *mesher->layout()) {
        if (iter.coordinates()[1] == 0U) {
            x.push_back(mesher->location(iter, 0));
        }
        if (iter.coordinates()[0] == 0U) {
            y.push_back(mesher->location(iter, 1));
        }
    }

    return FdmMesherIntegral(mesher,
                             DiscreteSimpsonIntegral()).integrate(p);
}

Real hestonPxBoundary(Time maturity, Real eps,
                      const ext::shared_ptr<HestonModel>& model) {

    const AnalyticPDFHestonEngine pdfEngine(model);
    const Real sInit = model->process()->s0()->value();
    const Real xMin = Brent().solve(
            [&](Real x) -> Real { return pdfEngine.cdf(x, maturity) - eps; },
            sInit*1e-3, sInit, sInit*0.001, 1000*sInit);

    return xMin;
}

struct FokkerPlanckFwdTestCase {
    const Real s0, r, q, v0, kappa, theta, rho, sigma;
    const Size xGrid, vGrid, tGridPerYear, tMinGridPerYear;
    const Real avgEps, eps;
    const FdmSquareRootFwdOp::TransformationType trafoType;
    const FdmHestonGreensFct::Algorithm greensAlgorithm;
    const FdmSchemeDesc::FdmSchemeType schemeType;
};

void hestonFokkerPlanckFwdEquationTest(const FokkerPlanckFwdTestCase& testCase) {

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate = Date(28, Dec, 2014);
    Settings::instance().evaluationDate() = todaysDate;

    std::vector<Period> maturities = {
        Period(1, Months), Period(3, Months), Period(6, Months), Period(9, Months),
        Period(1, Years), Period(2, Years), Period(3, Years)
    };

    const Date maturityDate = todaysDate + maturities.back();
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = testCase.s0;
    const Real x0 = std::log(s0);
    const Rate r = testCase.r;
    const Rate q = testCase.q;

    const Real kappa = testCase.kappa;
    const Real theta = testCase.theta;
    const Real rho   = testCase.rho;
    const Real sigma = testCase.sigma;
    const Real v0    = testCase.v0;
    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const ext::shared_ptr<HestonProcess> process(
            ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const ext::shared_ptr<HestonModel> model(ext::make_shared<HestonModel>(process));

    const ext::shared_ptr<PricingEngine> engine(
            ext::make_shared<AnalyticHestonEngine>(model));

    const Size xGrid = testCase.xGrid;
    const Size vGrid = testCase.vGrid;
    const Size tGridPerYear = testCase.tGridPerYear;

    const FdmSquareRootFwdOp::TransformationType transformationType = testCase.trafoType;
    Real lowerBound, upperBound;
    std::vector<ext::tuple<Real, Real, bool> > cPoints;

    const SquareRootProcessRNDCalculator rnd(v0, kappa, theta, sigma);
    switch (transformationType) {
      case FdmSquareRootFwdOp::Log:
          {
              upperBound = std::log(rnd.stationary_invcdf(0.9995));
              lowerBound = std::log(0.00001);

              const Real v0Center = std::log(v0);
              const Real v0Density = 10.0;
              const Real upperBoundDensity = 100;
              const Real lowerBoundDensity = 1.0;
              cPoints = {
                  ext::make_tuple(lowerBound, lowerBoundDensity, false),
                  ext::make_tuple(v0Center, v0Density, true),
                  ext::make_tuple(upperBound, upperBoundDensity, false)
              };
          }
          break;
      case FdmSquareRootFwdOp::Plain:
          {
              upperBound = rnd.stationary_invcdf(0.9995);
              lowerBound = rnd.stationary_invcdf(1e-5);

              const Real v0Center = v0;
              const Real v0Density = 0.1;
              const Real lowerBoundDensity = 0.0001;
              cPoints = {
                  ext::make_tuple(lowerBound, lowerBoundDensity, false),
                  ext::make_tuple(v0Center, v0Density, true)
              };
          }
          break;
      case FdmSquareRootFwdOp::Power:
          {
              upperBound = rnd.stationary_invcdf(0.9995);
              lowerBound = 0.000075;

              const Real v0Center = v0;
              const Real v0Density = 1.0;
              const Real lowerBoundDensity = 0.005;
              cPoints = {
                  ext::make_tuple(lowerBound, lowerBoundDensity, false),
                  ext::make_tuple(v0Center, v0Density, true)
              };
          }
          break;
      default:
        QL_FAIL("unknown transformation type");
    }

    const ext::shared_ptr<Fdm1dMesher> varianceMesher(
            ext::make_shared<Concentrating1dMesher>(lowerBound, upperBound,
                                                    vGrid, cPoints, 1e-12));

    const Real sEps = 1e-4;
    const Real sLowerBound
        = std::log(hestonPxBoundary(maturity, sEps, model));
    const Real sUpperBound
        = std::log(hestonPxBoundary(maturity, 1-sEps,model));

    const ext::shared_ptr<Fdm1dMesher> spotMesher(
            ext::make_shared<Concentrating1dMesher>(sLowerBound, sUpperBound, xGrid,
                                                    std::make_pair(x0, 0.1), true));

    const ext::shared_ptr<FdmMesherComposite>
            mesher(ext::make_shared<FdmMesherComposite>(spotMesher, varianceMesher));

    const ext::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
            ext::make_shared<FdmHestonFwdOp>(mesher, process, transformationType));

    ModifiedCraigSneydScheme evolver(
                                     FdmSchemeDesc::ModifiedCraigSneyd().theta,
                                     FdmSchemeDesc::ModifiedCraigSneyd().mu, hestonFwdOp);

    // step one days using non-correlated process
    const Time eT = 1.0/365;
    Array p = FdmHestonGreensFct(mesher, process, testCase.trafoType)
        .get(eT, testCase.greensAlgorithm);

    const Real strikes[] = { 50, 80, 90, 100, 110, 120, 150, 200 };

    Time t=eT;
    for (auto maturitie : maturities) {

        // calculate step size
        const Date nextMaturityDate = todaysDate + maturitie;
        const Time nextMaturityTime
            = dc.yearFraction(todaysDate, nextMaturityDate);

        Time dt = (nextMaturityTime - t)/tGridPerYear;
        evolver.setStep(dt);

        for (Size i=0; i < tGridPerYear; ++i, t+=dt) {
            evolver.step(p, t+dt);
        }

        Real avg=0, min=QL_MAX_REAL, max=0;
        for (Real strike : strikes) {
            const ext::shared_ptr<StrikedTypePayoff> payoff(
                    ext::make_shared<PlainVanillaPayoff>((strike > s0) ? Option::Call : Option::Put, strike));

            Array pd(p.size());
            for (const auto& iter : *mesher->layout()) {
                const Size idx = iter.index();
                const Real s = std::exp(mesher->location(iter, 0));

                pd[idx] = (*payoff)(s)*p[idx];
                if (transformationType == FdmSquareRootFwdOp::Power) {
                    const Real v = mesher->location(iter, 1);
                    pd[idx] *= std::pow(v, -alpha);
                }
            }

            const Real calculated = fokkerPlanckPrice2D(pd, mesher)
                                  * rTS->discount(nextMaturityDate);

            const ext::shared_ptr<Exercise> exercise(
                    ext::make_shared<EuropeanExercise>(nextMaturityDate));

            VanillaOption option(payoff, exercise);
            option.setPricingEngine(engine);

            const Real expected = option.NPV();
            const Real absDiff = std::fabs(expected - calculated);
            const Real relDiff = absDiff / std::max(QL_EPSILON, expected);
            const Real diff = std::min(absDiff, relDiff);

            avg += diff;
            min = std::min(diff, min);
            max = std::max(diff, max);

            if (diff > testCase.eps) {
                BOOST_FAIL("failed to reproduce Heston SLV prices at"
                           << "\n   strike      " << strike
                           << "\n   kappa       " << kappa
                           << "\n   theta       " << theta
                           << "\n   rho         " << rho
                           << "\n   sigma       " << sigma
                           << "\n   v0          " << v0
                           << "\n   transform   " << transformationType
                           << std::fixed << std::setprecision(5)
                           << "\n   calculated: " << calculated
                           << "\n   expected:   " << expected
                           << "\n   tolerance:  " << testCase.eps);
            }
        }

        avg/=LENGTH(strikes); // NOLINT(bugprone-integer-division)

        if (avg > testCase.avgEps) {
            BOOST_FAIL("failed to reproduce Heston SLV prices"
                       " on average at"
                       << "\n   kappa       " << kappa
                       << "\n   theta       " << theta
                       << "\n   rho         " << rho
                       << "\n   sigma       " << sigma
                       << "\n   v0          " << v0
                       << "\n   transform   " << transformationType
                       << std::fixed << std::setprecision(5)
                       << "\n   average diff: " << avg
                       << "\n   tolerance:  " << testCase.avgEps);
        }
    }
}

ext::shared_ptr<Matrix>
createLocalVolMatrixFromProcess(const ext::shared_ptr<BlackScholesMertonProcess>& lvProcess,
                                const std::vector<Real>& strikes,
                                const std::vector<Date>& dates,
                                std::vector<Time>& times) {

    const ext::shared_ptr<LocalVolTermStructure> localVol =
        lvProcess->localVolatility().currentLink();

    const DayCounter dc = localVol->dayCounter();
    const Date todaysDate = Settings::instance().evaluationDate();

    QL_REQUIRE(times.size() == dates.size(), "mismatch");

    for (Size i=0; i < times.size(); ++i) {
        times[i] = dc.yearFraction(todaysDate, dates[i]);
    }

    ext::shared_ptr<Matrix> surface(
            ext::make_shared<Matrix>(strikes.size(), dates.size()));

    for (Size i=0; i < strikes.size(); ++i) {
        for (Size j=0; j < dates.size(); ++j) {
            try {
                (*surface)[i][j] = localVol->localVol(dates[j], strikes[i], true);
            } catch (Error&) {
                (*surface)[i][j] = 0.2;
            }
        }
    }

    return surface;
}

ext::tuple<std::vector<Real>, std::vector<Date>,
           ext::shared_ptr<BlackVarianceSurface> >
createSmoothImpliedVol(const DayCounter& dc, const Calendar& cal) {

    const Date todaysDate = Settings::instance().evaluationDate();

    Integer times[] = { 13, 41, 75, 165, 256, 345, 524, 703 };
    std::vector<Date> dates;
    for (int time : times) {
        Date date = todaysDate + time;
        dates.push_back(date);
    }

    const std::vector<Real> surfaceStrikes = {
        2.222222222, 11.11111111, 44.44444444, 75.55555556, 80, 84.44444444, 88.88888889, 93.33333333, 97.77777778, 100,
        102.2222222, 106.6666667, 111.1111111, 115.5555556, 120, 124.4444444, 166.6666667, 222.2222222, 444.4444444, 666.6666667
    };

    Volatility v[] =
        { 1.015873, 1.015873, 0.915873, 0.89729, 0.796493, 0.730914, 0.631335, 0.568895,
          0.851309, 0.821309, 0.781309, 0.641309, 0.635593, 0.583653, 0.508045, 0.463182,
          0.686034, 0.630534, 0.590534, 0.500534, 0.448706, 0.416661, 0.375470, 0.353442,
          0.526034, 0.482263, 0.447713, 0.387703, 0.355064, 0.337438, 0.316966, 0.306859,
          0.497587, 0.464373, 0.430764, 0.374052, 0.344336, 0.328607, 0.310619, 0.301865,
          0.479511, 0.446815, 0.414194, 0.361010, 0.334204, 0.320301, 0.304664, 0.297180,
          0.461866, 0.429645, 0.398092, 0.348638, 0.324680, 0.312512, 0.299082, 0.292785,
          0.444801, 0.413014, 0.382634, 0.337026, 0.315788, 0.305239, 0.293855, 0.288660,
          0.428604, 0.397219, 0.368109, 0.326282, 0.307555, 0.298483, 0.288972, 0.284791,
          0.420971, 0.389782, 0.361317, 0.321274, 0.303697, 0.295302, 0.286655, 0.282948,
          0.413749, 0.382754, 0.354917, 0.316532, 0.300016, 0.292251, 0.284420, 0.281164,
          0.400889, 0.370272, 0.343525, 0.307904, 0.293204, 0.286549, 0.280189, 0.277767,
          0.390685, 0.360399, 0.334344, 0.300507, 0.287149, 0.281380, 0.276271, 0.274588,
          0.383477, 0.353434, 0.327580, 0.294408, 0.281867, 0.276746, 0.272655, 0.271617,
          0.379106, 0.349214, 0.323160, 0.289618, 0.277362, 0.272641, 0.269332, 0.268846,
          0.377073, 0.347258, 0.320776, 0.286077, 0.273617, 0.269057, 0.266293, 0.266265,
          0.399925, 0.369232, 0.338895, 0.289042, 0.265509, 0.255589, 0.249308, 0.249665,
          0.423432, 0.406891, 0.373720, 0.314667, 0.281009, 0.263281, 0.246451, 0.242166,
          0.453704, 0.453704, 0.453704, 0.381255, 0.334578, 0.305527, 0.268909, 0.251367,
          0.517748, 0.517748, 0.517748, 0.416577, 0.364770, 0.331595, 0.287423, 0.264285 };

    Matrix blackVolMatrix(surfaceStrikes.size(), dates.size());
    for (Size i=0; i < surfaceStrikes.size(); ++i)
        for (Size j=0; j < dates.size(); ++j) {
            blackVolMatrix[i][j] = v[i*(dates.size())+j];
        }

    const ext::shared_ptr<BlackVarianceSurface> volTS(
            ext::make_shared<BlackVarianceSurface>(todaysDate, cal,
                                                   dates,
                                                   surfaceStrikes, blackVolMatrix,
                                                   dc,
                                                   BlackVarianceSurface::ConstantExtrapolation,
                                                   BlackVarianceSurface::ConstantExtrapolation));
    volTS->setInterpolation<Bicubic>();

    return ext::make_tuple(surfaceStrikes, dates, volTS);
}

struct HestonModelParams {
    const Rate r, q;
    const Real kappa, theta, rho, sigma, v0;
};

struct HestonSLVTestCase {
    const HestonModelParams hestonParams;
    const HestonSLVFokkerPlanckFdmParams fdmParams;
};


//    void lsvCalibrationTest(const HestonSLVTestCase& testCase) {
//        const Date todaysDate(2, June, 2015);
//        Settings::instance().evaluationDate() = todaysDate;
//        const Date finalDate(2, June, 2020);
//
//        const DayCounter dc = Actual365Fixed();
//
//        const Real s0 = 100;
//        const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
//
//        const Rate r = testCase.hestonParams.r;
//        const Rate q = testCase.hestonParams.q;
//
//        const Real kappa = testCase.hestonParams.kappa;
//        const Real theta = testCase.hestonParams.theta;
//        const Real rho   = testCase.hestonParams.rho;
//        const Real sigma = testCase.hestonParams.sigma;
//        const Real v0    = testCase.hestonParams.v0;
//        const Volatility lv = 0.3;
//
//        const Handle<YieldTermStructure> rTS(flatRate(r, dc));
//        const Handle<YieldTermStructure> qTS(flatRate(q, dc));
//
//        const ext::shared_ptr<HestonProcess> hestonProcess(
//            ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma, rho));
//
//        const Handle<HestonModel> hestonModel(
//            ext::make_shared<HestonModel>(hestonProcess));
//
//        const Handle<LocalVolTermStructure> localVol(
//            ext::make_shared<LocalConstantVol>(todaysDate, lv, dc));
//
//        const HestonSLVFDMModel slvModel(
//            localVol, hestonModel, finalDate, testCase.fdmParams);
//
//        // this includes a calibration of the leverage function!
//        ext::shared_ptr<LocalVolTermStructure>
//            l = slvModel.leverageFunction();
//
//        const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
//            ext::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS,
//                                                             Handle<BlackVolTermStructure>(flatVol(lv, dc))));
//
//        const ext::shared_ptr<PricingEngine> analyticEngine(
//            ext::make_shared<AnalyticEuropeanEngine>(bsProcess));
//
//        const Real strikes[] = { 50, 75, 80, 90, 100, 110, 125, 150 };
//        const Size times[] = { 3, 6, 9, 12, 24, 36, 60 };
//
//        for (unsigned long time : times) {
//            const Date expiry = todaysDate + Period(time, Months);
//            const ext::shared_ptr<Exercise> exercise(
//                ext::make_shared<EuropeanExercise>(expiry));
//
//            const ext::shared_ptr<PricingEngine> slvEngine(
//                (time <= 3) ?
//                    ext::make_shared<FdHestonVanillaEngine>(
//                        hestonModel.currentLink(), Size(std::max(101.0, 51 * time / 12.0)), 401,
//                        101, 0, FdmSchemeDesc::ModifiedCraigSneyd(), l) :
//                    ext::make_shared<FdHestonVanillaEngine>(
//                        hestonModel.currentLink(), Size(std::max(51.0, 51 * time / 12.0)), 201, 101,
//                        0, FdmSchemeDesc::ModifiedCraigSneyd(), l));
//
//            for (Real strike : strikes) {
//                const ext::shared_ptr<StrikedTypePayoff> payoff(
//                    ext::make_shared<PlainVanillaPayoff>((strike > s0) ? Option::Call : Option::Put,
//                                                         strike));
//
//                VanillaOption option(payoff, exercise);
//
//                option.setPricingEngine(slvEngine);
//                const Real calculated = option.NPV();
//
//                option.setPricingEngine(analyticEngine);
//                const Real expected = option.NPV();
//                const Real vega = option.vega();
//
//                const ext::shared_ptr<GeneralizedBlackScholesProcess> bp(
//                    ext::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS,
//                                                                     Handle<BlackVolTermStructure>(flatVol(lv,
//                                                                                                           dc))));
//
//                const Real tol = 0.001;//testCase.eps;
//                if (std::fabs((calculated-expected)/vega) > tol) {
//                    BOOST_FAIL("failed to reproduce round trip vola "
//                               << "\n   strike         " << strike << "\n   time           " << time
//                               << "\n   expected NPV   " << expected << "\n   calculated NPV "
//                               << calculated << "\n   vega           " << vega << std::fixed
//                               << std::setprecision(5)
//                               << "\n   calculated:    " << lv + (calculated - expected) / vega
//                               << "\n   expected:      " << lv << "\n   diff  (in bp)  "
//                               << (calculated - expected) / vega * 1e4
//                               << "\n   tolerance:     " << tol);
//                }
//            }
//        }
//    }

ext::shared_ptr<LocalVolTermStructure> getFixedLocalVolFromHeston(
        const ext::shared_ptr<HestonModel>& hestonModel,
        const ext::shared_ptr<TimeGrid>& timeGrid) {

    const Handle<BlackVolTermStructure> trueImpliedVolSurf(
            ext::make_shared<HestonBlackVolSurface>(
                Handle<HestonModel>(hestonModel),
                AnalyticHestonEngine::AndersenPiterbarg,
                AnalyticHestonEngine::Integration::gaussLaguerre(32)));

    const ext::shared_ptr<HestonProcess> hestonProcess
        = hestonModel->process();

    const ext::shared_ptr<LocalVolTermStructure> localVol(
            ext::make_shared<NoExceptLocalVolSurface>(
                trueImpliedVolSurf,
                hestonProcess->riskFreeRate(),
                hestonProcess->dividendYield(),
                hestonProcess->s0(),
                std::sqrt(hestonProcess->theta())));


    const ext::shared_ptr<LocalVolRNDCalculator> localVolRND(
            ext::make_shared<LocalVolRNDCalculator>(
                hestonProcess->s0().currentLink(),
                hestonProcess->riskFreeRate().currentLink(),
                hestonProcess->dividendYield().currentLink(),
                localVol,
                timeGrid));

    std::vector<ext::shared_ptr<std::vector<Real> > > strikes;
    for (Size i=1; i < timeGrid->size(); ++i) {
        const Time t = timeGrid->at(i);
        const ext::shared_ptr<Fdm1dMesher> fdm1dMesher
            = localVolRND->mesher(t);

        const std::vector<Real>& logStrikes = fdm1dMesher->locations();
        const ext::shared_ptr<std::vector<Real> > strikeSlice(
                ext::make_shared<std::vector<Real> >(logStrikes.size()));

        for (Size j=0; j < logStrikes.size(); ++j) {
            (*strikeSlice)[j] = std::exp(logStrikes[j]);
        }

        strikes.push_back(strikeSlice);
    }

    const Size nStrikes = strikes.front()->size();
    const ext::shared_ptr<Matrix> localVolMatrix(
                                                 ext::make_shared<Matrix>(nStrikes, timeGrid->size()-1));
    for (Size i=1; i < timeGrid->size(); ++i) {
        const Time t = timeGrid->at(i);
        const ext::shared_ptr<std::vector<Real> > strikeSlice = strikes[i-1];

        for (Size j=0; j < nStrikes; ++j) {
            const Real s = (*strikeSlice)[j];
            (*localVolMatrix)[j][i-1] = localVol->localVol(t, s, true);
        }
    }

    const Date todaysDate
        = hestonProcess->riskFreeRate()->referenceDate();
    const DayCounter dc = hestonProcess->riskFreeRate()->dayCounter();

    const std::vector<Time> expiries(
            timeGrid->begin()+1, timeGrid->end());

    return ext::make_shared<FixedLocalVolSurface>(todaysDate, expiries, strikes, localVolMatrix, dc);
}


BOOST_AUTO_TEST_CASE(testBlackScholesFokkerPlanckFwdEquation) {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation for BS process...");

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturityDate = todaysDate + Period(2, Years);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.035;
    const Rate q = 0.01;
    const Volatility v = 0.35;

    const Size xGrid = 2*100+1;
    const Size tGrid = 400;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<BlackVolTermStructure> vTS(flatVol(v, dc));

    const ext::shared_ptr<GeneralizedBlackScholesProcess> process(
		ext::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS, vTS));

    const ext::shared_ptr<PricingEngine> engine(
		ext::make_shared<AnalyticEuropeanEngine>(process));

    const ext::shared_ptr<FdmMesher> uniformMesher(
		ext::make_shared<FdmMesherComposite>(
			ext::make_shared<FdmBlackScholesMesher>(xGrid, process, maturity, s0)));

    const ext::shared_ptr<FdmLinearOpComposite> uniformBSFwdOp(
		ext::make_shared<FdmBlackScholesFwdOp>(uniformMesher, process, s0, false));

    const ext::shared_ptr<FdmMesher> concentratedMesher(
		ext::make_shared<FdmMesherComposite>(
			ext::make_shared<FdmBlackScholesMesher>(xGrid, process, maturity, s0,
                                      Null<Real>(), Null<Real>(), 0.0001, 1.5,
                                      std::pair<Real, Real>(s0, 0.1))));

    const ext::shared_ptr<FdmLinearOpComposite> concentratedBSFwdOp(
		ext::make_shared<FdmBlackScholesFwdOp>(concentratedMesher, process, s0, false));

    const ext::shared_ptr<FdmMesher> shiftedMesher(
		ext::make_shared<FdmMesherComposite>(
			ext::make_shared<FdmBlackScholesMesher>(xGrid, process, maturity, s0,
                                      Null<Real>(), Null<Real>(), 0.0001, 1.5,
                                      std::pair<Real, Real>(s0*1.1, 0.2))));

    const ext::shared_ptr<FdmLinearOpComposite> shiftedBSFwdOp(
		ext::make_shared<FdmBlackScholesFwdOp>(shiftedMesher, process, s0, false));

    const ext::shared_ptr<Exercise> exercise(
		ext::make_shared<EuropeanExercise>(maturityDate));
    const Real strikes[] = { 50, 80, 100, 130, 150 };

    for (Real strike : strikes) {
        const ext::shared_ptr<StrikedTypePayoff> payoff(
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike));

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(engine);

        const Real expected = option.NPV()/rTS->discount(maturityDate);
        const Real calcUniform
            = fokkerPlanckPrice1D(uniformMesher, uniformBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real calcConcentrated
            = fokkerPlanckPrice1D(concentratedMesher, concentratedBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real calcShifted
            = fokkerPlanckPrice1D(shiftedMesher, shiftedBSFwdOp,
                                  payoff, x0, maturity, tGrid);
        const Real tol = 0.02;

        if (std::fabs(expected - calcUniform) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with an uniform mesher"
                       << "\n   strike:     " << strike << std::fixed << std::setprecision(8)
                       << "\n   calculated: " << calcUniform << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
        if (std::fabs(expected - calcConcentrated) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with a concentrated mesher"
                       << "\n   strike:     " << strike << std::fixed << std::setprecision(8)
                       << "\n   calculated: " << calcConcentrated << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
        if (std::fabs(expected - calcShifted) > tol) {
            BOOST_FAIL("failed to reproduce european option price "
                       << "with a shifted mesher"
                       << "\n   strike:     " << strike << std::fixed << std::setprecision(8)
                       << "\n   calculated: " << calcShifted << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testSquareRootZeroFlowBC) {
    BOOST_TEST_MESSAGE("Testing zero-flow BC for the square root process...");

    const Real kappa = 1.0;
    const Real theta = 0.4;
    const Real sigma = 0.8;
    const Real v_0   = 0.1;
    const Time t     = 1.0;

    const Real vmin = 0.0005;
    const Real h    = 0.0001;

    const Real expected[5][5]
        = {{ 0.000548, -0.000245, -0.005657, -0.001167, -0.000024},
           {-0.000595, -0.000701, -0.003296, -0.000883, -0.000691},
           {-0.001277, -0.001320, -0.003128, -0.001399, -0.001318},
           {-0.001979, -0.002002, -0.003425, -0.002047, -0.002001},
           {-0.002715, -0.002730, -0.003920, -0.002760, -0.002730} };

    for (Size i=0; i < 5; ++i) {
        const Real v = vmin + i*0.001;
        const Real vm2 = v - 2*h;
        const Real vm1 = v - h;
        const Real v0  = v;
        const Real v1  = v + h;
        const Real v2  = v + 2*h;

        const SquareRootProcessRNDCalculator rndCalculator(
            v_0, kappa, theta, sigma);

        const Real pm2 = rndCalculator.pdf(vm2, t);
        const Real pm1 = rndCalculator.pdf(vm1, t);
        const Real p0  = rndCalculator.pdf(v0 , t);
        const Real p1  = rndCalculator.pdf(v1 , t);
        const Real p2  = rndCalculator.pdf(v2 , t);

        // test derivatives
        const Real flowSym2Order = sigma*sigma*v0/(4*h)*(p1-pm1)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real flowSym4Order
            = sigma*sigma*v0/(24*h)*(-p2 + 8*p1 - 8*pm1 + pm2)
              + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd1Order = sigma*sigma*v0/(2*h)*(p1-p0)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd2Order = sigma*sigma*v0/(4*h)*(4*p1-3*p0-p2)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real fwd3Order
            = sigma*sigma*v0/(12*h)*(-p2 + 6*p1 - 3*p0 - 2*pm1)
                                + (kappa*(v0-theta)+sigma*sigma/2)*p0;

        const Real tol = 0.000002;
        if (   std::fabs(expected[i][0] - flowSym2Order) > tol
            || std::fabs(expected[i][1] - flowSym4Order) > tol
            || std::fabs(expected[i][2] - fwd1Order) > tol
            || std::fabs(expected[i][3] - fwd2Order) > tol
            || std::fabs(expected[i][4] - fwd3Order) > tol ) {
            BOOST_ERROR("failed to reproduce Zero Flow BC at"
                       << "\n   v:          " << v
                       << "\n   tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testTransformedZeroFlowBC) {
    BOOST_TEST_MESSAGE("Testing zero-flow BC for transformed "
                       "Fokker-Planck forward equation...");

    const Real kappa = 1.0;
    const Real theta = 0.4;
    const Real sigma = 2.0;
    const Size vGrid = 100;

    const ext::shared_ptr<FdmMesher> mesher
        = createStationaryDistributionMesher(kappa, theta, sigma, vGrid);
    const Array v = mesher->locations(0);

    Array p(vGrid);
    const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);
    for (Size i=0; i < v.size(); ++i)
        p[i] =  rnd.stationary_pdf(v[i]);


    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);
    const Array q = Pow(v, alpha)*p;

    for (Size i=0; i < vGrid/2; ++i) {
        const Real hm = v[i+1] - v[i];
        const Real hp = v[i+2] - v[i+1];

        const Real eta=1.0/(hm*(hm+hp)*hp);
        const Real a = -eta*(squared(hm+hp) - hm*hm);
        const Real b  = eta*squared(hm+hp);
        const Real c = -eta*hm*hm;

        const Real df = a*q[i] + b*q[i+1] + c*q[i+2];
        const Real flow = 0.5*sigma*sigma*v[i]*df + kappa*v[i]*q[i];

        const Real tol = 1e-6;
        if (std::fabs(flow) > tol) {
            BOOST_ERROR("failed to reproduce Zero Flow BC at"
                       << "\n v:          " << v
                       << "\n flow:       " << flow
                       << "\n tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testSquareRootEvolveWithStationaryDensity) {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root process with stationary density...");

    // Documentation for this test case:
    // http://www.spanderen.de/2013/05/04/fokker-planck-equation-feller-constraint-and-boundary-conditions/
    const Real kappa = 2.5;
    const Real theta = 0.2;
    const Size vGrid = 100;
    const Real eps = 1e-2;

    for (Real sigma = 0.2; sigma < 2.01; sigma+=0.1) {
        const Real alpha = (1.0 - 2*kappa*theta/(sigma*sigma));

        const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);
        const Real vMin = rnd.stationary_invcdf(eps);
        const Real vMax = rnd.stationary_invcdf(1-eps);

        const ext::shared_ptr<FdmMesher> mesher(
			ext::make_shared<FdmMesherComposite>(
				ext::make_shared<Uniform1dMesher>(vMin, vMax, vGrid)));

        const Array v = mesher->locations(0);
        const FdmSquareRootFwdOp::TransformationType transform =
            (sigma < 0.75) ?
               FdmSquareRootFwdOp::Plain :
               FdmSquareRootFwdOp::Power;

        Array vq (v.size());
        Array vmq(v.size());
        for (Size i=0; i < v.size(); ++i) {
            vmq[i] = 1.0/(vq[i] = std::pow(v[i], alpha));
        }

        Array p(vGrid);
        for (Size i=0; i < v.size(); ++i) {
            p[i] =  rnd.stationary_pdf(v[i]);
            if (transform == FdmSquareRootFwdOp::Power)
                p[i] *= vq[i];
        }

        const ext::shared_ptr<FdmSquareRootFwdOp> op(
			ext::make_shared<FdmSquareRootFwdOp>(mesher, kappa, theta,
                                   sigma, 0, transform));

        const Size n = 100;
        const Time dt = 0.01;
        DouglasScheme evolver(0.5, op);
        evolver.setStep(dt);

        for (Size i=1; i <= n; ++i) {
            evolver.step(p, i*dt);
        }

        const Real expected = 1-2*eps;

        if (transform == FdmSquareRootFwdOp::Power)
            for (Size i=0; i < v.size(); ++i) {
                p[i] *= vmq[i];
            }

        const q_fct f(v, p, alpha);

        const Real calculated = GaussLobattoIntegral(1000000, 1e-6)(
                                        f, v.front(), v.back());

        const Real tol = 0.005;
        if (std::fabs(calculated-expected) > tol) {
            BOOST_ERROR("failed to reproduce stationary probability function"
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testSquareRootLogEvolveWithStationaryDensity) {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root log process with stationary density...");

    // Documentation for this test case:
    // nowhere yet :)
    const Real kappa = 2.5;
    const Real theta = 0.2;
    const Size vGrid = 1000;
    const Real eps = 1e-2;

    for (Real sigma = 0.2; sigma < 2.01; sigma+=0.1) {
        const Real lowerLimit = 0.001;
        // should not go to very large negative values, distributions flattens with sigma
        // causing numerical instabilities log/exp evaluations

        const SquareRootProcessRNDCalculator rnd(theta, kappa, theta, sigma);

        const Real vMin = std::max(lowerLimit, rnd.stationary_invcdf(eps));
        const Real lowEps = std::max(eps, rnd.stationary_cdf(lowerLimit));

        const Real expected = 1-eps-lowEps;
        const Real vMax = rnd.stationary_invcdf(1-eps);

        const ext::shared_ptr<FdmMesherComposite> mesher(
			ext::make_shared<FdmMesherComposite>(
				ext::make_shared<Uniform1dMesher>(std::log(vMin), std::log(vMax), vGrid)));

        const Array v = mesher->locations(0);

        Array p(vGrid);
        for (Size i=0; i < v.size(); ++i)
            p[i] =  stationaryLogProbabilityFct(kappa, theta, sigma, v[i]);

        const ext::shared_ptr<FdmSquareRootFwdOp> op(
			ext::make_shared<FdmSquareRootFwdOp>(mesher, kappa, theta,
                                   sigma, 0,
                                   FdmSquareRootFwdOp::Log));

        const Size n = 100;
        const Time dt = 0.01;
        FdmBoundaryConditionSet bcSet;
        DouglasScheme evolver(0.5, op);
        evolver.setStep(dt);

        for (Size i=1; i <= n; ++i) {
            evolver.step(p, i*dt);
        }

        const Real calculated
            = FdmMesherIntegral(mesher, DiscreteSimpsonIntegral()).integrate(p);

        const Real tol = 0.005;
        if (std::fabs(calculated-expected) > tol) {
            BOOST_ERROR("failed to reproduce stationary probability function for "
                    << "\n    sigma:      " << sigma
                    << "\n    calculated: " << calculated
                    << "\n    expected:   " << expected
                    << "\n    tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testSquareRootFokkerPlanckFwdEquation) {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the square root process with Dirac start...");

    const Real kappa = 1.2;
    const Real theta = 0.4;
    const Real sigma = 0.7;
    const Real v0 = theta;
    const Real alpha = 1.0 - 2*kappa*theta/(sigma*sigma);

    const Time maturity = 1.0;

    const Size xGrid = 1001;
    const Size tGrid = 500;

    const Real vol = sigma*std::sqrt(theta/(2*kappa));
    const Real upperBound = theta+6*vol;
    const Real lowerBound = std::max(0.0002, theta-6*vol);

    const ext::shared_ptr<FdmMesher> mesher(
		ext::make_shared<FdmMesherComposite>(
			ext::make_shared<Uniform1dMesher>(lowerBound, upperBound, xGrid)));

    const Array x(mesher->locations(0));

    const ext::shared_ptr<FdmSquareRootFwdOp> op(
		ext::make_shared<FdmSquareRootFwdOp>(mesher, kappa, theta, sigma, 0)); //!

    const Time dt = maturity/tGrid;
    const Size n = 5;

    Array p(xGrid);
    SquareRootProcessRNDCalculator rndCalculator(v0, kappa, theta, sigma);
    for (Size i=0; i < p.size(); ++i) {
        p[i] = rndCalculator.pdf(x[i], n*dt);
    }
    Array q = Pow(x, alpha)*p;

    DouglasScheme evolver(0.5, op);
    evolver.setStep(dt);

    for (Time t=(n+1)*dt; t <= maturity+20*QL_EPSILON; t+=dt) {
        evolver.step(p, t);
        evolver.step(q, t);
    }

    const Real tol = 0.002;

    Array y(x.size());
    for (Size i=0; i < x.size(); ++i) {
        const Real expected = rndCalculator.pdf(x[i], maturity);

        const Real calculated = p[i];
        if (std::fabs(expected - calculated) > tol) {
            BOOST_FAIL("failed to reproduce pdf at"
                       << std::fixed << std::setprecision(5)
                       << "\n   x:          " << x[i]
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testHestonFokkerPlanckFwdEquation, *precondition(if_speed(Slow))) {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the Heston process...");

    FokkerPlanckFwdTestCase testCases[] = {
        {
            100.0, 0.01, 0.02,
            // Feller constraint violated, high vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.2 = 0.5 < 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.2),
            101, 401, 25, 25,
            0.02, 0.05,
            FdmSquareRootFwdOp::Power,
            FdmHestonGreensFct::Gaussian,
            FdmSchemeDesc::DouglasType
        },
        {
            100.0, 0.01, 0.02,
            // Feller constraint violated, high vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.2 = 0.5 < 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.2),
            201, 501, 10, 10,
            0.005, 0.02,
            FdmSquareRootFwdOp::Log,
            FdmHestonGreensFct::Gaussian,
            FdmSchemeDesc::HundsdorferType
        },
        {
            100.0, 0.01, 0.02,
            // Feller constraint violated, high vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.2 = 0.5 < 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.2),
            201, 501, 25, 25,
            0.01, 0.03,
            FdmSquareRootFwdOp::Log,
            FdmHestonGreensFct::ZeroCorrelation,
            FdmSchemeDesc::HundsdorferType
        },
        {
            100.0, 0.01, 0.02,
            // Feller constraint fulfilled, low vol-of-vol case
            // \frac{2\kappa\theta}{\sigma^2} = 2.0 * 1.0 * 0.05 / 0.05 = 2.0 > 1
            0.05, 1.0, 0.05, -0.75, std::sqrt(0.05),
            201, 401, 5, 5,
            0.01, 0.02,
            FdmSquareRootFwdOp::Plain,
            FdmHestonGreensFct::Gaussian,
            FdmSchemeDesc::HundsdorferType
        }
    };

    for (const auto& testCase : testCases) {
        hestonFokkerPlanckFwdEquationTest(testCase);
    }
}

BOOST_AUTO_TEST_CASE(testHestonFokkerPlanckFwdEquationLogLVLeverage, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE("Testing Fokker-Planck forward equation "
                       "for the Heston process Log Transformation with leverage LV limiting case...");

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate = Date(28, Dec, 2012);
    Settings::instance().evaluationDate() = todaysDate;

    const Date maturityDate = todaysDate + Period(1, Years);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.0;
    const Rate q = 0.0;

    const Real kappa =  1.0;
    const Real theta =  1.0;
    const Real rho   = -0.75;
    const Real sigma =  0.02;
    const Real v0    =  theta;

    const FdmSquareRootFwdOp::TransformationType transform
        = FdmSquareRootFwdOp::Plain;

    const DayCounter dayCounter = Actual365Fixed();
    const Calendar calendar = TARGET();

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Handle<YieldTermStructure> rTS(flatRate(todaysDate, r, dayCounter));
    const Handle<YieldTermStructure> qTS(flatRate(todaysDate, q, dayCounter));

    ext::shared_ptr<HestonProcess> hestonProcess(
		ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Size xGrid = 201;
    const Size vGrid = 401;
    const Size tGrid = 25;

    const SquareRootProcessRNDCalculator rnd(v0, kappa, theta, sigma);

    const Real upperBound = rnd.stationary_invcdf(0.99);
    const Real lowerBound = rnd.stationary_invcdf(0.01);

    const Real beta = 10.0;
    std::vector<ext::tuple<Real, Real, bool>> critPoints = {
        ext::make_tuple(lowerBound, beta, true),
        ext::make_tuple(v0, beta/100, true),
        ext::make_tuple(upperBound, beta, true)
    };
    const ext::shared_ptr<Fdm1dMesher> varianceMesher(
		ext::make_shared<Concentrating1dMesher>(lowerBound, upperBound, vGrid, critPoints));

    const ext::shared_ptr<Fdm1dMesher> equityMesher(
		ext::make_shared<Concentrating1dMesher>(std::log(2.0), std::log(600.0), xGrid,
            std::make_pair(x0+0.005, 0.1), true));

    const ext::shared_ptr<FdmMesherComposite>
        mesher(ext::make_shared<FdmMesherComposite>(equityMesher, varianceMesher));

    const ext::tuple<std::vector<Real>, std::vector<Date>,
                 ext::shared_ptr<BlackVarianceSurface> > smoothSurface =
        createSmoothImpliedVol(dayCounter, calendar);
    const ext::shared_ptr<BlackScholesMertonProcess> lvProcess(
		ext::make_shared<BlackScholesMertonProcess>(spot, qTS, rTS,
        Handle<BlackVolTermStructure>(ext::get<2>(smoothSurface))));

    // step two days using non-correlated process
    const Time eT = 2.0/365;

    Real v=-Null<Real>(), p_v(0.0);
    Array p(mesher->layout()->size(), 0.0);
    const Real bsV0 = squared(lvProcess->blackVolatility()->blackVol(0.0, s0, true));

    SquareRootProcessRNDCalculator rndCalculator(v0, kappa, theta, sigma);
    for (const auto& iter : *mesher->layout()) {
        const Real x = mesher->location(iter, 0);
        if (v != mesher->location(iter, 1)) {
            v = mesher->location(iter, 1);
            // the extreme tail probabilities of the non central
            // chi square distribution lead to numerical exceptions
            // on some platforms
            if (std::fabs(v - v0) < 5*sigma*std::sqrt(v0*eT))
                p_v = rndCalculator.pdf(v, eT);
            else
                p_v = 0.0;
        }
        const Real p_x = 1.0/(std::sqrt(M_TWOPI*bsV0*eT))
            * std::exp(-0.5*squared(x - x0)/(bsV0*eT));
        p[iter.index()] = p_v*p_x;
    }
    const Time dt = (maturity-eT)/tGrid;


    const std::vector<Real> denseStrikes =
        { 2.222222222, 11.11111111, 20, 25, 30, 35, 40,
          44.44444444, 50, 55, 60, 65, 70, 75.55555556,
          80, 84.44444444, 88.88888889, 93.33333333, 97.77777778, 100,
          102.2222222, 106.6666667, 111.1111111, 115.5555556, 120,
          124.4444444, 166.6666667, 222.2222222, 444.4444444, 666.6666667 };

    Matrix surface(denseStrikes.size(), ext::get<1>(smoothSurface).size());
    std::vector<Time> times(surface.columns());

    const std::vector<Date>& dates = ext::get<1>(smoothSurface);
    ext::shared_ptr<Matrix> m = createLocalVolMatrixFromProcess(
        lvProcess, denseStrikes, dates, times);

    const ext::shared_ptr<FixedLocalVolSurface> leverage(
		ext::make_shared<FixedLocalVolSurface>(todaysDate, dates, denseStrikes, m, dc));

    const ext::shared_ptr<PricingEngine> lvEngine(
		ext::make_shared<AnalyticEuropeanEngine>(lvProcess));

    const ext::shared_ptr<FdmLinearOpComposite> hestonFwdOp(
		ext::make_shared<FdmHestonFwdOp>(mesher, hestonProcess, transform, leverage));

    HundsdorferScheme evolver(FdmSchemeDesc::Hundsdorfer().theta,
                              FdmSchemeDesc::Hundsdorfer().mu,
                              hestonFwdOp);

    Time t=dt;
    evolver.setStep(dt);

    for (Size i=0; i < tGrid; ++i, t+=dt) {
        evolver.step(p, t);
    }

    const ext::shared_ptr<Exercise> exercise(
		ext::make_shared<EuropeanExercise>(maturityDate));

    const ext::shared_ptr<PricingEngine> fdmEngine(
		ext::make_shared<FdBlackScholesVanillaEngine>(lvProcess, 50, 201, 0,
                                        FdmSchemeDesc::Douglas(), true,0.2));

    for (Size strike=5; strike < 200; strike+=10) {
        const ext::shared_ptr<StrikedTypePayoff> payoff(
			ext::make_shared<CashOrNothingPayoff>(Option::Put, Real(strike), 1.0));

        Array pd(p.size());
        for (const auto& iter : *mesher->layout()) {
            const Size idx = iter.index();
            const Real s = std::exp(mesher->location(iter, 0));

            pd[idx] = (*payoff)(s)*p[idx];
        }

        const Real calculated
            = fokkerPlanckPrice2D(pd, mesher)*rTS->discount(maturityDate);

        VanillaOption option(payoff, exercise);
        option.setPricingEngine(fdmEngine);
        const Real expected = option.NPV();

        const Real tol = 0.015;
        if (std::fabs(expected - calculated ) > tol) {
            BOOST_FAIL("failed to reproduce Heston prices at"
                       << "\n   strike      " << strike
                       << std::fixed << std::setprecision(5)
                       << "\n   calculated: " << calculated
                       << "\n   expected:   " << expected
                       << "\n   tolerance:  " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testBlackScholesFokkerPlanckFwdEquationLocalVol, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE(
            "Testing Fokker-Planck forward equation for BS Local Vol process...");

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate(5, July, 2014);
    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Real x0 = std::log(s0);
    const Rate r = 0.035;
    const Rate q = 0.01;

    const Calendar calendar = TARGET();
    const DayCounter dayCounter = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
            flatRate(todaysDate, r, dayCounter));
    const Handle<YieldTermStructure> qTS(
            flatRate(todaysDate, q, dayCounter));

    ext::tuple<std::vector<Real>, std::vector<Date>,
            ext::shared_ptr<BlackVarianceSurface> > smoothImpliedVol =
            createSmoothImpliedVol(dayCounter, calendar);

    const std::vector<Real>& strikes = ext::get<0>(smoothImpliedVol);
    const std::vector<Date>& dates = ext::get<1>(smoothImpliedVol);
    const Handle<BlackVolTermStructure> vTS =
        Handle<BlackVolTermStructure>(ext::get<2>(smoothImpliedVol));

    const Size xGrid = 101;
    const Size tGrid = 51;

    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const ext::shared_ptr<BlackScholesMertonProcess> process(
		ext::make_shared<BlackScholesMertonProcess>(spot, qTS, rTS, vTS));

    const ext::shared_ptr<LocalVolTermStructure> localVol(
        ext::make_shared<NoExceptLocalVolSurface>(vTS, rTS, qTS, spot, 0.2));

    const ext::shared_ptr<PricingEngine> engine(
		ext::make_shared<AnalyticEuropeanEngine>(process));

    for (Size i = 1; i < dates.size(); i += 2) {
        for (Size j = 3; j < strikes.size() - 3; j += 2) {
            const Date& exDate = dates[i];
            const Date maturityDate = exDate;
            const Time maturity = dc.yearFraction(todaysDate, maturityDate);
            const ext::shared_ptr<Exercise> exercise(
				ext::make_shared<EuropeanExercise>(exDate));

            const ext::shared_ptr<FdmMesher> uniformMesher(
				ext::make_shared<FdmMesherComposite>(
					ext::make_shared<FdmBlackScholesMesher>(xGrid, process,
                            maturity, s0)));

            //-- operator --
            const ext::shared_ptr<FdmLinearOpComposite> uniformBSFwdOp(
				ext::make_shared<FdmLocalVolFwdOp>(
                    uniformMesher, *spot, *rTS, *qTS, localVol));

            const ext::shared_ptr<FdmMesher> concentratedMesher(
				ext::make_shared<FdmMesherComposite>(
					ext::make_shared<FdmBlackScholesMesher>(xGrid, process,
                                maturity, s0, Null<Real>(),
                                Null<Real>(), 0.0001, 1.5,
                                std::pair<Real, Real>(s0, 0.1))));

            //-- operator --
            const ext::shared_ptr<FdmLinearOpComposite> concentratedBSFwdOp(
				ext::make_shared<FdmLocalVolFwdOp>(
                    concentratedMesher, *spot, *rTS, *qTS, localVol));

            const ext::shared_ptr<FdmMesher> shiftedMesher(
				ext::make_shared<FdmMesherComposite>(
					ext::make_shared<FdmBlackScholesMesher>(xGrid, process,
                            maturity, s0, Null<Real>(),
                            Null<Real>(), 0.0001, 1.5,
                            std::pair<Real, Real>(s0 * 1.1,
                                    0.2))));

            //-- operator --
            const ext::shared_ptr<FdmLinearOpComposite> shiftedBSFwdOp(
				ext::make_shared<FdmLocalVolFwdOp>(
                    shiftedMesher, *spot, *rTS, *qTS, localVol));

            const ext::shared_ptr<StrikedTypePayoff> payoff(
				ext::make_shared<PlainVanillaPayoff>(Option::Call, strikes[j]));

            VanillaOption option(payoff, exercise);
            option.setPricingEngine(engine);

            const Real expected = option.NPV();
            const Real calcUniform = fokkerPlanckPrice1D(uniformMesher,
                    uniformBSFwdOp, payoff, x0, maturity, tGrid)
                    * rTS->discount(maturityDate);
            const Real calcConcentrated = fokkerPlanckPrice1D(
                    concentratedMesher, concentratedBSFwdOp, payoff, x0,
                    maturity, tGrid) * rTS->discount(maturityDate);
            const Real calcShifted = fokkerPlanckPrice1D(shiftedMesher,
                    shiftedBSFwdOp, payoff, x0, maturity, tGrid)
                    * rTS->discount(maturityDate);
            const Real tol = 0.05;

            if (std::fabs(expected - calcUniform) > tol) {
                BOOST_FAIL("failed to reproduce european option price "
                           << "with an uniform mesher"
                           << "\n   strike:     " << strikes[i]
                           << std::fixed << std::setprecision(8)
                           << "\n   calculated: " << calcUniform
                           << "\n   expected:   " << expected
                           << "\n   tolerance:  " << tol);
            }
            if (std::fabs(expected - calcConcentrated) > tol) {
                BOOST_FAIL("failed to reproduce european option price "
                           << "with a concentrated mesher"
                           << "\n   strike:     " << strikes[i]
                           << std::fixed << std::setprecision(8)
                           << "\n   calculated: " << calcConcentrated
                           << "\n   expected:   " << expected
                           << "\n   tolerance:  " << tol);
            }
            if (std::fabs(expected - calcShifted) > tol) {
                BOOST_FAIL("failed to reproduce european option price "
                           << "with a shifted mesher"
                           << "\n   strike:     " << strikes[i]
                           << std::fixed << std::setprecision(8)
                           << "\n   calculated: " << calcShifted
                           << "\n   expected:   " << expected
                           << "\n   tolerance:  " << tol);
            }
        }
    }
}

//BOOST_AUTO_TEST_CASE(testFDMCalibration) {
//    const HestonSLVFokkerPlanckFdmParams plainParams =
//        { 201, 301, 1000, 25, 3.0, 0, 2,
//          0.1, 1e-4, 10000,
//          1e-8, 1e-8, 0.0, 1.0, 1.0, 1.0, 1e-6,
//          FdmHestonGreensFct::Gaussian,
//          FdmSquareRootFwdOp::Plain,
//          FdmSchemeDesc::ModifiedCraigSneyd()
//        };
//
//    const HestonSLVFokkerPlanckFdmParams logParams =
//        { 301, 601, 2000, 30, 2.0, 0, 2,
//          0.1, 1e-4, 10000,
//          1e-5, 1e-5, 0.0000025, 1.0, 0.1, 0.9, 1e-5,
//          FdmHestonGreensFct::Gaussian,
//          FdmSquareRootFwdOp::Log,
//          FdmSchemeDesc::ModifiedCraigSneyd()
//        };
//
//    const HestonSLVFokkerPlanckFdmParams powerParams =
//        { 401, 801, 2000, 30, 2.0, 0, 2,
//          0.1, 1e-3, 10000,
//          1e-6, 1e-6, 0.001, 1.0, 0.001, 1.0, 1e-5,
//          FdmHestonGreensFct::Gaussian,
//          FdmSquareRootFwdOp::Power,
//          FdmSchemeDesc::ModifiedCraigSneyd()
//        };
//
//
//    const HestonSLVTestCase testCases[] = {
//        { {0.035, 0.01, 1.0, 0.06, -0.75, 0.1, 0.09}, plainParams},
//        { {0.035, 0.01, 1.0, 0.06, -0.75, std::sqrt(0.2), 0.09}, logParams },
//        { {0.035, 0.01, 1.0, 0.09, -0.75, std::sqrt(0.2), 0.06}, logParams },
//        { {0.035, 0.01, 1.0, 0.06, -0.75, 0.2, 0.09}, powerParams }
//    };
//
//
//    for (Size i=0; i < LENGTH(testCases); ++i) {
//        BOOST_TEST_MESSAGE("Testing stochastic local volatility calibration case " << i << " ...");
//        lsvCalibrationTest(testCases[i]);
//    }
//
//}

BOOST_AUTO_TEST_CASE(testLocalVolsvSLVPropDensity) {
    BOOST_TEST_MESSAGE("Testing local volatility vs SLV model...");

    const Date todaysDate(5, Oct, 2015);
    const Date finalDate = todaysDate + Period(1, Years);
    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Rate r = 0.01;
    const Rate q = 0.02;

    const Calendar calendar = TARGET();
    const DayCounter dayCounter = Actual365Fixed();

    const Handle<YieldTermStructure> rTS(
        flatRate(todaysDate, r, dayCounter));
    const Handle<YieldTermStructure> qTS(
        flatRate(todaysDate, q, dayCounter));

    const Handle<BlackVolTermStructure> vTS = Handle<BlackVolTermStructure>(
        ext::get<2>(createSmoothImpliedVol(dayCounter, calendar)));

    // Heston parameter from implied calibration
    const Real kappa =  2.0;
    const Real theta =  0.074;
    const Real rho   = -0.51;
    const Real sigma =  0.8;
    const Real v0    =  0.1974;

    const ext::shared_ptr<HestonProcess> hestonProcess(
		ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Handle<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(hestonProcess));

    const Handle<LocalVolTermStructure> localVol(
        ext::make_shared<NoExceptLocalVolSurface>(vTS, rTS, qTS, spot, 0.3));
    localVol->enableExtrapolation(true);

    const Size vGrid = 151;
    const Size xGrid = 51;

    const HestonSLVFokkerPlanckFdmParams fdmParams = {
        xGrid, vGrid, 500, 50, 100.0, 5, 2,
        0.1, 1e-4, 10000,
        1e-5, 1e-5, 0.0000025,
        1.0, 0.1, 0.9, 1e-5,
        FdmHestonGreensFct::ZeroCorrelation,
        FdmSquareRootFwdOp::Log,
        FdmSchemeDesc::ModifiedCraigSneyd()
    };

    const HestonSLVFDMModel slvModel(
         localVol, hestonModel, finalDate, fdmParams, true);

    const std::list<HestonSLVFDMModel::LogEntry>& logEntries
        = slvModel.logEntries();

    const SquareRootProcessRNDCalculator squareRootRndCalculator(
        v0, kappa, theta, sigma);

    for (const auto& logEntrie : logEntries) {

        const Time t = logEntrie.t;
        if (t > 0.2) {
            const Array x(logEntrie.mesher->getFdm1dMeshers().at(0)->locations().begin(),
                          logEntrie.mesher->getFdm1dMeshers().at(0)->locations().end());
            const std::vector<Real>& z = logEntrie.mesher->getFdm1dMeshers().at(1)->locations();

            const ext::shared_ptr<Array>& prob = logEntrie.prob;

            for (Size i=0; i < z.size(); ++i) {
                const Real pCalc = DiscreteSimpsonIntegral()(
                    x, Array(prob->begin()+i*xGrid,
                             prob->begin()+(i+1)*xGrid));

                const Real expected
                    = squareRootRndCalculator.pdf(std::exp(z[i]), t);
                const Real calculated = pCalc/std::exp(z[i]);

                if (   std::fabs(expected-calculated) > 0.01
                    && std::fabs((expected-calculated)/expected) > 0.04) {
                    BOOST_ERROR("failed to reproduce probability at "
                            << "\n  v :          " << std::exp(z[i])
                            << "\n  t :          " << t
                            << "\n  expected :   " << expected
                            << "\n  calculated : " << calculated);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(testBarrierPricingViaHestonLocalVol) {
    BOOST_TEST_MESSAGE("Testing calibration via vanilla options...");

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate(5, Nov, 2015);
    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Rate r = 0.1;
    const Rate q = 0.025;

    const Real kappa =  2.0;
    const Real theta =  0.09;
    const Real rho   = -0.75;
    const Real sigma =  0.8;
    const Real v0    =  0.19;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const ext::shared_ptr<HestonProcess> hestonProcess(
		ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma, rho));

    const Handle<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(hestonProcess));

    const Handle<BlackVolTermStructure> surf(
        ext::make_shared<HestonBlackVolSurface>(hestonModel));

    const Real strikeValues[] = { 50, 75, 100, 125, 150, 200, 400 };
    const Period maturities[] = {
        Period(1, Months), Period(2,Months),
        Period(3, Months), Period(4, Months),
        Period(5, Months), Period(6, Months),
        Period(9, Months), Period(1, Years),
        Period(18, Months), Period(2, Years),
        Period(3, Years), Period(5, Years) };

    const ext::shared_ptr<LocalVolSurface> localVolSurface(
		ext::make_shared<LocalVolSurface>(surf, rTS, qTS, spot));

    const ext::shared_ptr<PricingEngine> hestonEngine(
		ext::make_shared<AnalyticHestonEngine>(hestonModel.currentLink(), 164));

    for (Real strike : strikeValues) {
        for (auto maturitie : maturities) {
            const Date exerciseDate = todaysDate + maturitie;
            const Time t = dc.yearFraction(todaysDate, exerciseDate);

            const Volatility impliedVol = surf->blackVol(t, strike, true);

            const ext::shared_ptr<GeneralizedBlackScholesProcess>
                bsProcess(ext::make_shared<GeneralizedBlackScholesProcess>(
                    spot, qTS, rTS,
                    Handle<BlackVolTermStructure>(flatVol(impliedVol, dc))));

            const ext::shared_ptr<PricingEngine> analyticEngine(
				ext::make_shared<AnalyticEuropeanEngine>(bsProcess));

            const ext::shared_ptr<Exercise> exercise(
				ext::make_shared<EuropeanExercise>(exerciseDate));
            const ext::shared_ptr<StrikedTypePayoff> payoff(
				ext::make_shared<PlainVanillaPayoff>(
                    spot->value() < strike ? Option::Call : Option::Put,
                    strike));

            const ext::shared_ptr<PricingEngine> localVolEngine(
				ext::make_shared<FdBlackScholesVanillaEngine>(bsProcess, 201, 801, 0,
                    FdmSchemeDesc::Douglas(), true));

            VanillaOption option(payoff, exercise);

            option.setPricingEngine(analyticEngine);
            const Real analyticNPV = option.NPV();

            option.setPricingEngine(hestonEngine);
            const Real hestonNPV = option.NPV();

            option.setPricingEngine(localVolEngine);
            const Real localVolNPV = option.NPV();

            const Real tol = 1e-3;
            if (std::fabs(analyticNPV - hestonNPV) > tol)
                BOOST_ERROR("Heston and BS price do not match "
                        << "\n  Heston :       " << hestonNPV
                        << "\n  Black-Scholes: " << analyticNPV
                        << "\n  diff :   "
                        << std::fabs(analyticNPV - hestonNPV));

            if (std::fabs(analyticNPV - localVolNPV) > tol)
                BOOST_ERROR("LocalVol and BS price do not match "
                        << "\n  LocalVol :     " << localVolNPV
                        << "\n  Black-Scholes: " << analyticNPV
                        << "\n  diff :   "
                        << std::fabs(analyticNPV - localVolNPV));
        }
    }
}

//BOOST_AUTO_TEST_CASE(testBarrierPricingMixedModels) {
//    BOOST_TEST_MESSAGE("Testing Barrier pricing with mixed models...");
//
//    const DayCounter dc = ActualActual(ActualActual::ISDA);
//    const Date todaysDate(5, Nov, 2015);
//    const Date exerciseDate = todaysDate + Period(1, Years);
//    Settings::instance().evaluationDate() = todaysDate;
//
//    const Real s0 = 100;
//    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
//    const Rate r = 0.05;
//    const Rate q = 0.02;
//
//    const Real kappa =  2.0;
//    const Real theta =  0.09;
//    const Real rho   = -0.75;
//    const Real sigma =  0.4;
//    const Real v0    =  0.19;
//
//    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
//    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
//
//    const ext::shared_ptr<HestonProcess> hestonProcess(
//		ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma, rho));
//
//    const Handle<HestonModel> hestonModel(
//        ext::make_shared<HestonModel>(hestonProcess));
//
//    const Handle<BlackVolTermStructure> impliedVolSurf(
//        ext::make_shared<HestonBlackVolSurface>(hestonModel));
//
//    const Handle<LocalVolTermStructure> localVolSurf(
//        ext::make_shared<NoExceptLocalVolSurface>(
//            impliedVolSurf, rTS, qTS, spot, 0.3));
//
//    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
//        ext::make_shared<GeneralizedBlackScholesProcess>(
//            spot, qTS, rTS, impliedVolSurf));
//
//    const ext::shared_ptr<Exercise> exercise(
//        ext::make_shared<EuropeanExercise>(exerciseDate));
//    const ext::shared_ptr<StrikedTypePayoff> payoff(
//        ext::make_shared<PlainVanillaPayoff>(Option::Put, s0));
//
//    const ext::shared_ptr<PricingEngine> hestonEngine(
//        ext::make_shared<FdHestonBarrierEngine>(
//            hestonModel.currentLink(), 26, 101, 51));
//
//    const ext::shared_ptr<PricingEngine> localEngine(
//        ext::make_shared<FdBlackScholesBarrierEngine>(
//            bsProcess, 26, 101, 0, FdmSchemeDesc::Douglas(), true, 0.3));
//
//    const Real barrier = 10.0;
//    BarrierOption barrierOption(
//        Barrier::DownOut, barrier, 0.0, payoff, exercise);
//
//    barrierOption.setPricingEngine(hestonEngine);
//    const Real hestonDeltaCalculated = barrierOption.delta();
//
//    barrierOption.setPricingEngine(localEngine);
//    const Real localDeltaCalculated = barrierOption.delta();
//
//    const Real localDeltaExpected =  -0.439068;
//    const Real hestonDeltaExpected = -0.342059;
//    const Real tol = 0.001;
//    if (std::fabs(hestonDeltaExpected - hestonDeltaCalculated) > tol) {
//        BOOST_ERROR("Heston Delta does not match"
//                << "\n calculated : " << hestonDeltaCalculated
//                << "\n expected   : " << hestonDeltaExpected);
//    }
//    if (std::fabs(localDeltaExpected - localDeltaCalculated) > tol) {
//        BOOST_ERROR("Local Vol Delta does not match"
//                << "\n calculated : " << localDeltaCalculated
//                << "\n expected   : " << localDeltaExpected);
//    }
//
//    const HestonSLVFokkerPlanckFdmParams params =
//        { 51, 201, 1000, 100, 3.0, 0, 2,
//          0.1, 1e-4, 10000,
//          1e-8, 1e-8, 0.0, 1.0, 1.0, 1.0, 1e-6,
//          FdmHestonGreensFct::Gaussian,
//          FdmSquareRootFwdOp::Plain,
//          FdmSchemeDesc::ModifiedCraigSneyd()
//        };
//
//    const Real eta[] = { 0.1, 0.2, 0.3, 0.4,
//                         0.5, 0.6, 0.7, 0.8, 0.9, 1.0 };
//
//    const Real slvDeltaExpected[] = {
//        -0.429475, -0.419749, -0.410055, -0.400339,
//        -0.390616, -0.380888, -0.371156, -0.361425,
//        -0.351699, -0.341995 };
//
//    for (Size i=0; i < LENGTH(eta); ++i) {
//        const Handle<HestonModel> modHestonModel(
//            ext::make_shared<HestonModel>(
//                ext::make_shared<HestonProcess>(
//                    rTS, qTS, spot, v0, kappa, theta, eta[i]*sigma, rho)));
//
//        const HestonSLVFDMModel slvModel(
//            localVolSurf, modHestonModel, exerciseDate, params);
//
//        const ext::shared_ptr<LocalVolTermStructure>
//            leverageFct = slvModel.leverageFunction();
//
//        const ext::shared_ptr<PricingEngine> slvEngine(
//            ext::make_shared<FdHestonBarrierEngine>(
//                modHestonModel.currentLink(), 201, 801, 201, 0,
//                FdmSchemeDesc::Hundsdorfer(), leverageFct));
//
//        BarrierOption barrierOption(
//            Barrier::DownOut, barrier, 0.0, payoff, exercise);
//
//        barrierOption.setPricingEngine(slvEngine);
//        const Real slvDeltaCalculated = barrierOption.delta();
//
//        if (std::fabs(slvDeltaExpected[i] - slvDeltaCalculated) > tol) {
//            BOOST_ERROR("Stochastic Local Vol Delta does not match"
//                    << "\n calculated : " << slvDeltaCalculated
//                    << "\n expected   : " << slvDeltaExpected);
//        }
//    }
//}

BOOST_AUTO_TEST_CASE(testMonteCarloVsFdmPricing, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE(
        "Testing Monte-Carlo vs FDM Pricing for "
        "Heston SLV models...");

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate(5, Dec, 2015);
    const Date exerciseDate = todaysDate + Period(1, Years);
    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Rate r = 0.05;
    const Rate q = 0.02;

    const Real kappa =  2.0;
    const Real theta =  0.18;
    const Real rho   =  -0.75;
    const Real sigma =  0.8;
    const Real v0    =  0.19;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const ext::shared_ptr<HestonProcess> hestonProcess
        = ext::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, sigma, rho);

    const ext::shared_ptr<HestonModel> hestonModel
        = ext::make_shared<HestonModel>(hestonProcess);

    const ext::shared_ptr<LocalVolTermStructure> leverageFct
        = ext::make_shared<LocalConstantVol>(todaysDate, 0.25, dc);

    const ext::shared_ptr<HestonSLVProcess> slvProcess
        = ext::make_shared<HestonSLVProcess>(hestonProcess, leverageFct);

    const ext::shared_ptr<PricingEngine> mcEngine
        = MakeMCEuropeanHestonEngine<
            PseudoRandom, GeneralStatistics, HestonSLVProcess>(slvProcess)
            .withStepsPerYear(100)
            .withAntitheticVariate()
            .withSamples(10000)
            .withSeed(1234);

    const ext::shared_ptr<PricingEngine> fdEngine
        = ext::make_shared<FdHestonVanillaEngine>(
            hestonModel, 51, 401, 101, 0,
            FdmSchemeDesc::ModifiedCraigSneyd(), leverageFct);

    const ext::shared_ptr<Exercise> exercise
        = ext::make_shared<EuropeanExercise>(exerciseDate);

    const ext::shared_ptr<HestonProcess> mixingProcess
        = ext::make_shared<HestonProcess>(rTS, qTS, spot, v0, kappa, theta, sigma * 10, rho,
                                          HestonProcess::QuadraticExponentialMartingale);
    const ext::shared_ptr<HestonModel> mixingModel
        = ext::make_shared<HestonModel>(mixingProcess);

    const ext::shared_ptr<PricingEngine> fdEngineWithMixingFactor
        = ext::make_shared<FdHestonVanillaEngine>(
            mixingModel, 51, 401, 101, 0,
            FdmSchemeDesc::ModifiedCraigSneyd(), leverageFct, 0.1);

    const Real strikes[] = { s0, 1.1*s0 };
    for (Real strike : strikes) {
        const ext::shared_ptr<StrikedTypePayoff> payoff =
            ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);

        VanillaOption option(payoff, exercise);

        option.setPricingEngine(fdEngine);

        const Real priceFDM = option.NPV();

        option.setPricingEngine(fdEngineWithMixingFactor);

        const Real priceFDMWithMix = option.NPV();

        option.setPricingEngine(mcEngine);

        const Real priceMC = option.NPV();
        const Real priceError = option.errorEstimate();

        if (priceError > 0.1) {
            BOOST_ERROR("Heston Monte-Carlo error is too large"
                    << "\n MC Error: " << priceError
                    << "\n Limit   : " << 0.1);
        }

        if (std::fabs(priceFDM - priceMC) > 2.3*priceError) {
            BOOST_ERROR("Heston Monte-Carlo price does not match with FDM"
                    << "\n MC Price : " << priceMC
                    << "\n MC Error : " << priceError
                    << "\n FDM Price: " << priceFDM);
        }

        if (priceFDM != priceFDMWithMix) {
            BOOST_ERROR("Heston mixing FDM price does not match with non-mixing FDM"
                        << "\n Mixing FDM Price : " << priceFDMWithMix
                        << "\n Non Mixing FDM Price : " << priceFDM);
        }
    }
}

BOOST_AUTO_TEST_CASE(testMonteCarloCalibration, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE(
        "Testing Monte-Carlo Calibration...");

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate(5, Jan, 2016);
    const Date maturityDate = todaysDate + Period(1, Years);
    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Rate r = 0.05;
    const Rate q = 0.02;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const ext::shared_ptr<LocalVolTermStructure> localVol
          = ext::make_shared<LocalConstantVol>(todaysDate, 0.3, dc);

    // parameter of the "calibrated" Heston model
    const Real kappa =   1.0;
    const Real theta =   0.06;
    const Real rho   =  -0.75;
    const Real sigma =   0.4;
    const Real v0    =   0.09;

    const ext::shared_ptr<HestonProcess> hestonProcess
        = ext::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, sigma, rho);

    const ext::shared_ptr<HestonModel> hestonModel
        = ext::make_shared<HestonModel>(hestonProcess);

    const Size xGrid = 400;
    const Size nSims[] = { 40000 };

    for (unsigned long nSim : nSims) {
        const bool sobol = true;

        const ext::shared_ptr<LocalVolTermStructure> leverageFct =
            HestonSLVMCModel(
                Handle<LocalVolTermStructure>(localVol), Handle<HestonModel>(hestonModel),
                sobol ? ext::shared_ptr<BrownianGeneratorFactory>(new SobolBrownianGeneratorFactory(
                            SobolBrownianGenerator::Diagonal, 1234UL, SobolRsg::JoeKuoD7)) :
                        ext::shared_ptr<BrownianGeneratorFactory>(
                            new MTBrownianGeneratorFactory(1234UL)),
                maturityDate, 91, xGrid, nSim)
                .leverageFunction();

        const ext::shared_ptr<PricingEngine> bsEngine(
            ext::make_shared<AnalyticEuropeanEngine>(
                ext::make_shared<GeneralizedBlackScholesProcess>(
                    spot, qTS, rTS,
                    Handle<BlackVolTermStructure>(flatVol(0.3, dc)))));

        const Real strikes[] = { 50, 80, 100, 120, 150, 200 };
        const Date maturities[] = {
            todaysDate + Period(3, Months),  todaysDate + Period(6, Months),
            todaysDate + Period(12, Months)
        };

        Real qualityFactor = 0.0;
        Real maxQualityFactor = 0.0;
        Size nValues = 0U;

        for (auto maturity : maturities) {
            const Time maturityTime = dc.yearFraction(todaysDate, maturity);

            const ext::shared_ptr<PricingEngine> fdEngine
                = ext::make_shared<FdHestonVanillaEngine>(
                    hestonModel, std::max(Size(26), Size(maturityTime*51)),
                    201, 51, 0,
                    FdmSchemeDesc::ModifiedCraigSneyd(), leverageFct);

            const ext::shared_ptr<Exercise> exercise
                = ext::make_shared<EuropeanExercise>(maturity);

            for (Real strike : strikes) {
                const ext::shared_ptr<StrikedTypePayoff> payoff =
                    ext::make_shared<PlainVanillaPayoff>(strike < s0 ? Option::Put : Option::Call,
                                                         strike);

                VanillaOption option(payoff, exercise);

                option.setPricingEngine(bsEngine);
                const Real bsNPV = option.NPV();
                const Real bsVega = option.vega();

                if (bsNPV > 0.02) {
                    option.setPricingEngine(fdEngine);
                    const Real fdmNPV = option.NPV();

                    const Real diff = std::fabs(fdmNPV-bsNPV)/bsVega*1e4;

                    qualityFactor+=diff;
                    maxQualityFactor = std::max(maxQualityFactor, diff);
                    ++nValues;
                }
            }
        }

        if (qualityFactor/nValues > 7.5) {
            BOOST_ERROR(
                "Failed to reproduce average calibration quality"
                << "\n average calibration quality : "
                << qualityFactor/nValues << "bp"
                << "\n tolerance                  :  5.0bp");
        }

        if (qualityFactor/nValues > 15.0) {
            BOOST_ERROR(
                "Failed to reproduce maximum calibration error"
                << "\n maximum calibration error : "
                << maxQualityFactor << "bp"
                << "\n tolerance                 : 15.0bp");
        }
    }
}

//BOOST_AUTO_TEST_CASE(testForwardSkewSLV) {
//    BOOST_TEST_MESSAGE("Testing the implied volatility skew of "
//        "forward starting options in SLV model...");
//
//    const DayCounter dc = ActualActual(ActualActual::ISDA);
//    const Date todaysDate(5, Jan, 2017);
//    const Date maturityDate = todaysDate + Period(2, Years);
//    Settings::instance().evaluationDate() = todaysDate;
//
//    const Real s0 = 100;
//    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
//    const Rate r = 0.05;
//    const Rate q = 0.02;
//    const Volatility flatLocalVol = 0.3;
//
//    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
//    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
//
//    const Handle<LocalVolTermStructure> localVol(
//        ext::make_shared<LocalConstantVol>(todaysDate, flatLocalVol, dc));
//
//    // parameter of the "calibrated" Heston model
//    const Real kappa =   2.0;
//    const Real theta =   0.06;
//    const Real rho   =  -0.75;
//    const Real sigma =   0.6;
//    const Real v0    =   0.09;
//
//    const ext::shared_ptr<HestonProcess> hestonProcess
//        = ext::make_shared<HestonProcess>(
//            rTS, qTS, spot, v0, kappa, theta, sigma, rho);
//
//    const Handle<HestonModel> hestonModel(
//        ext::make_shared<HestonModel>(hestonProcess));
//
//
//    // Monte-Carlo calibration
//    const Size nSim = 40000;
//    const Size xGrid = 200;
//
//    const ext::shared_ptr<LocalVolTermStructure> leverageFctMC =
//        HestonSLVMCModel(
//            localVol, hestonModel,
//            ext::shared_ptr<BrownianGeneratorFactory>(new MTBrownianGeneratorFactory(1234UL)),
//            maturityDate, 182, xGrid, nSim)
//            .leverageFunction();
//
//    const ext::shared_ptr<HestonSLVProcess> mcSlvProcess(
//        ext::make_shared<HestonSLVProcess>(hestonProcess, leverageFctMC));
//
//    // finite difference calibration
//    const HestonSLVFokkerPlanckFdmParams logParams = {
//        201, 401, 1000, 30, 2.0, 0, 2,
//        0.1, 1e-4, 10000,
//        1e-5, 1e-5, 0.0000025, 1.0, 0.1, 0.9, 1e-5,
//        FdmHestonGreensFct::Gaussian,
//        FdmSquareRootFwdOp::Log,
//        FdmSchemeDesc::ModifiedCraigSneyd()
//    };
//
//    const ext::shared_ptr<LocalVolTermStructure> leverageFctFDM =
//        HestonSLVFDMModel(
//            localVol, hestonModel, maturityDate, logParams).leverageFunction();
//
//    const ext::shared_ptr<HestonSLVProcess> fdmSlvProcess(
//        ext::make_shared<HestonSLVProcess>(hestonProcess, leverageFctFDM));
//
//    const Date resetDate = todaysDate + Period(12, Months);
//    const Time resetTime = dc.yearFraction(todaysDate, resetDate);
//    const Time maturityTime = dc.yearFraction(todaysDate, maturityDate);
//    std::vector<Time> mandatoryTimes = {resetTime, maturityTime};
//
//    const Size tSteps = 100;
//    const TimeGrid grid(mandatoryTimes.begin(), mandatoryTimes.end(), tSteps);
//    const Size resetIndex = grid.closestIndex(resetTime);
//
//    typedef SobolBrownianBridgeRsg rsg_type;
//    typedef MultiPathGenerator<rsg_type>::sample_type sample_type;
//
//    const Size factors = mcSlvProcess->factors();
//
//    std::vector<ext::shared_ptr<MultiPathGenerator<rsg_type> > > pathGen = {
//        ext::make_shared<MultiPathGenerator<rsg_type> >(
//            mcSlvProcess, grid, rsg_type(factors, tSteps), false),
//        ext::make_shared<MultiPathGenerator<rsg_type> >(
//            fdmSlvProcess, grid, rsg_type(factors, tSteps), false)
//    };
//
//    const Real strikes[] = {
//        0.5, 0.7, 0.8, 0.9, 1.0, 1.1, 1.25, 1.5, 1.75, 2.0
//    };
//
//    std::vector<std::vector<GeneralStatistics> >
//        stats(2, std::vector<GeneralStatistics>(LENGTH(strikes)));
//
//    for (Size i=0; i < 5*nSim; ++i) {
//        for (Size k= 0; k < 2; ++k) {
//            const sample_type& path = pathGen[k]->next();
//
//            const Real S_t1 = path.value[0][resetIndex-1];
//            const Real S_T1 = path.value[0][tSteps-1];
//
//            const sample_type& antiPath = pathGen[k]->antithetic();
//            const Real S_t2 = antiPath.value[0][resetIndex-1];
//            const Real S_T2 = antiPath.value[0][tSteps-1];
//
//            for (Size j=0; j < LENGTH(strikes); ++j) {
//                const Real strike = strikes[j];
//                if (strike < 1.0)
//                    stats[k][j].add(0.5*(
//                          S_t1 * std::max(0.0, strike - S_T1/S_t1)
//                        + S_t2 * std::max(0.0, strike - S_T2/S_t2)));
//                else
//                    stats[k][j].add(0.5*(
//                          S_t1 * std::max(0.0, S_T1/S_t1 - strike)
//                        + S_t2 * std::max(0.0, S_T2/S_t2 - strike)));
//            }
//        }
//    }
//
//    const ext::shared_ptr<Exercise> exercise(
//        ext::make_shared<EuropeanExercise>(maturityDate));
//
//    const ext::shared_ptr<SimpleQuote> vol(
//        ext::make_shared<SimpleQuote>(flatLocalVol));
//    const Handle<BlackVolTermStructure> volTS(flatVol(todaysDate, vol, dc));
//
//    const ext::shared_ptr<GeneralizedBlackScholesProcess> bsProcess(
//		ext::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS, volTS));
//
//    const ext::shared_ptr<PricingEngine> fwdEngine(
//        ext::make_shared<ForwardVanillaEngine<AnalyticEuropeanEngine> >(
//            bsProcess));
//
//    const Volatility expected[] = {
//        0.37804, 0.346608, 0.330682, 0.314978, 0.300399,
//        0.287273, 0.272916, 0.26518, 0.268663, 0.277052
//    };
//
//    const DiscountFactor df = rTS->discount(grid.back());
//
//    for (Size j=0; j < LENGTH(strikes); ++j) {
//        for (Size k= 0; k < 2; ++k) {
//            const Real strike = strikes[j];
//            const Real npv = stats[k][j].mean() * df;
//
//            const ext::shared_ptr<StrikedTypePayoff> payoff(
//                ext::make_shared<PlainVanillaPayoff>(
//                    (strike < 1.0) ? Option::Put : Option::Call, strike));
//
//            const ext::shared_ptr<ForwardVanillaOption> fwdOption(
//                ext::make_shared<ForwardVanillaOption>(
//                    strike, resetDate, payoff, exercise));
//
//            const Volatility implVol =
//                QuantLib::detail::ImpliedVolatilityHelper::calculate(
//                    *fwdOption, *fwdEngine, *vol, npv, 1e-8, 200, 1e-4, 2.0);
//
//            const Real tol = 0.002;
//            const Volatility volError = std::fabs(implVol - expected[j]);
//
//            if (volError > tol) {
//                BOOST_ERROR("Implied forward volatility error is too large"
//                        << "\n expected forward volatility: " << expected[j]
//                        << "\n SLV forward volatility     : " << implVol
//                        << "\n difference                 : " << volError
//                        << "\n tolerance                  : " << tol
//                        << "\n calibration method         : " <<
//                        ((k) ? "Monte-Carlo" : "Finite Difference"));
//            }
//        }
//    }
//}

BOOST_AUTO_TEST_CASE(testMoustacheGraph, *precondition(if_speed(Fast))) {
    BOOST_TEST_MESSAGE(
        "Testing double no touch pricing with SLV and mixing...");

    /*
     A more detailed description of this test case can found on
     https://hpcquantlib.wordpress.com/2016/01/10/monte-carlo-calibration-of-the-heston-stochastic-local-volatiltiy-model/

     The comparison of Black-Scholes and SLV prices is derived
     from figure 8.8 in Iain J. Clark's book,
     Foreign Exchange Option Pricing: A Practitioner’s Guide
    */

    const DayCounter dc = ActualActual(ActualActual::ISDA);
    const Date todaysDate(5, Jan, 2016);
    const Date maturityDate = todaysDate + Period(1, Years);
    Settings::instance().evaluationDate() = todaysDate;

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Rate r = 0.02;
    const Rate q = 0.01;

    // parameter of the "calibrated" Heston model
    const Real kappa =   1.0;
    const Real theta =   0.06;
    const Real rho   =  -0.8;
    const Real sigma =   0.8;
    const Real v0    =   0.09;

    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(q, dc));

    const ext::shared_ptr<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, spot, v0, kappa, theta, sigma, rho)));

    const ext::shared_ptr<Exercise> europeanExercise(
        ext::make_shared<EuropeanExercise>(maturityDate));

    VanillaOption vanillaOption(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0),
        europeanExercise);

    vanillaOption.setPricingEngine(
        ext::make_shared<AnalyticHestonEngine>(hestonModel));

    const Volatility implVol = vanillaOption.impliedVolatility(
        vanillaOption.NPV(),
        ext::make_shared<GeneralizedBlackScholesProcess>(spot, qTS, rTS,
            Handle<BlackVolTermStructure>(flatVol(std::sqrt(theta), dc))));

    const ext::shared_ptr<PricingEngine> analyticEngine(
        ext::make_shared<AnalyticDoubleBarrierBinaryEngine>(
            ext::make_shared<GeneralizedBlackScholesProcess>(
                spot, qTS, rTS,
                Handle<BlackVolTermStructure>(flatVol(implVol, dc)))));

    std::vector<Time> expiries;
    const Period timeStepPeriod = Period(1, Weeks);
    for (Date expiry = todaysDate + timeStepPeriod;
        expiry <= maturityDate; expiry+=timeStepPeriod) {
        expiries.push_back(dc.yearFraction(todaysDate, expiry));
    }

    const ext::shared_ptr<TimeGrid> timeGrid(
        ext::make_shared<TimeGrid>(expiries.begin(), expiries.end()));

    // first build the true local vol surface from another Heston model
    const Handle<LocalVolTermStructure> localVol(
        getFixedLocalVolFromHeston(hestonModel, timeGrid));

    const ext::shared_ptr<BrownianGeneratorFactory> sobolGeneratorFactory(
        ext::make_shared<SobolBrownianGeneratorFactory>(SobolBrownianGenerator::Diagonal, 1234UL,
                                                        SobolRsg::JoeKuoD7));

    const Size xGrid = 100;
    const Size nSim  = 20000;

    const Real eta = 0.90;

    const Handle<HestonModel> modHestonModel(
        ext::make_shared<HestonModel>(
            ext::make_shared<HestonProcess>(
                rTS, qTS, spot, v0, kappa, theta, eta*sigma, rho)));

    const ext::shared_ptr<LocalVolTermStructure> leverageFct =
        HestonSLVMCModel(localVol, modHestonModel, sobolGeneratorFactory,
                         maturityDate, 182, xGrid, nSim)
            .leverageFunction();

    const ext::shared_ptr<PricingEngine> fdEngine(
        ext::make_shared<FdHestonDoubleBarrierEngine>(
            modHestonModel.currentLink(), 51, 101, 31, 0,
            FdmSchemeDesc::Hundsdorfer(), leverageFct));

    const Real expected[] = {
         0.0334, 0.1141, 0.1319, 0.0957, 0.0464, 0.0058,-0.0192,
        -0.0293,-0.0297,-0.0251,-0.0192,-0.0134,-0.0084,-0.0045,
        -0.0015, 0.0005, 0.0017, 0.0020
    };
    const Real tol = 1e-2;

    for (Size i=0; i < 18; ++i) {
        const Real dist = 10.0+5.0*i;

        const Real barrier_lo = std::max(s0 - dist, 1e-2);
        const Real barrier_hi = s0 + dist;
        DoubleBarrierOption doubleBarrier(
            DoubleBarrier::KnockOut, barrier_lo, barrier_hi, 0.0,
            ext::make_shared<CashOrNothingPayoff>(
                Option::Call, 0.0, 1.0),
            europeanExercise);

        doubleBarrier.setPricingEngine(analyticEngine);
        const Real bsNPV = doubleBarrier.NPV();

        doubleBarrier.setPricingEngine(fdEngine);
        const Real slvNPV = doubleBarrier.NPV();

        const Real diff = slvNPV - bsNPV;
        if (std::fabs(diff - expected[i]) > tol) {
            BOOST_ERROR(
                "Failed to reproduce price difference for a Double-No-Touch "
                << "option between Black-Scholes and "
                << "Heston Stochastic Local Volatility model"
                << "\n Barrier Low        : " << barrier_lo
                << "\n Barrier High       : " << barrier_hi
                << "\n Black-Scholes Price: " << bsNPV
                << "\n Heston SLV Price   : " << slvNPV
                << "\n diff               : " << diff
                << "\n expected diff      : " << expected[i]
                << "\n tolerance          : " << tol);
        }
    }
}

BOOST_AUTO_TEST_CASE(testDiffusionAndDriftSlvProcess) {
    BOOST_TEST_MESSAGE(
        "Testing diffusion and drift of the SLV process...");

    const Date todaysDate(6, June, 2020);
    Settings::instance().evaluationDate() = todaysDate;

    const DayCounter dc = Actual365Fixed();
    const Date maturityDate = todaysDate + Period(6, Months);
    const Time maturity = dc.yearFraction(todaysDate, maturityDate);

    const Real s0 = 100;
    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
    const Rate r = -0.005;
    const Rate q =  0.04;

    const Handle<YieldTermStructure> rTS(flatRate(todaysDate, r, dc));
    const Handle<YieldTermStructure> qTS(flatRate(todaysDate, q, dc));

    const ext::shared_ptr<LocalVolTermStructure> localVol =
        getFixedLocalVolFromHeston(
            ext::make_shared<HestonModel>(
                ext::make_shared<HestonProcess>(
                    rTS, qTS, spot, 0.1, 1.0, 0.13, 0.8, 0.4)),
            ext::make_shared<TimeGrid>(maturity, 20));

    const Real kappa =  2.5;
    const Real theta =  1.0;
    const Real rho   =  -0.75;
    const Real sigma =  2.4;
    const Real v0    =  1.0;

    const ext::shared_ptr<HestonProcess> hestonProcess =
        ext::make_shared<HestonProcess>(
            rTS, qTS, spot, v0, kappa, theta, sigma, rho);

    const Handle<HestonModel> hestonModel(
        ext::make_shared<HestonModel>(hestonProcess));

    const ext::shared_ptr<HestonSLVProcess> slvProcess =
        ext::make_shared<HestonSLVProcess>(hestonProcess, localVol);

    VanillaOption option(
        ext::make_shared<PlainVanillaPayoff>(Option::Call, s0),
        ext::make_shared<EuropeanExercise>(maturityDate));

    option.setPricingEngine(
        ext::make_shared<FdHestonVanillaEngine>(
            hestonModel.currentLink(),
            26, 201, 101, 0,
            FdmSchemeDesc::ModifiedCraigSneyd(),
            localVol));

    const Real expected = option.NPV();

    const Size nSims = 16733;
    const Size nTimeSteps = 40;
    const DiscountFactor df = rTS->discount(maturity);

    SobolBrownianBridgeRsg rsg(2, nTimeSteps, SobolBrownianGenerator::Diagonal, 12345U,
                               SobolRsg::JoeKuoD7);

    Array x(2), xt(2), dw(2);
    GeneralStatistics stats;

    const Time dt = maturity/nTimeSteps;
    const Real sqrtDt = std::sqrt(dt);

    for (Size i=0; i < nSims; ++i) {
        Time t = 0.0;
        x[0] = s0; x[1] = v0;

        const std::vector<Real> n = rsg.nextSequence().value;

        for (Size j=0; j < nTimeSteps; ++j, t+=dt) {

            dw[0] = n[j];
            dw[1] = n[j+nTimeSteps];

            // full truncation scheme
            xt[0] = x[0];
            xt[1] = (x[1] > 0)? x[1] : 0.0;

            x = slvProcess->apply(x,
                    slvProcess->diffusion(t, xt)*sqrtDt*dw
                 + slvProcess->drift(t, xt)*dt);
        }

        stats.add(df*option.payoff()->operator()(x[0]));
    }

    const Real calculated = stats.mean();
    const Real errorEstimate = stats.errorEstimate();

    const Real diff = std::fabs(expected - calculated);

    if (diff > 2.35*errorEstimate) {
        BOOST_ERROR(
            "Failed to reproduce call option price with HestonSLVProcess "
            "diffusion and drift discretization scheme"
            << "\n expected   : " << expected
            << "\n calculated : " << calculated
            << "\n error est. : " << errorEstimate
            << "\n diff       : " << diff);
    }
}

//BOOST_AUTO_TEST_CASE(testBarrierPricingMixedModelsMonteCarloVsFdmPricing) {
//    BOOST_TEST_MESSAGE(
//        "Testing European and Barrier Pricing for Monte-Carlo and FDM "
//        "Pricing in Heston SLV models with a mixing factor...");
//
//    const Real epsilon = 0.015;
//
//    const DayCounter dc = ActualActual(ActualActual::ISDA);
//    const Date todaysDate(1, Jul, 2021);
//    const Date maturityDate = todaysDate + Period(2, Years);
//    const Time maturity = dc.yearFraction(todaysDate, maturityDate);
//    Settings::instance().evaluationDate() = todaysDate;
//
//    const Real s0 = 100;
//    const Handle<Quote> spot(ext::make_shared<SimpleQuote>(s0));
//    const Rate r = 0.02;
//    const Rate q = 0.01;
//    const Real mixingFactors[] = {1.0, 0.64, 0.3};
//    const std::vector<Date>& requiredDates = std::vector<Date>();
//
//    // Create two slightly different Heston models. The first will be our stochastic
//    // vol model, the second is used to create a similar implied vol surface which
//    // we fit a local vol model to
//    const Real kappa1 =  2.0;
//    const Real theta1 =  0.12;
//    const Real rho1   =  -0.25;
//    const Real sigma1 =  0.8;
//    const Real v01    =  0.09;
//
//    const Real kappa2 =  1.5;
//    const Real theta2 =  0.11;
//    const Real rho2   =  -0.2;
//    const Real sigma2 =  0.9;
//    const Real v02    =  0.1;
//
//    const Handle<YieldTermStructure> rTS(flatRate(r, dc));
//    const Handle<YieldTermStructure> qTS(flatRate(q, dc));
//
//    const ext::shared_ptr<HestonProcess> hestonProcess
//        = ext::make_shared<HestonProcess>(
//            rTS, qTS, spot, v01, kappa1, theta1, sigma1, rho1);
//
//    const ext::shared_ptr<HestonModel> hestonModelPtr
//        = ext::make_shared<HestonModel>(hestonProcess);
//
//    const ext::shared_ptr<HestonProcess> hestonProcess2
//        = ext::make_shared<HestonProcess>(
//            rTS, qTS, spot, v02, kappa2, theta2, sigma2, rho2);
//
//    const ext::shared_ptr<HestonModel> hestonModelPtr2
//        = ext::make_shared<HestonModel>(hestonProcess2);
//
//    const ext::shared_ptr<LocalVolTermStructure> localVolPtr =
//        getFixedLocalVolFromHeston(hestonModelPtr2,
//            ext::make_shared<TimeGrid>(maturity, 20));
//
//    const Handle<LocalVolTermStructure> localVol = Handle<LocalVolTermStructure>(localVolPtr);
//    localVol->enableExtrapolation();
//    const Handle<HestonModel> hestonModel = Handle<HestonModel>(hestonModelPtr);
//    const Handle<HestonModel> hestonModel2 = Handle<HestonModel>(hestonModelPtr2);
//
//    // Create the options we will price - a vanilla and a barrier
//    const ext::shared_ptr<Exercise> exercise
//        = ext::make_shared<EuropeanExercise>(maturityDate);
//
//    const Real strike = 100;
//    const ext::shared_ptr<StrikedTypePayoff> payoff =
//        ext::make_shared<PlainVanillaPayoff>(Option::Call, strike);
//
//    VanillaOption vanillaOption(payoff, exercise);
//
//    const Real rebate = 0.0;
//    const Real barrier = 110.0;
//    BarrierOption barrierOption(Barrier::UpOut, barrier, rebate, payoff, exercise);
//
//    // hestonModel2 is our simulated local vol model, so its vanilla prices
//    // should match the calibrated SLV model pricers
//    const ext::shared_ptr<PricingEngine> hestonVanillaEngine
//        = ext::make_shared<AnalyticHestonEngine>(hestonModelPtr2);
//    vanillaOption.setPricingEngine(hestonVanillaEngine);
//    const Real localVolPrice = vanillaOption.NPV();
//
//    const ext::shared_ptr<BrownianGeneratorFactory> sobolGeneratorFactory(
//        ext::make_shared<SobolBrownianGeneratorFactory>(SobolBrownianGenerator::Diagonal, 1234UL,
//                                                        SobolRsg::JoeKuoD7));
//
//    for (Real mixingFactor : mixingFactors) {
//
//        // Finite Difference calibration
//        const HestonSLVFokkerPlanckFdmParams logParams = {
//            201, 401, 1000, 30, 2.0, 0, 2,
//            0.1, 1e-4, 10000,
//            1e-5, 1e-5, 0.0000025, 1.0, 0.1, 0.9, 1e-5,
//            FdmHestonGreensFct::Gaussian,
//            FdmSquareRootFwdOp::Log,
//            FdmSchemeDesc::ModifiedCraigSneyd()
//        };
//
//        const ext::shared_ptr<LocalVolTermStructure> leverageFctFDM =
//            HestonSLVFDMModel(
//                localVol, hestonModel, maturityDate, logParams, false, requiredDates,
//                mixingFactor).leverageFunction();
//
//        // Monte-Carlo calibration
//        const Size timeStepsPerYear = 365;
//        const Size nBins = 201;
//        const Size calibrationPaths = 65536;
//
//        const ext::shared_ptr<LocalVolTermStructure> leverageFctMC =
//            HestonSLVMCModel(
//                localVol, hestonModel,
//                sobolGeneratorFactory,
//                maturityDate, timeStepsPerYear, nBins, calibrationPaths, requiredDates,
//                mixingFactor).leverageFunction();
//
//        // Create SLV pricing engines with both leverage functions
//        const ext::shared_ptr<PricingEngine> fdEngineWithMixingFactor
//            = ext::make_shared<FdHestonVanillaEngine>(
//                hestonModelPtr, 100, 100, 50, 0,
//                FdmSchemeDesc::Hundsdorfer(), leverageFctFDM, mixingFactor);
//
//        const ext::shared_ptr<PricingEngine> mcEngineWithMixingFactor
//            = ext::make_shared<FdHestonVanillaEngine>(
//                hestonModelPtr, 100, 100, 50, 0,
//                FdmSchemeDesc::Hundsdorfer(), leverageFctMC, mixingFactor);
//
//        const ext::shared_ptr<PricingEngine> fdBarrierEngineWithMixingFactor
//            = ext::make_shared<FdHestonBarrierEngine>(
//                hestonModelPtr, 100, 100, 50, 0,
//                FdmSchemeDesc::Hundsdorfer(), leverageFctFDM, mixingFactor);
//
//        const ext::shared_ptr<PricingEngine> mcBarrierEngineWithMixingFactor
//            = ext::make_shared<FdHestonBarrierEngine>(
//                hestonModelPtr, 100, 100, 50, 0,
//                FdmSchemeDesc::Hundsdorfer(), leverageFctMC, mixingFactor);
//
//        // Price the vanilla and barrier with both engines
//        vanillaOption.setPricingEngine(fdEngineWithMixingFactor);
//        const Real priceFDM = vanillaOption.NPV();
//
//        vanillaOption.setPricingEngine(mcEngineWithMixingFactor);
//        const Real priceMC = vanillaOption.NPV();
//
//        barrierOption.setPricingEngine(fdBarrierEngineWithMixingFactor);
//        const Real barrierPriceFDM = barrierOption.NPV();
//
//        barrierOption.setPricingEngine(mcBarrierEngineWithMixingFactor);
//        const Real barrierPriceMC = barrierOption.NPV();
//
//        // Check MC and FDM vanilla prices against local vol, and ensure that the barrier
//        // prices from MC and FDM are also consistent
//        if (relativeError(priceFDM, localVolPrice, localVolPrice) > epsilon) {
//            BOOST_ERROR("FDM price does not match with Local Vol"
//                    << "\n Local Vol Price: " << localVolPrice
//                    << "\n FDM Price: " << priceFDM
//                    << "\n Relative Error: " << relativeError(priceFDM, localVolPrice, localVolPrice)
//                    << "\n Allowed Error: " << epsilon
//                    << "\n Mixing Factor: " << mixingFactor);
//        }
//
//        if (relativeError(priceMC, localVolPrice, localVolPrice) > epsilon) {
//            BOOST_ERROR("MC price does not match with Local Vol"
//                    << "\n Local Vol Price: " << localVolPrice
//                    << "\n MC Price: " << priceMC
//                    << "\n Relative Error: " << relativeError(priceMC, localVolPrice, localVolPrice)
//                    << "\n Allowed Error: " << epsilon
//                    << "\n Mixing Factor: " << mixingFactor);
//        }
//
//        if (relativeError(barrierPriceFDM, barrierPriceMC, barrierPriceMC) > epsilon) {
//            BOOST_ERROR("FDM Barrier Price does not match MC Barrier Price"
//                    << "\n FDM Barrier Price: " << barrierPriceFDM
//                    << "\n MC Barrier Price: " << barrierPriceMC
//                    << "\n Relative Error: " << relativeError(barrierPriceFDM, barrierPriceMC, barrierPriceMC)
//                    << "\n Allowed Error: " << epsilon
//                    << "\n Mixing Factor: " << mixingFactor);
//        }
//    }
//}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
