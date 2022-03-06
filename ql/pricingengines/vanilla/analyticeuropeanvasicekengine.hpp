/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2020 Lew Wei Hao

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

#ifndef quantlib_analytic_black_vasicek_engine_hpp
#define quantlib_analytic_black_vasicek_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>
#include <ql/models/shortrate/onefactormodels/vasicek.hpp>
#include <ql/math/integrals/integral.hpp>

namespace QuantLib {

    /**
     *
     * Pricing of Vanilla European options under stochastic Vasicek interest rate model
     * Analytical solution is based on following research paper:
     *
     * http://hsrm-mathematik.de/WS201516/master/option-pricing/Black-Scholes-Vasicek-Model.pdf
     */

    class AnalyticBlackVasicekEngine : public VanillaOption::engine {
      public:
        AnalyticBlackVasicekEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>,
                                   ext::shared_ptr<Vasicek>,
                                   Real correlation);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> blackProcess_;
        ext::shared_ptr<Vasicek> vasicekProcess_;
        ext::shared_ptr<Integrator> simpsonIntegral_;
        Real correlation_;
    };

}

#endif


#ifndef id_83eed8cfa9cfd1f3bb364e661a79a809
#define id_83eed8cfa9cfd1f3bb364e661a79a809
inline bool test_83eed8cfa9cfd1f3bb364e661a79a809(const int* i) {
    return i != nullptr;
}
#endif
