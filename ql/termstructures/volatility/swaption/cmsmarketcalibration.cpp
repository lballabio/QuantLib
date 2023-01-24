/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Cpoyright (C) 2014 Peter Caspers

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

#include <ql/termstructures/volatility/swaption/cmsmarketcalibration.hpp>
#include <ql/termstructures/volatility/swaption/cmsmarket.hpp>
#include <ql/termstructures/volatility/swaption/sabrswaptionvolatilitycube.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace {
    using namespace QuantLib;

    class ObjectiveFunction : public CostFunction {
      public:
        explicit ObjectiveFunction(CmsMarketCalibration *smileAndCms)
            : smileAndCms_(smileAndCms), volCube_(smileAndCms->volCube_),
              cmsMarket_(smileAndCms->cmsMarket_),
              weights_(smileAndCms->weights_),
              calibrationType_(smileAndCms->calibrationType_) {};

        Real value(const Array& x) const override;
        Array values(const Array& x) const override;

      protected:
        Real switchErrorFunctionOnCalibrationType() const;
        Array switchErrorsFunctionOnCalibrationType() const;

        CmsMarketCalibration *smileAndCms_;
        Handle<SwaptionVolatilityStructure> volCube_;
        ext::shared_ptr<CmsMarket> cmsMarket_;
        Matrix weights_;
        CmsMarketCalibration::CalibrationType calibrationType_;

      private:
        virtual void updateVolatilityCubeAndCmsMarket(const Array &x) const;
    };

    class ObjectiveFunction2 : public ObjectiveFunction {
      public:
        ObjectiveFunction2(CmsMarketCalibration *smileAndCms,
                           Real fixedMeanReversion)
            : ObjectiveFunction(smileAndCms),
              fixedMeanReversion_(fixedMeanReversion) {};

      private:
        void updateVolatilityCubeAndCmsMarket(const Array& x) const override;
        Real fixedMeanReversion_;
    };

    class ObjectiveFunction3 : public ObjectiveFunction {
      public:
        explicit ObjectiveFunction3(CmsMarketCalibration *smileAndCms)
            : ObjectiveFunction(smileAndCms) {};

      private:
        void updateVolatilityCubeAndCmsMarket(const Array& x) const override;
    };

    class ObjectiveFunction4 : public ObjectiveFunction {
      public:
        ObjectiveFunction4(CmsMarketCalibration *smileAndCms,
                           Real fixedMeanReversion)
            : ObjectiveFunction(smileAndCms),
              fixedMeanReversion_(fixedMeanReversion) {};

      private:
        void updateVolatilityCubeAndCmsMarket(const Array& x) const override;
        Real fixedMeanReversion_;
    };

    class ObjectiveFunction5 : public ObjectiveFunction {
      public:
        ObjectiveFunction5(CmsMarketCalibration *smileAndCms,
                           Real fixedMeanReversion)
            : ObjectiveFunction(smileAndCms),
              fixedMeanReversion_(fixedMeanReversion) {};

      private:
        void updateVolatilityCubeAndCmsMarket(const Array& x) const override;
        Real fixedMeanReversion_;
    };

    class ObjectiveFunction6 : public ObjectiveFunction {
      public:
        explicit ObjectiveFunction6(CmsMarketCalibration *smileAndCms)
            : ObjectiveFunction(smileAndCms) {};

      private:
        void updateVolatilityCubeAndCmsMarket(const Array& x) const override;
    };

    //===========================================================================//
    //        ObjectiveFunction (constant beta, free mean reversion)             //
    //===========================================================================//

    Real ObjectiveFunction::value(const Array &x) const {
        updateVolatilityCubeAndCmsMarket(x);
        return switchErrorFunctionOnCalibrationType();
    }

    Array ObjectiveFunction::values(const Array &x) const {
        updateVolatilityCubeAndCmsMarket(x);
        return switchErrorsFunctionOnCalibrationType();
    }

    void
    ObjectiveFunction::updateVolatilityCubeAndCmsMarket(const Array &x) const {
        const std::vector<Period> &swapTenors = cmsMarket_->swapTenors();
        Size nSwapTenors = swapTenors.size();
        QL_REQUIRE(nSwapTenors + 1 == x.size(),
                   "bad calibration guess nSwapTenors+1 != x.size()");
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        for (Size i = 0; i < nSwapTenors; ++i)
            volCubeBySabr->recalibration(CmsMarketCalibration::betaTransformDirect(x[i]),
                                         swapTenors[i]);
        Real meanReversion =
            CmsMarketCalibration::reversionTransformDirect(x[nSwapTenors]);
        cmsMarket_->reprice(volCube_, meanReversion);
    }

    Real ObjectiveFunction::switchErrorFunctionOnCalibrationType() const {
        switch (calibrationType_) {
        case CmsMarketCalibration::OnSpread:
            return cmsMarket_->weightedSpreadError(weights_);
        case CmsMarketCalibration::OnPrice:
            return cmsMarket_->weightedSpotNpvError(weights_);
        case CmsMarketCalibration::OnForwardCmsPrice:
            return cmsMarket_->weightedFwdNpvError(weights_);
        default:
            QL_FAIL("unknown/illegal calibration type");
        }
    }

    Array ObjectiveFunction::switchErrorsFunctionOnCalibrationType() const {
        switch (calibrationType_) {
        case CmsMarketCalibration::OnSpread:
            return cmsMarket_->weightedSpreadErrors(weights_);
        case CmsMarketCalibration::OnPrice:
            return cmsMarket_->weightedSpotNpvErrors(weights_);
        case CmsMarketCalibration::OnForwardCmsPrice:
            return cmsMarket_->weightedFwdNpvErrors(weights_);
        default:
            QL_FAIL("unknown/illegal calibration type");
        }
    }

    //===========================================================================//
    //        ObjectiveFunction2 (constant beta, fixed mean reversion)           //
    //===========================================================================//

    void
    ObjectiveFunction2::updateVolatilityCubeAndCmsMarket(const Array &x) const {
        const std::vector<Period> &swapTenors = cmsMarket_->swapTenors();
        Size nSwapTenors = swapTenors.size();
        QL_REQUIRE(nSwapTenors == x.size(),
                   "bad calibration guess nSwapTenors != x.size()");
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        for (Size i = 0; i < nSwapTenors; ++i)
            volCubeBySabr->recalibration(QuantLib::CmsMarketCalibration::betaTransformDirect(x[i]),
                                         swapTenors[i]);
        cmsMarket_->reprice(volCube_, fixedMeanReversion_ == Null<Real>() ?
                                          Null<Real>() :
                                          QuantLib::CmsMarketCalibration::reversionTransformDirect(
                                              fixedMeanReversion_));
    }

    //===========================================================================//
    //        ObjectiveFunction3 (beta termstructure, free mean reversion)       //
    //===========================================================================//

    void
    ObjectiveFunction3::updateVolatilityCubeAndCmsMarket(const Array &x) const {
        const std::vector<Period> &swapTenors = cmsMarket_->swapTenors();
        const std::vector<Period> &swapLengths = cmsMarket_->swapLengths();
        Size nSwapTenors = swapTenors.size();
        Size nSwapLengths = swapLengths.size();
        QL_REQUIRE(
            (nSwapLengths * nSwapTenors) + 1 == x.size(),
            "bad calibration guess (nSwapLengths*nSwapTenors)+1 != x.size()");
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        for (Size i = 0; i < nSwapTenors; ++i) {
            std::vector<Real> beta(x.begin() + (i * nSwapLengths),
                                   x.begin() + ((i + 1) * nSwapLengths));
            for (Real& j : beta)
                j = CmsMarketCalibration::betaTransformDirect(j);
            volCubeBySabr->recalibration(swapLengths, beta, swapTenors[i]);
        }
        Real meanReversion =
            CmsMarketCalibration::reversionTransformDirect(x[nSwapLengths + nSwapTenors]);
        cmsMarket_->reprice(volCube_, meanReversion);
    }

    //===========================================================================//
    //        ObjectiveFunction4 (beta termstructure, fixed mean reversion)      //
    //===========================================================================//

    void
    ObjectiveFunction4::updateVolatilityCubeAndCmsMarket(const Array &x) const {
        const std::vector<Period> &swapTenors = cmsMarket_->swapTenors();
        const std::vector<Period> &swapLengths = cmsMarket_->swapLengths();
        Size nSwapTenors = swapTenors.size();
        Size nSwapLengths = swapLengths.size();
        QL_REQUIRE(
            (nSwapLengths * nSwapTenors) == x.size(),
            "bad calibration guess (nSwapLengths*nSwapTenors) != x.size()");
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        for (Size i = 0; i < nSwapTenors; ++i) {
            std::vector<Real> beta(x.begin() + (i * nSwapLengths),
                                   x.begin() + ((i + 1) * nSwapLengths));
            for (Real& j : beta)
                j = CmsMarketCalibration::betaTransformDirect(j);
            volCubeBySabr->recalibration(swapLengths, beta, swapTenors[i]);
        }
        cmsMarket_->reprice(volCube_, fixedMeanReversion_ == Null<Real>() ?
                                          Null<Real>() :
                                          CmsMarketCalibration::reversionTransformDirect(
                                              fixedMeanReversion_));
    }

    //=============================================================================//
    // ObjectiveFunction5 (beta parameteric termstructure, fixed mean reversion)   //
    //=============================================================================//

    void
    ObjectiveFunction5::updateVolatilityCubeAndCmsMarket(const Array &x) const {
        const std::vector<Period> &swapTenors = cmsMarket_->swapTenors();
        const std::vector<Period> &swapLengths = cmsMarket_->swapLengths();
        Size nSwapTenors = swapTenors.size();
        Size nSwapLengths = swapLengths.size();
        QL_REQUIRE((3 * nSwapTenors) == x.size(),
                   "bad calibration guess (3*nSwapTenors) != x.size()");
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        for (Size i = 0; i < nSwapTenors; ++i) {
            Real betaInf = CmsMarketCalibration::betaTransformDirect(x[0 + 3 * i]);
            Real beta0 = CmsMarketCalibration::betaTransformDirect(x[1 + 3 * i]);
            Real decay = x[2 + 3 * i] * x[2 + 3 * i];
            std::vector<Real> beta(nSwapLengths);
            for (Size j = 0; j < beta.size(); ++j) {
                Real t = smileAndCms_->volCube_->timeFromReference(
                    smileAndCms_->volCube_->optionDateFromTenor(swapLengths[j]));
                beta[j] = betaInf + (beta0 - betaInf) * std::exp(-decay * t);
            }
            volCubeBySabr->recalibration(swapLengths, beta, swapTenors[i]);
        }
        cmsMarket_->reprice(volCube_, fixedMeanReversion_ == Null<Real>() ?
                                          Null<Real>() :
                                          CmsMarketCalibration::reversionTransformDirect(
                                              fixedMeanReversion_));
    }

    //===========================================================================//
    // ObjectiveFunction6 (beta parameteric termstructure, free mean reversion)  //
    //===========================================================================//

    void
    ObjectiveFunction6::updateVolatilityCubeAndCmsMarket(const Array &x) const {
        const std::vector<Period> &swapTenors = cmsMarket_->swapTenors();
        const std::vector<Period> &swapLengths = cmsMarket_->swapLengths();
        Size nSwapTenors = swapTenors.size();
        Size nSwapLengths = swapLengths.size();
        QL_REQUIRE((3 * nSwapTenors) == x.size(),
                   "bad calibration guess (3*nSwapTenors) != x.size()");
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        for (Size i = 0; i < nSwapTenors; ++i) {
            Real betaInf = CmsMarketCalibration::betaTransformDirect(x[0 + 3 * i]);
            Real beta0 = CmsMarketCalibration::betaTransformDirect(x[1 + 3 * i]);
            Real decay = x[2 + 3 * i] * x[2 + 3 * i];
            std::vector<Real> beta(nSwapLengths);
            for (Size j = 0; j < beta.size(); ++j) {
                Real t = smileAndCms_->volCube_->timeFromReference(
                    smileAndCms_->volCube_->optionDateFromTenor(swapLengths[j]));
                beta[j] = betaInf + (beta0 - betaInf) * std::exp(-decay * t);
            }
            volCubeBySabr->recalibration(swapLengths, beta, swapTenors[i]);
        }
        Real meanReversion =
            CmsMarketCalibration::reversionTransformDirect(x[3 * nSwapTenors]);
        cmsMarket_->reprice(volCube_, meanReversion);
    }
}

