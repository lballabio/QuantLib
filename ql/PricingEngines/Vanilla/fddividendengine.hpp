/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005 Joseph Wang

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

/*! \file fddividendengine.hpp
    \brief base engine for option with dividends
*/

#ifndef quantlib_fd_dividend_engine_hpp
#define quantlib_fd_dividend_engine_hpp

#include <ql/PricingEngines/Vanilla/fdmultiperiodengine.hpp>

#include <ql/cashflow.hpp>

namespace QuantLib {

    //! Abstract base class for dividend engines
    /*! \todo The dividend class really needs to be made more
              sophisticated to distinguish between fixed dividends and
              fractional dividends
    */
    class Arguments;
    class FDDividendEngineBase : public FDMultiPeriodEngine {
    public:
        FDDividendEngineBase(Size timeSteps = 100,
                             Size gridPoints = 100,
                             bool timeDependent = false)
            : FDMultiPeriodEngine(timeSteps, gridPoints,
                                  timeDependent) {}
    protected:
        virtual void setupArguments(const Arguments *a) const;
        void setGridLimits() const = 0;
        void executeIntermediateStep(Size step) const = 0;
        Real getDividend(Size i) const {
            const CashFlow *dividend =
                dynamic_cast<const CashFlow *>(events_[i].get());
            if (dividend) {
                return dividend->amount();
            } else {
                return 0.0;
            }
        }
        Real getDiscountedDividend(Size i) const {
            Real dividend = getDividend(i);
            Real discount =
                process_->riskFreeRate()->
                discount(events_[i]->date()) /
                process_->dividendYield()->
                discount(events_[i]->date());
            return dividend * discount;
        }
    };

    //! Finite-differences pricing engine for dividend options using
    // escowed dividend model
    /*! \ingroup vanillaengines */
    /* The merton 73 engine is the classic engine described in most
       derivatives texts.  However, Haug, Haug, and Lewis in
       "Back to Basics: a new approach to the discrete dividend
       problem" argues that this scheme underprices call options.
       This is set as the default engine, because it is consistent
       with the analytic version.
    */
    class FDDividendEngineMerton73 : public FDDividendEngineBase {
      public:
        FDDividendEngineMerton73(Size timeSteps = 100,
                         Size gridPoints = 100,
                         bool timeDependent = false)
        : FDDividendEngineBase(timeSteps, gridPoints,
                               timeDependent) {}
      private:
        void setGridLimits() const;
        void executeIntermediateStep(Size step) const;
    };

    //! Finite-differences pricing engine for dividend options using
    // shifted dividends
    /*! \ingroup vanillaengines */
    /* This engine uses the same algorithm that was used in quantlib
       in versions 0.3.11 and earlier.  It produces results that
       are different from the Merton 73 engine.

       \todo Review literature to see whether this is described
    */

    class FDDividendEngineShift : public FDDividendEngineBase {
      public:
        FDDividendEngineShift(Size timeSteps = 100,
                         Size gridPoints = 100,
                         bool timeDependent = false)
        : FDDividendEngineBase(timeSteps, gridPoints,
                               timeDependent) {}
      private:
        void setGridLimits() const;
        void executeIntermediateStep(Size step) const;
    };

    // Use Merton73 engine as default.
    typedef FDDividendEngineMerton73 FDDividendEngine;

}


#endif
