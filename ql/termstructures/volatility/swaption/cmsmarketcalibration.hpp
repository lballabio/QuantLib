/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Marco Bianchetti
 Copyright (C) 2006, 2007 Giorgio Facchinetti
 Copyright (C) 2014 Peter Caspers

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

/*! \file cmsmarketcalibration.hpp
*/

#ifndef quantlib_cms_market_calibration_h
#define quantlib_cms_market_calibration_h

#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/matrix.hpp>
#include <ql/math/array.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class SwaptionVolatilityStructure;
    class CmsMarket;
    class OptimizationMethod;

    class CmsMarketCalibration {
      public:
        enum CalibrationType {OnSpread, OnPrice, OnForwardCmsPrice };

        CmsMarketCalibration(
            Handle<SwaptionVolatilityStructure>& volCube,
            std::shared_ptr<CmsMarket>& cmsMarket,
            const Matrix& weights,
            CalibrationType calibrationType);

        Handle<SwaptionVolatilityStructure> volCube_;
        std::shared_ptr<CmsMarket> cmsMarket_;
        Matrix weights_;
        CalibrationType calibrationType_;
        Matrix sparseSabrParameters_, denseSabrParameters_, browseCmsMarket_;

        Array compute(const std::shared_ptr<EndCriteria>& endCriteria,
                      const std::shared_ptr<OptimizationMethod>& method,
                      const Array& guess,
                      bool isMeanReversionFixed);

        Matrix compute(const std::shared_ptr<EndCriteria>& endCriteria,
                       const std::shared_ptr<OptimizationMethod>& method,
                       const Matrix& guess,
                       bool isMeanReversionFixed,
                       Real meanReversionGuess = Null<Real>());

        Matrix computeParametric(const std::shared_ptr<EndCriteria>& endCriteria,
                                 const std::shared_ptr<OptimizationMethod>& method,
                                 const Matrix& guess,
                                 bool isMeanReversionFixed,
                                 Real meanReversionGuess = Null<Real>());

        Real error() const { return error_; }
        EndCriteria::Type endCriteria() { return endCriteria_; };

        static Real betaTransformInverse(Real beta) {
            return std::sqrt(-std::log(beta));
        }
        static Real betaTransformDirect(Real y) {
            return std::max(
                std::min(std::fabs(y) < 10.0 ? Real(std::exp(-(y * y))) : 0.0,
                         0.999999),
                0.000001);
        }
        static Real reversionTransformInverse(Real reversion) {
            return reversion * reversion;
        }
        static Real reversionTransformDirect(Real y) {
            return std::sqrt(y);
        }

      private:
        Real error_;
        EndCriteria::Type endCriteria_;
    };

}

#endif
