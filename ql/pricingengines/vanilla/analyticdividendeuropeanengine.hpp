/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2004, 2007 StatPro Italia srl

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

/*! \file analyticdividendeuropeanengine.hpp
    \brief Analytic discrete-dividend European engine
*/

#ifndef quantlib_analytic_dividend_european_engine_hpp
#define quantlib_analytic_dividend_european_engine_hpp

#include <ql/instruments/dividendvanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic pricing engine for European options with discrete dividends
    /*! \ingroup vanillaengines

        \test the correctness of the returned greeks is tested by
              reproducing numerical derivatives.
    */
    class AnalyticDividendEuropeanEngine
        : public DividendVanillaOption::engine {
      public:
        AnalyticDividendEuropeanEngine(ext::shared_ptr<GeneralizedBlackScholesProcess>);
        void calculate() const override;

      private:
        ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
    };

}


#endif


#ifndef id_ce50f36b3373f580cb6443f5c579a140
#define id_ce50f36b3373f580cb6443f5c579a140
inline bool test_ce50f36b3373f580cb6443f5c579a140(int* i) { return i != 0; }
#endif
