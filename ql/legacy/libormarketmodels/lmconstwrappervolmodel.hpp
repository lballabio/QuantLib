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

/*! \file lmconstwrappervolmodel.hpp
    \brief const wrapper for a volatility model for libor market models
*/

#ifndef quantlib_libor_market_const_wrapper_volatility_model_hpp
#define quantlib_libor_market_const_wrapper_volatility_model_hpp

#include <ql/legacy/libormarketmodels/lmvolmodel.hpp>

namespace QuantLib {

    //! caplet const volatility model
    class LmConstWrapperVolatilityModel : public LmVolatilityModel {
      public:
        explicit LmConstWrapperVolatilityModel(
            const ext::shared_ptr<LmVolatilityModel> & volaModel)
        : LmVolatilityModel(volaModel->size(), 0),
          volaModel_(volaModel) {
        }

        Array volatility(Time t, const Array& x = {}) const override {
            return volaModel_->volatility(t, x);
        }
        Volatility volatility(
            Size i, Time t, const Array& x = {}) {
            return volaModel_->volatility(i, t, x);
        }
        Real integratedVariance(Size i, Size j, Time u, const Array& x = {}) const override {
            return volaModel_->integratedVariance(i, j, u, x);
        }

      protected:
        const ext::shared_ptr<LmVolatilityModel> volaModel_;

      private:
        using LmVolatilityModel::volatility;
        void generateArguments() override {}
    };

}


#endif


