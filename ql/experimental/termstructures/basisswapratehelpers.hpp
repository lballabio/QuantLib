/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2021 StatPro Italia srl

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

/*! \file basisswapratehelpers.hpp
    \brief ibor-ibor and ois-ibor basis swap rate helpers
*/

#ifndef quantlib_basisswapratehelpers_hpp
#define quantlib_basisswapratehelpers_hpp

#include <ql/termstructures/yield/ratehelpers.hpp>

namespace QuantLib {

    //! Rate helper for bootstrapping over ibor-ibor basis swaps
    /*! The swap is assumed to pay baseIndex + basis and receive
        otherIndex.  The helper can be used to bootstrap the forecast
        curve for baseIndex (in which case you'll have to pass
        bootstrapBaseCurve = true and provide otherIndex with a
        forecast curve) or the forecast curve for otherIndex (in which
        case bootstrapBaseCurve = false and baseIndex will need a
        forecast curve).
        In both cases, an exogenous discount curve is required.
    */
    class IborIborBasisSwapRateHelper : public RelativeDateRateHelper {
      public:
        IborIborBasisSwapRateHelper(const Handle<Quote>& basis,
                                    const Period& tenor,
                                    Natural settlementDays,
                                    Calendar calendar,
                                    BusinessDayConvention convention,
                                    bool endOfMonth,
                                    const ext::shared_ptr<IborIndex>& baseIndex,
                                    const ext::shared_ptr<IborIndex>& otherIndex,
                                    const Handle<YieldTermStructure>& discountHandle,
                                    bool bootstrapBaseCurve);

        Real impliedQuote() const override;
        void accept(AcyclicVisitor&) override;
      private:
        void initializeDates() override;
        void setTermStructure(YieldTermStructure*) override;

        Period tenor_;
        Natural settlementDays_;
        Calendar calendar_;
        BusinessDayConvention convention_;
        bool endOfMonth_;
        ext::shared_ptr<IborIndex> baseIndex_;
        ext::shared_ptr<IborIndex> otherIndex_;
        Handle<YieldTermStructure> discountHandle_;
        bool bootstrapBaseCurve_;

        ext::shared_ptr<Swap> swap_;

        RelinkableHandle<YieldTermStructure> termStructureHandle_;
    };

}

#endif
