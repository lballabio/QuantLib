
/*
 Copyright (C) 2004 StatPro Italia srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email quantlib-dev@lists.sf.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

/*! \file Instruments/europeanoption.hpp
    \brief European option on a single asset
*/

#ifndef quantlib_european_option_hpp
#define quantlib_european_option_hpp

#include <ql/Instruments/vanillaoption.hpp>

namespace QuantLib {

    //! European option on a single asset
    /*! \ingroup instruments */
    class EuropeanOption : public VanillaOption {
      public:
        EuropeanOption(const boost::shared_ptr<StochasticProcess>&,
                       const boost::shared_ptr<StrikedTypePayoff>&,
                       const boost::shared_ptr<Exercise>&,
                       const boost::shared_ptr<PricingEngine>& engine =
                           boost::shared_ptr<PricingEngine>());
    };

}


#endif

