/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2001, 2002, 2003 Sadruddin Rejeb
 Copyright (C) 2006 StatPro Italia srl

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

/*! \file blackswaptionengine.hpp
    \brief Black-formula swaption engine
*/

#ifndef quantlib_pricers_black_swaption_hpp
#define quantlib_pricers_black_swaption_hpp

#include <ql/instruments/swaption.hpp>
#include <ql/swaptionvolstructure.hpp>

namespace QuantLib {

    //! Black-formula swaption engine
    /*! \ingroup swaptionengines

        \warning The engine assumes that the exercise date equals the
                 start date of the passed swap.
    */
    class BlackSwaptionEngine : public Swaption::engine,
                                public Observer {
      public:
        BlackSwaptionEngine(const Handle<Quote>& volatility);
        BlackSwaptionEngine(const Handle<SwaptionVolatilityStructure>&);
        void calculate() const;
        void update();
      private:
        Handle<SwaptionVolatilityStructure> volatility_;
    };

}


#endif
