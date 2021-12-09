/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Ferdinando Ametrano

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

/*! \file forwardswapquote.hpp
    \brief quote for a forward starting swap
*/

#ifndef quantlib_forward_swap_quote_hpp
#define quantlib_forward_swap_quote_hpp

#include <ql/patterns/lazyobject.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/indexes/swapindex.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Quote for a forward starting swap
    class ForwardSwapQuote : public Quote,
                             public LazyObject {
      public:
        ForwardSwapQuote(ext::shared_ptr<SwapIndex> swapIndex,
                         Handle<Quote> spread,
                         const Period& fwdStart);
        //! \name Quote interface
        //@{
        Real value() const override;
        bool isValid() const override;
        //@}
        //! \name Observer interface
        //@{
        void update() override;
        //@}
        const Date& valueDate() const;
        const Date& startDate() const;
        const Date& fixingDate() const;
      protected:
        void initializeDates();
        void performCalculations() const override;

        ext::shared_ptr<SwapIndex> swapIndex_;
        Handle<Quote> spread_;
        Period fwdStart_;

        Date evaluationDate_, valueDate_, startDate_, fixingDate_;
        ext::shared_ptr<VanillaSwap> swap_;

        mutable Rate result_;
    };

}

#endif
