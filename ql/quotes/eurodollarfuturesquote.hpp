/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2006, 2008 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud

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

/*! \file eurodollarfuturesquote.hpp
    \brief quote for the Eurodollar-future implied standard deviation
*/

#ifndef quantlib_eurodollar_futures_quote_hpp
#define quantlib_eurodollar_futures_quote_hpp

#include <ql/quote.hpp>
#include <ql/handle.hpp>
#include <ql/patterns/lazyobject.hpp>

namespace QuantLib {

    //! %quote for the Eurodollar-future implied standard deviation
    class EurodollarFuturesImpliedStdDevQuote : public Quote,
                                                public LazyObject {
      public:
        EurodollarFuturesImpliedStdDevQuote(Handle<Quote> forward,
                                            Handle<Quote> callPrice,
                                            Handle<Quote> putPrice,
                                            Real strike,
                                            Real guess = .15,
                                            Real accuracy = 1.0e-6,
                                            Natural maxIter = 100);
        //! \name Quote interface
        //@{
        Real value() const override;
        bool isValid() const override;
        //@}
      protected:
        void performCalculations() const override;
        mutable Real impliedStdev_;
        Real strike_;
        Real accuracy_;
        Natural maxIter_;
        Handle<Quote> forward_;
        Handle<Quote> callPrice_;
        Handle<Quote> putPrice_;
    };

}

#endif
