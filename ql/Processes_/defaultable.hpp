/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/reference/license.html>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file defaultable.hpp
    \brief Defaultable processes
*/

#ifndef quantlib_defaultable_hpp
#define quantlib_defaultable_hpp

#include <ql/types.hpp>

namespace QuantLib {
    class  Defaultable {
      public:
        virtual ~Defaultable() {};
        virtual Real hazardRate(Time t, Real underlying) const = 0;
        virtual Real defaultRecovery(Time t, Real underlying) const = 0;
    };

    class NegativePowerDefaultIntensity : public Defaultable {
    private:
        Real alpha_, p_, recovery_;
    public:
        NegativePowerDefaultIntensity(Real alpha, Real p,
                               Real recovery = 0.0) {
            alpha_ = alpha; p_ = p;
            recovery_ = recovery;
        }
        Real hazardRate(Time t, Real s) {
            if (s <= 0.0) return 0.0;
            return alpha_ * std::pow(s, -p_);
        }
        Real defaultRecovery(Time t, Real s) {
            return recovery_;
        }
    };

    class ConstantDefaultIntensity : public Defaultable {
    private:
        Real constant_, recovery_;
    public:
        ConstantDefaultIntensity(Real constant,
                                 Real recovery = 0.0) {
            constant_ = constant;
            recovery_ = recovery;
        }
        Real hazardRate(Time t, Real s) {
            return constant_;
        }
        Real defaultRecovery(Time t, Real s) {
            return recovery_;
        }
    };
}

#endif