namespace QuantLib {

    //===========================================================================//
    //                       CmsMarketCalibration                                //
    //===========================================================================//

    CmsMarketCalibration::CmsMarketCalibration(
        Handle<SwaptionVolatilityStructure> &volCube,
        ext::shared_ptr<CmsMarket> &cmsMarket, const Matrix &weights,
        CalibrationType calibrationType)
        : volCube_(volCube), cmsMarket_(cmsMarket), weights_(weights),
          calibrationType_(calibrationType) {

        QL_REQUIRE(weights.rows() == cmsMarket_->swapLengths().size(),
                   "weights number of rows ("
                       << weights.rows()
                       << ") must be equal to number of swap lengths ("
                       << cmsMarket_->swapLengths().size() << ")");
        QL_REQUIRE(weights.columns() == cmsMarket_->swapTenors().size(),
                   "weights number of columns ("
                       << weights.columns()
                       << ") must be equal to number of swap indexes ("
                       << cmsMarket_->swapTenors().size());
    }

    Array CmsMarketCalibration::compute(
        const ext::shared_ptr<EndCriteria> &endCriteria,
        const ext::shared_ptr<OptimizationMethod> &method, const Array &guess,
        bool isMeanReversionFixed) {
        Size nSwapTenors = cmsMarket_->swapTenors().size();
        QL_REQUIRE(isMeanReversionFixed || guess.size() == nSwapTenors + 1,
                   "if mean reversion is not fixed, a guess must be provided");
        QL_REQUIRE(nSwapTenors == guess.size() ||
                       nSwapTenors == guess.size() - 1,
                   "guess size (" << guess.size()
                                  << ") must be equal to swap tenors size ("
                                  << nSwapTenors
                                  << ") or greater by one if mean reversion is "
                                     "given as last element");
        bool isMeanReversionGiven = (nSwapTenors == guess.size() - 1);
        Size nBeta = guess.size() - (isMeanReversionGiven ? 1 : 0);
        Array result;
        if (isMeanReversionFixed) {
            NoConstraint constraint;
            Real fixedMeanReversion =
                isMeanReversionGiven ? guess[nBeta] : Null<Real>();
            Array betasGuess(nBeta);
            for (Size i = 0; i < nBeta; ++i)
                betasGuess[i] = guess[i];
            ObjectiveFunction2 costFunction(
                this, fixedMeanReversion == Null<Real>()
                          ? Null<Real>()
                          : reversionTransformInverse(fixedMeanReversion));
            Problem problem(costFunction, constraint, betasGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            Array tmp = problem.currentValue();
            error_ = costFunction.value(tmp);
            result = Array(nBeta + (isMeanReversionGiven ? 1 : 0));
            for (Size i = 0; i < nBeta; ++i)
                result[i] = betaTransformDirect(tmp[i]);
            if (isMeanReversionGiven)
                result[nBeta] = fixedMeanReversion;
        } else {
            NoConstraint constraint;
            ObjectiveFunction costFunction(this);
            Array betaReversionGuess(nBeta + 1);
            for (Size i = 0; i < nBeta; ++i)
                betaReversionGuess[i] = betaTransformInverse(guess[i]);
            betaReversionGuess[nBeta] = reversionTransformInverse(guess[nBeta]);
            Problem problem(costFunction, constraint, betaReversionGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            result = problem.currentValue();
            error_ = costFunction.value(result);
            for (Size i = 0; i < nBeta; ++i)
                result[i] = betaTransformDirect(result[i]);
            result[nBeta] = reversionTransformDirect(result[nBeta]);
        }
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        volCubeBySabr->updateAfterRecalibration();
        sparseSabrParameters_ = volCubeBySabr->sparseSabrParameters();
        denseSabrParameters_ = volCubeBySabr->denseSabrParameters();
        browseCmsMarket_ = cmsMarket_->browse();

        return result;
    }

    Matrix CmsMarketCalibration::compute(
        const ext::shared_ptr<EndCriteria> &endCriteria,
        const ext::shared_ptr<OptimizationMethod> &method,
        const Matrix &guess, bool isMeanReversionFixed,
        const Real meanReversionGuess) {
        Size nSwapTenors = cmsMarket_->swapTenors().size();
        Size nSwapLengths = cmsMarket_->swapLengths().size();
        QL_REQUIRE(isMeanReversionFixed || meanReversionGuess != Null<Real>(),
                   "if mean reversion is not fixed, a guess must be provided");
        QL_REQUIRE(nSwapTenors == guess.columns(),
                   "number of swap tenors ("
                       << nSwapTenors
                       << ") must be equal to number of guess columns ("
                       << guess.columns() << ")");
        QL_REQUIRE(nSwapLengths == guess.rows(),
                   "number of swap lengths ("
                       << nSwapLengths
                       << ") must be equal to number of guess rows ("
                       << guess.rows() << ")");
        Matrix result;
        Size nBeta = nSwapTenors * nSwapLengths;
        if (isMeanReversionFixed) {
            NoConstraint constraint;
            Array betasGuess(nBeta);
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < nSwapLengths; ++j) {
                    betasGuess[i * nSwapLengths + j] =
                        betaTransformInverse(guess[j][i]);
                }
            }
            ObjectiveFunction4 costFunction(
                this, meanReversionGuess == Null<Real>()
                          ? meanReversionGuess
                          : reversionTransformInverse(meanReversionGuess));
            Problem problem(costFunction, constraint, betasGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            Array tmp = problem.currentValue();
            error_ = costFunction.value(tmp);
            result = Matrix(nSwapLengths,
                            nSwapTenors +
                                (meanReversionGuess != Null<Real>() ? 1 : 0));
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < nSwapLengths; ++j) {
                    result[j][i] =
                        betaTransformDirect(tmp[i * nSwapLengths + j]);
                }
            }
            if (meanReversionGuess != Null<Real>()) {
                for (Size j = 0; j < nSwapLengths; ++j) {
                    result[j][nSwapTenors] = meanReversionGuess;
                }
            }
        } else {
            NoConstraint constraint;
            Array betasReversionGuess(nBeta + 1);
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < nSwapLengths; ++j) {
                    betasReversionGuess[i * nSwapLengths + j] =
                        betaTransformInverse(guess[j][i]);
                }
            }
            betasReversionGuess[nBeta] =
                reversionTransformInverse(meanReversionGuess);
            ObjectiveFunction3 costFunction(this);
            Problem problem(costFunction, constraint, betasReversionGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            Array tmp = problem.currentValue();
            error_ = costFunction.value(tmp);
            result = Matrix(nSwapLengths, nSwapTenors + 1);
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < nSwapLengths; ++j) {
                    result[j][i] =
                        betaTransformDirect(tmp[i * nSwapLengths + j]);
                }
            }
            for (Size j = 0; j < nSwapLengths; ++j) {
                result[j][nSwapTenors] = reversionTransformDirect(tmp[nBeta]);
            }
        }
        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        volCubeBySabr->updateAfterRecalibration();
        sparseSabrParameters_ = volCubeBySabr->sparseSabrParameters();
        denseSabrParameters_ = volCubeBySabr->denseSabrParameters();
        browseCmsMarket_ = cmsMarket_->browse();

        return result;
    }

    Matrix CmsMarketCalibration::computeParametric(
        const ext::shared_ptr<EndCriteria> &endCriteria,
        const ext::shared_ptr<OptimizationMethod> &method,
        const Matrix &guess, bool isMeanReversionFixed,
        const Real meanReversionGuess) {

        Size nSwapTenors = cmsMarket_->swapTenors().size();
        Size nSwapLengths = cmsMarket_->swapLengths().size();
        QL_REQUIRE(isMeanReversionFixed || meanReversionGuess != Null<Real>(),
                   "if mean reversion is not fixed, a guess must be provided");
        QL_REQUIRE(nSwapTenors == guess.columns(),
                   "number of swap tenors ("
                       << nSwapTenors
                       << ") must be equal to number of guess columns ("
                       << guess.columns() << ")");
        QL_REQUIRE(3 == guess.rows(),
                   "number of parameters ("
                       << 3 << ") must be equal to number of guess rows ("
                       << guess.rows() << ")");

        Matrix result;
        Size nParams = nSwapTenors * 3;
        if (isMeanReversionFixed) {
            NoConstraint constraint;
            Array betasGuess(nParams);
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < nParams; ++j) {
                    betasGuess[i * 3 + j] =
                        (j == 0 || j == 1) ? betaTransformInverse(guess[j][i])
                                           : std::sqrt(guess[j][i]);
                }
            }
            ObjectiveFunction5 costFunction(
                this, meanReversionGuess == Null<Real>()
                          ? meanReversionGuess
                          : reversionTransformInverse(meanReversionGuess));
            Problem problem(costFunction, constraint, betasGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            Array tmp = problem.currentValue();
            error_ = costFunction.value(tmp);
            result = Matrix(
                3, nSwapTenors + (meanReversionGuess != Null<Real>() ? 1 : 0));
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < 3; ++j) {
                    result[j][i] = (j == 0 || j == 1)
                                       ? betaTransformDirect(tmp[i * 3 + j])
                                       : tmp[i * 3 + j] * tmp[i * 3 + j];
                }
            }
            if (meanReversionGuess != Null<Real>()) {
                for (Size j = 0; j < nSwapLengths; ++j) {
                    result[j][nSwapTenors] = meanReversionGuess;
                }
            }
        } else {
            NoConstraint constraint;
            Array betasReversionGuess(nParams + 1);
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < nParams; ++j) {
                    betasReversionGuess[i * nSwapLengths + j] =
                        (j == 0 || j == 1) ? betaTransformInverse(guess[j][i])
                                           : std::sqrt(guess[j][i]);
                }
            }
            betasReversionGuess[nParams] =
                reversionTransformInverse(meanReversionGuess);
            ObjectiveFunction6 costFunction(this);
            Problem problem(costFunction, constraint, betasReversionGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            Array tmp = problem.currentValue();
            error_ = costFunction.value(tmp);
            result = Matrix(3, nSwapTenors + 1);
            for (Size i = 0; i < nSwapTenors; ++i) {
                for (Size j = 0; j < 3; ++j) {
                    result[j][i] =
                        (j == 0 || j == 1)
                            ? betaTransformDirect(tmp[i * nSwapLengths + j])
                            : tmp[i * 3 + j] * tmp[i * 3 + j];
                }
            }
            for (Size j = 0; j < nSwapLengths; ++j) {
                result[j][nSwapTenors] = reversionTransformDirect(tmp[nParams]);
            }
        }

        const ext::shared_ptr<SabrSwaptionVolatilityCube> volCubeBySabr =
            ext::dynamic_pointer_cast<SabrSwaptionVolatilityCube>(*volCube_);
        volCubeBySabr->updateAfterRecalibration();
        sparseSabrParameters_ = volCubeBySabr->sparseSabrParameters();
        denseSabrParameters_ = volCubeBySabr->denseSabrParameters();
        browseCmsMarket_ = cmsMarket_->browse();

        return result;
    }
}
