/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2026 Thiago Rodrigues

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

/*! \file di1future.hpp
    \brief B3 one-day interbank deposit (DI1) future
*/

#ifndef quantlib_di1future_hpp
#define quantlib_di1future_hpp

#include <ql/indexes/iborindex.hpp>
#include <ql/instrument.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! B3 one-day interbank deposit (DI1) future
    /*! The contract is quoted as an annual effective rate \f$ R \f$
        with a 252-business-day basis and pays 100,000 points at
        maturity. Its unit price (PU) is
        \f[
            PU = \frac{100000}{(1+R)^{n/252}}
        \f]
        where \f$ n \f$ is the number of business days between the
        trade date, included, and the maturity date, excluded.  The
        maturity is the first trading session of the contract month.

        This class uses the evaluation date as the trade date and
        counts \f$ n \f$ with the day counter of the given index,
        which should be a Business252 one as in the Cdi index.  The
        forward rate implied by the index curve is
        \f[
            (1+R_{fwd})^{n/252} = \frac{P(t_0)}{P(T)}
        \f]
        and the futures rate is \f$ R = R_{fwd} + c \f$, where
        \f$ c \f$ is the convexity adjustment.  The NPV is the
        theoretical unit price obtained from \f$ R \f$.

        Unlike OvernightIndexFuture, which is quoted as
        \f$ 100 \times (1 - R) \f$ with \f$ R \f$ an average or
        compounded rate over a reference period, the DI1 is quoted
        directly in rate and settled in unit price, and its accrual
        period always starts on the trade date.

        \warning The contract is considered expired on its maturity
                 date, when it settles at 100,000 points.

        \ingroup instruments
    */
    class Di1Future : public Instrument {
      public:
        Di1Future(ext::shared_ptr<OvernightIndex> overnightIndex,
                  const Date& maturityDate,
                  Handle<Quote> convexityAdjustment = {});

        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        //@}
        //! \name Inspectors
        //@{
        const ext::shared_ptr<OvernightIndex>& overnightIndex() const { return overnightIndex_; }
        Date maturityDate() const { return maturityDate_; }
        Real convexityAdjustment() const;
        //! trade date, i.e., the evaluation date
        Date referenceDate() const;
        //! business days between the reference date and the maturity
        Date::serial_type businessDays() const;
        //@}
        //! \name Results
        //@{
        //! futures rate, including the convexity adjustment
        Rate rate() const;
        //! unit price (PU) in points; the same as the NPV
        Real unitPrice() const;
        //@}
      private:
        void setupExpired() const override;
        void performCalculations() const override;
        ext::shared_ptr<OvernightIndex> overnightIndex_;
        Date maturityDate_;
        Handle<Quote> convexityAdjustment_;
        mutable Rate rate_ = Null<Rate>();
    };

}

#endif
