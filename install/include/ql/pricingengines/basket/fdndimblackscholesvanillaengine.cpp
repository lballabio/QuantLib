/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2024 Klaus Spanderen

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

#include <ql/exercise.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/math/matrixutilities/getcovariance.hpp>
#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/matrixutilities/symmetricschurdecomposition.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmeshercomposite.hpp>
#include <ql/methods/finitedifferences/meshers/predefined1dmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmwienerop.hpp>
#include <ql/methods/finitedifferences/solvers/fdmndimsolver.hpp>
#include <ql/methods/finitedifferences/utilities/fdminnervaluecalculator.hpp>
#include <ql/methods/finitedifferences/stepconditions/fdmstepconditioncomposite.hpp>
#include <ql/pricingengines/basket/fdndimblackscholesvanillaengine.hpp>
#include <ql/pricingengines/basket/vectorbsmprocessextractor.hpp>
#include <boost/preprocessor/iteration/local.hpp>
#include <utility>

namespace QuantLib {

    namespace detail {
        class FdmPCABasketInnerValue: public FdmInnerValueCalculator {
          public:
            FdmPCABasketInnerValue(
                ext::shared_ptr<BasketPayoff> payoff,
                ext::shared_ptr<FdmMesher> mesher,
                Array logS0, const Array& vols,
                std::vector<ext::shared_ptr<YieldTermStructure>> qTS,
                ext::shared_ptr<YieldTermStructure> rTS,
                Matrix Q, Array l)
            : n_(logS0.size()),
              payoff_(std::move(payoff)),
              mesher_(std::move(mesher)),
              logS0_(std::move(logS0)), v_(vols*vols),
              qTS_(std::move(qTS)),
              rTS_(std::move(rTS)),
              Q_(std::move(Q)), l_(std::move(l)),
              cachedT_(Null<Real>()),
              qf_(n_) { }

            Real innerValue(const FdmLinearOpIterator& iter, Time t) override {
                if (!close_enough(t, cachedT_)) {
                    rf_ = rTS_->discount(t);
                    for (Size i=0; i < n_; ++i)
                        qf_[i] = qTS_[i]->discount(t);
                }
                Array x(n_);
                for (Size i=0; i < n_; ++i)
                    x[i] = mesher_->location(iter, i);

                const Array S = Exp(Q_*x - 0.5*v_*t + logS0_)*qf_/rf_;

                return (*payoff_)(S);
            }
            Real avgInnerValue(const FdmLinearOpIterator& iter, Time t) override {
                return innerValue(iter, t);
            }

          private:
            const Size n_;
            const ext::shared_ptr<BasketPayoff> payoff_;
            const ext::shared_ptr<FdmMesher> mesher_;
            const Array logS0_, v_;
            const std::vector<ext::shared_ptr<YieldTermStructure>> qTS_;
            const ext::shared_ptr<YieldTermStructure> rTS_;
            const Matrix Q_;
            const Array l_;
            Time cachedT_;
            Array qf_;
            DiscountFactor rf_;
        };
    }

