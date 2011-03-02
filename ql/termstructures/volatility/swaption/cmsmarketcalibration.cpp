/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2006, 2007 Giorgio Facchinetti

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
#include <ql/termstructures/volatility/swaption/swaptionvolcube1.hpp>
#include <ql/math/optimization/problem.hpp>
#include <ql/math/optimization/constraint.hpp>

namespace {
    using namespace QuantLib;
    class ParametersConstraint : public Constraint {
      private:
        class Impl : public Constraint::Impl {
            Size nBeta_;
          public:
            Impl(Size nBeta) : Constraint::Impl(), nBeta_(nBeta) {}

            bool test(const Array& params) const {
                QL_REQUIRE(params.size()==nBeta_+1,
                           "params.size()!=nBeta_+1");
                for (Size i=0; i<nBeta_; ++i)
                    if (params[i]<0.0 || params[i]>1.0)
                        return false;

                return params[nBeta_]>0.0; // mean reversion
            }
        };
      public:
        ParametersConstraint(Size nBeta)
        : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl(nBeta))) {}
    };

    class ObjectiveFunction : public CostFunction {
      public:
        ObjectiveFunction(CmsMarketCalibration* smileAndCms)
        : smileAndCms_(smileAndCms),
          volCube_(smileAndCms->volCube_),
          cmsMarket_(smileAndCms->cmsMarket_),
          weights_(smileAndCms->weights_),
          calibrationType_(smileAndCms->calibrationType_) {};

        Real value(const Array& x) const;
        Disposable<Array> values(const Array& x) const;

      protected:
        Real switchErrorFunctionOnCalibrationType() const;
        Disposable<Array> switchErrorsFunctionOnCalibrationType() const;

        CmsMarketCalibration* smileAndCms_;
        Handle<SwaptionVolatilityStructure> volCube_;
        boost::shared_ptr<CmsMarket> cmsMarket_;
        Matrix weights_;
        CmsMarketCalibration::CalibrationType calibrationType_;
      private:
        virtual void updateVolatilityCubeAndCmsMarket(const Array& x) const;
    };

    class ParametersConstraint2 : public Constraint {
      private:
        class Impl : public Constraint::Impl {
            Size nBeta_;
          public:
            Impl(Size nBeta) : Constraint::Impl(), nBeta_(nBeta) {}

            bool test(const Array& params) const {
                QL_REQUIRE(params.size()==nBeta_,
                           "params.size()!=nBeta_");
                for (Size i=0; i<nBeta_; ++i)
                    if (params[i]<0.0 || params[i]>1.0)
                        return false;
                return true;
            }
        };
      public:
        ParametersConstraint2(Size nBeta)
        : Constraint(boost::shared_ptr<Constraint::Impl>(new Impl(nBeta))) {}
    };

    class ObjectiveFunction2 : public ObjectiveFunction {
      public:
        ObjectiveFunction2(CmsMarketCalibration* smileAndCms,
                                                Real fixedMeanReversion)
        : ObjectiveFunction(smileAndCms),
          fixedMeanReversion_(fixedMeanReversion){};

      private:
        virtual void updateVolatilityCubeAndCmsMarket(const Array& x) const;
        Real fixedMeanReversion_;
    };


    //===========================================================================//
    //                              ObjectiveFunction                            //
    //===========================================================================//

    Real ObjectiveFunction::value(const Array& x) const {
        updateVolatilityCubeAndCmsMarket(x);
        return switchErrorFunctionOnCalibrationType();
    }

    Disposable<Array> ObjectiveFunction::values(const Array& x) const {
        updateVolatilityCubeAndCmsMarket(x);
        return switchErrorsFunctionOnCalibrationType();
    }

    void
    ObjectiveFunction::updateVolatilityCubeAndCmsMarket(const Array& x) const {
        const std::vector<Period>& swapTenors = cmsMarket_->swapTenors();
        Size nSwapTenors = swapTenors.size();
        QL_REQUIRE(nSwapTenors+1 == x.size(),
                   "bad calibration guess nSwapTenors+1 != x.size()");
        const boost::shared_ptr<SwaptionVolCube1> volCubeBySabr =
               boost::dynamic_pointer_cast<SwaptionVolCube1>(*volCube_);
        for (Size i=0; i<nSwapTenors; ++i)
            volCubeBySabr->recalibration(x[i], swapTenors[i]);
        Real meanReversion = x[nSwapTenors];
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

    Disposable<Array>
    ObjectiveFunction::switchErrorsFunctionOnCalibrationType() const {
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
    //                      ObjectiveFunction2              //
    //===========================================================================//

    void
    ObjectiveFunction2::updateVolatilityCubeAndCmsMarket(
                                                        const Array& x) const {
        const std::vector<Period>& swapTenors = cmsMarket_->swapTenors();
        Size nSwapTenors = swapTenors.size();
        QL_REQUIRE(nSwapTenors == x.size(),
                   "bad calibration guess nSwapTenors != x.size()");
        const boost::shared_ptr<SwaptionVolCube1> volCubeBySabr =
               boost::dynamic_pointer_cast<SwaptionVolCube1>(*volCube_);
        for (Size i=0; i<nSwapTenors; ++i)
            volCubeBySabr->recalibration(x[i], swapTenors[i]);
        cmsMarket_->reprice(volCube_, fixedMeanReversion_);
    }
}

namespace QuantLib {

    //===========================================================================//
    //                       CmsMarketCalibration                                //
    //===========================================================================//

    CmsMarketCalibration::CmsMarketCalibration(
                                Handle<SwaptionVolatilityStructure>& volCube,
                                boost::shared_ptr<CmsMarket>& cmsMarket,
                                const Matrix& weights,
                                CalibrationType calibrationType)
    : volCube_(volCube),
      cmsMarket_(cmsMarket),
      weights_(weights),
      calibrationType_(calibrationType) {}

    Array CmsMarketCalibration::compute(
                        const boost::shared_ptr<EndCriteria>& endCriteria,
                        const boost::shared_ptr<OptimizationMethod>& method,
                        const Array& guess,
                        bool isMeanReversionFixed) {
        Array result;
        if (isMeanReversionFixed) {
            Size nBeta = guess.size()-1;
            ParametersConstraint2 constraint(nBeta);
            Real fixedMeanReversion = guess[nBeta];
            Array betasGuess(nBeta);
            for (Size i=0; i<nBeta; ++i)
                betasGuess[i] = guess[i];
            ObjectiveFunction2 costFunction(this, fixedMeanReversion);
            Problem problem(costFunction, constraint, betasGuess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            Array tmp = problem.currentValue();
            result = Array(nBeta+1);
            for (Size i=0; i<nBeta; ++i)
                result[i] = tmp[i];
            result[nBeta] = fixedMeanReversion;
            error_ = costFunction.value(tmp);
        } else {
            ParametersConstraint constraint(guess.size()-1);
            ObjectiveFunction costFunction(this);
            Problem problem(costFunction, constraint, guess);
            endCriteria_ = method->minimize(problem, *endCriteria);
            result = problem.currentValue();
            error_ = costFunction.value(result);
        }
        const boost::shared_ptr<SwaptionVolCube1> volCubeBySabr =
            boost::dynamic_pointer_cast<SwaptionVolCube1>(*volCube_);
        sparseSabrParameters_ = volCubeBySabr->sparseSabrParameters();
        denseSabrParameters_ = volCubeBySabr->denseSabrParameters();
        browseCmsMarket_ = cmsMarket_->browse();

        return result;
    }

}
