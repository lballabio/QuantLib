/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file analyticamericanmargrabeengine.hpp
    \brief Analytic engine for American Margrabe option
*/

#ifndef quantlib_analytic_american_margrabe_engine_hpp
#define quantlib_analytic_american_margrabe_engine_hpp

#include <ql/experimental/exoticoptions/margrabeoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    //! Analytic engine for American Margrabe option
    /*! This class implements formulae from
        "The Value of an American Option to Exchange One Asset for Another",
        W. Margrabe,
        Journal of Finance, 33, 177-86.

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class AnalyticAmericanMargrabeEngine : public MargrabeOption::engine {
      public:
        AnalyticAmericanMargrabeEngine(std::shared_ptr<GeneralizedBlackScholesProcess> process1,
                                       std::shared_ptr<GeneralizedBlackScholesProcess> process2,
                                       Real correlation);
        void calculate() const override;

      private:
        std::shared_ptr<GeneralizedBlackScholesProcess> process1_;
        std::shared_ptr<GeneralizedBlackScholesProcess> process2_;
        Real rho_;
    };

}

#endif
