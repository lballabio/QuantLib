/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Master IMAFA - Polytech'Nice Sophia - Université de Nice Sophia Antipolis

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

/*! \file kirkspreadoptionengine.hpp
    \brief Kirk approximation for European spread option on futures
*/

#ifndef quantlib_kirk_spread_option_engine_hpp
#define quantlib_kirk_spread_option_engine_hpp

#include <ql/experimental/exoticoptions/spreadoption.hpp>
#include <ql/processes/blackscholesprocess.hpp>

namespace QuantLib {

    QL_DEPRECATED_DISABLE_WARNING
    
    /*! \deprecated Use BasketOption and KirkEngine instead.
                        Deprecated in version 1.37.
    */
    class [[deprecated("Use BasketOption and KirkEngine instead")]] KirkSpreadOptionEngine : public SpreadOption::engine {
      public:
        KirkSpreadOptionEngine(ext::shared_ptr<BlackProcess> process1,
                               ext::shared_ptr<BlackProcess> process2,
                               Handle<Quote> correlation);
        void calculate() const override;

      private:
        ext::shared_ptr<BlackProcess> process1_;
        ext::shared_ptr<BlackProcess> process2_;
        Handle<Quote> rho_;
    };

    QL_DEPRECATED_ENABLE_WARNING

}

#endif