    FdndimBlackScholesVanillaEngine::FdndimBlackScholesVanillaEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix rho,
        std::vector<Size> xGrids,
        Size tGrid, Size dampingSteps,
        const FdmSchemeDesc& schemeDesc)
    : processes_(std::move(processes)),
      rho_(std::move(rho)),
      xGrids_(std::move(xGrids)),
      tGrid_(tGrid),
      dampingSteps_(dampingSteps),
      schemeDesc_(schemeDesc) {

        QL_REQUIRE(!processes_.empty(), "no Black-Scholes process is given.");
        QL_REQUIRE(rho_.size1() == rho_.size2()
                && rho_.size1() == processes_.size(),
                "correlation matrix has the wrong size.");
        QL_REQUIRE(xGrids_.size() == 1 || xGrids_.size() == processes_.size(),
                "wrong number of xGrids is given.");

        std::for_each(processes_.begin(), processes_.end(),
            [this](const auto& p) { registerWith(p); });
    }

    FdndimBlackScholesVanillaEngine::FdndimBlackScholesVanillaEngine(
        std::vector<ext::shared_ptr<GeneralizedBlackScholesProcess> > processes,
        Matrix rho, Size xGrid, Size tGrid, Size dampingSteps,
        const FdmSchemeDesc& schemeDesc)
    : FdndimBlackScholesVanillaEngine(
        std::move(processes), std::move(rho), std::vector<Size>(1, xGrid), tGrid, dampingSteps, schemeDesc)
    {}


    void FdndimBlackScholesVanillaEngine::calculate() const {
        #ifndef PDE_MAX_SUPPORTED_DIM
        #define PDE_MAX_SUPPORTED_DIM 4
        #endif
        QL_REQUIRE(processes_.size() <= PDE_MAX_SUPPORTED_DIM,
            "This engine does not support " << processes_.size() << " underlyings. "
            << "Max number of underlyings is " << PDE_MAX_SUPPORTED_DIM << ". "
            << "Please change preprocessor constant PDE_MAX_SUPPORTED_DIM and recompile "
            << "if a larger number of underlyings is needed.");

        const Date maturityDate = arguments_.exercise->lastDate();
        const Time maturity = processes_[0]->time(maturityDate);
        const Real sqrtT = std::sqrt(maturity);

        const detail::VectorBsmProcessExtractor pExtractor(processes_);
        const Array s = pExtractor.getSpot();
        const Array dq = pExtractor.getDividendYieldDf(maturityDate);
        const Array stdDev = Sqrt(pExtractor.getBlackVariance(maturityDate));
        const Array vols = stdDev / sqrtT;

        const SymmetricSchurDecomposition schur(
            getCovariance(vols.begin(), vols.end(), rho_));
        const Matrix& Q = schur.eigenvectors();
        const Array& l = schur.eigenvalues();

        const Real eps = 1e-4;
        std::vector<ext::shared_ptr<Fdm1dMesher> > meshers;

        for (Size i=0; i < processes_.size(); ++i) {
            const Size xGrid = (xGrids_.size() > 1)
                ? xGrids_[i]
                : std::max(Size(4), Size(xGrids_[0]*std::pow(l[i]/l[0], 0.1)));
            QL_REQUIRE(xGrid >= 4, "minimum grid size is four");

            const Real xStepStize = (1.0-2*eps)/(xGrid-1);

            std::vector<Real> x(xGrid);
            for (Size j=0; j < xGrid; ++j)
                x[j] = 1.3*std::sqrt(l[i])*sqrtT
                    *InverseCumulativeNormal()(eps + j*xStepStize);

            meshers.emplace_back(ext::make_shared<Predefined1dMesher>(x));
        }

        const ext::shared_ptr<FdmMesherComposite> mesher =
            ext::make_shared<FdmMesherComposite>(meshers);

        const ext::shared_ptr<BasketPayoff> payoff
            = ext::dynamic_pointer_cast<BasketPayoff>(arguments_.payoff);
        QL_REQUIRE(payoff, "basket payoff expected");

        const ext::shared_ptr<YieldTermStructure> rTS =
            processes_[0]->riskFreeRate().currentLink();

        std::vector<ext::shared_ptr<YieldTermStructure>> qTS(processes_.size());
        for (Size i=0; i < processes_.size(); ++i)
            qTS[i] = processes_[i]->dividendYield().currentLink();

        const ext::shared_ptr<FdmInnerValueCalculator> calculator =
            ext::make_shared<detail::FdmPCABasketInnerValue>(
                payoff, mesher,
                Log(s), stdDev/sqrtT,
                qTS, rTS,
                Q, l
            );

        const ext::shared_ptr<FdmStepConditionComposite> conditions
            = FdmStepConditionComposite::vanillaComposite(
                DividendSchedule(), arguments_.exercise,
                mesher, calculator,
                rTS->referenceDate(), rTS->dayCounter());

        const FdmBoundaryConditionSet boundaries;
        const FdmSolverDesc solverDesc
            = { mesher, boundaries, conditions, calculator,
                maturity, tGrid_, dampingSteps_ };

        const bool isEuropean =
            ext::dynamic_pointer_cast<EuropeanExercise>(arguments_.exercise) != nullptr;
        const ext::shared_ptr<FdmWienerOp> op =
            ext::make_shared<FdmWienerOp>(
                mesher,
                (isEuropean)? ext::shared_ptr<YieldTermStructure>() : rTS,
                l);

        switch(processes_.size()) {
            #define BOOST_PP_LOCAL_MACRO(n) \
                case n : \
                    results_.value = ext::make_shared<FdmNdimSolver<n>>( \
                        solverDesc, schemeDesc_, op)->interpolateAt( \
                            std::vector<Real>(processes_.size(), 0.0)); \
                break;
            #define BOOST_PP_LOCAL_LIMITS (1, PDE_MAX_SUPPORTED_DIM)
            #include BOOST_PP_LOCAL_ITERATE()
          default:
            QL_FAIL("Not implemented for " << processes_.size() << " processes");
        }

        if (isEuropean)
            results_.value *= pExtractor.getInterestRateDf(maturityDate);
    }
}
