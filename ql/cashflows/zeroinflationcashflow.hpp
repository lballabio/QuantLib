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

/*! \file zeroinflationcashflow.hpp
    \brief Cash flow dependent on an inflation index ratio (NOT a coupon, i.e. no accruals).
*/

#ifndef quantlib_inflation_cash_flow_hpp
#define quantlib_inflation_cash_flow_hpp

#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/indexes/inflationindex.hpp>

namespace QuantLib {

    //! Cash flow dependent on a zero inflation index ratio.
    /*! The ratio is taken between fixings observed at the start date
        and the end date minus the observation lag; that is, if the start
        and end dates are, e.g., in June and the observation lag is three
        months, the ratio will be taken between March fixings.
    */
    class ZeroInflationCashFlow : public IndexedCashFlow {
      public:
        /*! The fixings dates for the index are `startDate - observationLag` and
            `endDate - observationLag`.
        */
        ZeroInflationCashFlow(Real notional,
                              const ext::shared_ptr<ZeroInflationIndex>& index,
                              CPI::InterpolationType observationInterpolation,
                              const Date& startDate,
                              const Date& endDate,
                              const Period& observationLag,
                              const Date& paymentDate,
                              bool growthOnly = false);

        //! \name ZeroInflationCashFlow interface
        //@{
        ext::shared_ptr<ZeroInflationIndex> zeroInflationIndex() const {
            return zeroInflationIndex_;
        }
        CPI::InterpolationType observationInterpolation() const {
            return interpolation_;
        }
        //@}

        //! \name ZeroInflationCashFlow interface
        //@{
        Real baseFixing() const override;
        Real indexFixing() const override;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}

      private:
        ext::shared_ptr<ZeroInflationIndex> zeroInflationIndex_;
        CPI::InterpolationType interpolation_;
        Date startDate_, endDate_;
        Period observationLag_;
    };

}

#endif
