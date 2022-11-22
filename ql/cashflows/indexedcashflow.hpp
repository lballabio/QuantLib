/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon

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

/*! \file simplecashflow.hpp
 \brief Cash flow dependent on an index ratio (NOT a coupon, i.e. no accruals)
 */

#ifndef quantlib_indexed_cash_flow_hpp
#define quantlib_indexed_cash_flow_hpp

#include <ql/patterns/visitor.hpp>
#include <ql/cashflow.hpp>
#include <ql/index.hpp>

namespace QuantLib {

    //! Cash flow dependent on an index ratio.

    /*! This cash flow is not a coupon, i.e., there's no accrual.  The
        amount is either i(T)/i(0) or i(T)/i(0) - 1, depending on the
        growthOnly parameter.

        We expect this to be used inside an instrument that does all the date
        adjustment etc., so this takes just dates and does not change them.
        growthOnly = false means i(T)/i(0), which is a bond-type setting.
        growthOnly = true means i(T)/i(0) - 1, which is a swap-type setting.
    */
    class IndexedCashFlow : public CashFlow,
                            public Observer {
      public:
        IndexedCashFlow(Real notional,
                        ext::shared_ptr<Index> index,
                        const Date& baseDate,
                        const Date& fixingDate,
                        const Date& paymentDate,
                        bool growthOnly = false);
        //! \name Event interface
        //@{
        Date date() const override { return paymentDate_; }
        //@}
        virtual Real notional() const { return notional_; }
        virtual Date baseDate() const { return baseDate_; }
        virtual Date fixingDate() const { return fixingDate_; }
        virtual ext::shared_ptr<Index> index() const { return index_; }
        virtual bool growthOnly() const { return growthOnly_; }
        //! \name CashFlow interface
        //@{
        Real amount() const override; // already virtual
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
        //! \name Observer interface
        //@{
        void update() override { notifyObservers(); }
        //@}
      private:
        Real notional_;
        ext::shared_ptr<Index> index_;
        Date baseDate_, fixingDate_, paymentDate_;
        bool growthOnly_;
    };


    // inline definitions

    inline void IndexedCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<IndexedCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            CashFlow::accept(v);
    }

}

#endif
