/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Klaus Spanderen
 
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

/*! \file kirkengine.hpp
    \brief kirk formulae, due to Kirk (1995)
*/

#ifndef quantlib_kirk_engine_hpp
#define quantlib_kirk_engine_hpp

#include <ql/pricingengines/basket/spreadblackscholesvanillaengine.hpp>

namespace QuantLib {

    //! Pricing engine for spread option on two futures
    /*! This class implements formulae from
        "Correlation in the Energy Markets", E. Kirk
        Managing Energy Price Risk. 
        London: Risk Publications and Enron, pp. 71-78

        \ingroup basketengines

        \test the correctness of the returned value is tested by
              reproducing results available in literature.
    */
    class KirkEngine : public SpreadBlackScholesVanillaEngine {
      public:
        KirkEngine(ext::shared_ptr<GeneralizedBlackScholesProcess> process1,
                   ext::shared_ptr<GeneralizedBlackScholesProcess> process2,
                   Real correlation);

      protected:
        Real calculate(Real f1, Real f2, Real strike, Option::Type optionType,
            Real variance1, Real variance2, DiscountFactor df) const override;
    };
}


#endif
