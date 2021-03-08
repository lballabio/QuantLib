/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 Ralf Konrad Eckel

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

/*! \file inflationcashflow.hpp
 \brief Cash flow dependent on an inflation index ratio (NOT a coupon, i.e. no accruals).
 */

#ifndef quantlib_inflation_cash_flow_hpp
#define quantlib_inflation_cash_flow_hpp

#include <ql/cashflows/indexedcashflow.hpp>

namespace QuantLib {
    class ZeroInflationIndex;

    //! Cash flow dependent on a zero inflation index ratio.
    class ZeroInflationCashFlow : public IndexedCashFlow {
      public:
        ZeroInflationCashFlow(const Real& notional,
                              const ext::shared_ptr<ZeroInflationIndex>& index,
                              const bool& useInterpolatedFixings,
                              const Date& baseDate,
                              const Date& fixingDate,
                              const Date& paymentDate,
                              const bool& growthOnly = false);

        //! \name ZeroInflationCashFlow interface
        //@{
        virtual ext::shared_ptr<ZeroInflationIndex> zeroInflationIndex() const {
            return zeroInflationIndex_;
        }
        virtual bool useInterpolatedFixings() const { return useInterpolatedFixings_; }
        //@}

        //! \name CashFlow interface
        //@{
        Real amount() const override;
        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}


      private:
        ext::shared_ptr<ZeroInflationIndex> zeroInflationIndex_;
        bool useInterpolatedFixings_;
    };

    // inline definitions
    inline void ZeroInflationCashFlow::accept(AcyclicVisitor& v) {
        auto* v1 = dynamic_cast<Visitor<ZeroInflationCashFlow>*>(&v);
        if (v1 != nullptr)
            v1->visit(*this);
        else
            IndexedCashFlow::accept(v);
    }
}

#endif
