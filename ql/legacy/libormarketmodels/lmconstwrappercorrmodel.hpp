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

/*! \file lmconstwrappercorrmodel.hpp
    \brief const wrapper for correlation model for libor market models
*/

#ifndef quantlib_libor_forward_const_wrapper_correlation_model_hpp
#define quantlib_libor_forward_const_wrapper_correlation_model_hpp

#include <ql/legacy/libormarketmodels/lmcorrmodel.hpp>

namespace QuantLib {

    class LmConstWrapperCorrelationModel : public LmCorrelationModel {
      public:
        LmConstWrapperCorrelationModel(
            const ext::shared_ptr<LmCorrelationModel> & corrModel)
        : LmCorrelationModel(corrModel->size(), 0),
          corrModel_(corrModel) {
        }

        Size factors() const override { return corrModel_->factors(); }

        Disposable<Matrix> correlation(Time t, const Array& x = Null<Array>()) const override {
            return corrModel_->correlation(t, x);
        }
        Disposable<Matrix> pseudoSqrt(Time t, const Array& x = Null<Array>()) const override {
            return corrModel_->pseudoSqrt(t, x);
        }
        Real correlation(Size i, Size j, Time t, const Array& x = Null<Array>()) const override {
            return corrModel_->correlation(i, j, t, x);
        }
        bool isTimeIndependent() const override { return corrModel_->isTimeIndependent(); }

      protected:
        void generateArguments() override {}

        const ext::shared_ptr<LmCorrelationModel> corrModel_;
    };

}


#endif




#ifndef id_063dca4a2886c85ec4f87de286e5ed1c
#define id_063dca4a2886c85ec4f87de286e5ed1c
inline bool test_063dca4a2886c85ec4f87de286e5ed1c(int* i) { return i != 0; }
#endif
