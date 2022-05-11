/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006 Klaus Spanderen

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

/*! \file lfmcovarproxy.hpp
    \brief proxy for libor forward covariance parameterization
*/

#ifndef quantlib_libor_forward_market_covariance_proxy_hpp
#define quantlib_libor_forward_market_covariance_proxy_hpp

#include <ql/legacy/libormarketmodels/lfmcovarparam.hpp>
#include <ql/legacy/libormarketmodels/lmvolmodel.hpp>
#include <ql/legacy/libormarketmodels/lmcorrmodel.hpp>

namespace QuantLib {

    //! proxy for a libor forward model covariance parameterization

    class LfmCovarianceProxy : public LfmCovarianceParameterization {
      public:
        LfmCovarianceProxy(ext::shared_ptr<LmVolatilityModel> volaModel,
                           const ext::shared_ptr<LmCorrelationModel>& corrModel);

        ext::shared_ptr<LmVolatilityModel>  volatilityModel() const;
        ext::shared_ptr<LmCorrelationModel> correlationModel() const;

        Matrix diffusion(Time t, const Array& x = Null<Array>()) const override;
        Matrix covariance(Time t, const Array& x = Null<Array>()) const override;

        using LfmCovarianceParameterization::integratedCovariance;
        virtual Real integratedCovariance(
            Size i, Size j, Time t, const Array& x = Null<Array>()) const;

      protected:
        const ext::shared_ptr<LmVolatilityModel>  volaModel_;
        const ext::shared_ptr<LmCorrelationModel> corrModel_;

      private:
        class Var_Helper;
        friend class Var_Helper;
    };

}


#endif

