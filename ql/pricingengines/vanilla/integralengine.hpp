/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2002, 2003, 2004 Ferdinando Ametrano
 Copyright (C) 2002, 2003 RiskMap srl
 Copyright (C) 2003, 2004 StatPro Italia srl

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

/*! \file integralengine.hpp
    \brief Integral option engine
*/

#ifndef quantlib_integral_engine_hpp
#define quantlib_integral_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Pricing engine for European vanilla options using integral approach
    /*! \todo define tolerance for calculate()

        \ingroup vanillaengines
    */
    class IntegralEngine : public VanillaOption::engine {
      public:
        IntegralEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif


#ifndef id_d4bb585bdbdae921d9b49145187f2b5e
#define id_d4bb585bdbdae921d9b49145187f2b5e
inline bool test_d4bb585bdbdae921d9b49145187f2b5e(const int* i) {
    return i != nullptr;
}
#endif
