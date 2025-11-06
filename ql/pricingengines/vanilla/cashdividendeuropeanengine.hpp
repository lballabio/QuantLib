/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2025 Klaus Spanderen

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file cashdividendeuropeanengine.hpp
    \brief European engine for spot and escrowed cash dividends
*/

#ifndef quantlib_cash_dividend_european_engine_hpp
#define quantlib_cash_dividend_european_engine_hpp

#include <ql/instruments/vanillaoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>


namespace QuantLib {

    //! (Semi)-Analytic pricing engine for European options with cash dividends

	/*! References:

		Jherek Healy, 2021. The Pricing of Vanilla Options with Cash Dividends as
		a Classic Vanilla Basket Option Problem,
		https://arxiv.org/pdf/2106.12971

 	    \ingroup vanillaengines

        \test the correctness of the returned greeks is tested by
              reproducing numerical derivatives.
    */

    class CashDividendEuropeanEngine : public VanillaOption::engine {
      public:
        enum CashDividendModel { Spot, Escrowed };

        CashDividendEuropeanEngine(
            ext::shared_ptr<GeneralizedBlackScholesProcess> process,
            DividendSchedule dividends,
            CashDividendModel cashDividendModel = Spot);

        void calculate() const override;

      private:
        const ext::shared_ptr<GeneralizedBlackScholesProcess> process_;
        const DividendSchedule dividends_;
        const CashDividendModel cashDividendModel_;
    };


}

#endif
