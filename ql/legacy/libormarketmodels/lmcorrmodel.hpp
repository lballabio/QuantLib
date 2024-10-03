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

/*! \file lmcorrmodel.hpp
    \brief correlation model for libor market models
*/

#ifndef quantlib_libor_forward_correlation_model_hpp
#define quantlib_libor_forward_correlation_model_hpp

#include <ql/math/array.hpp>
#include <ql/math/matrix.hpp>
#include <ql/models/parameter.hpp>
#include <ql/utilities/null.hpp>

namespace QuantLib {

    //! %libor forward correlation model
    class LmCorrelationModel {
      public:
        LmCorrelationModel(Size size, Size nArguments);
        virtual ~LmCorrelationModel() = default;

        virtual Size size() const;
        virtual Size factors() const;

        std::vector<Parameter>& params();
        void setParams(const std::vector<Parameter> & arguments);

        virtual Matrix correlation(Time t, const Array& x = {}) const = 0;
        virtual Matrix pseudoSqrt(Time t, const Array& x = {}) const;
        virtual Real correlation(Size i, Size j, Time t, const Array& x = {}) const;
        virtual bool isTimeIndependent() const;

    protected:
        virtual void generateArguments() = 0;

        const Size size_;

        std::vector<Parameter> arguments_;
    };

}


#endif

