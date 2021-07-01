/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon

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

/*! \file cpicoupon.hpp
    \brief Coupon paying a zero-inflation index
*/

#ifndef quantlib_cpicoupon_hpp
#define quantlib_cpicoupon_hpp

#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/cashflows/indexedcashflow.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {


    class CPICouponPricer;

    //! %Coupon paying the performance of a CPI (zero inflation) index
    /*! The performance is relative to the index value on the base date.

        The other inflation value is taken from the refPeriodEnd date
        with observation lag, so any roll/calendar etc. will be built
        in by the caller.  By default this is done in the
        InflationCoupon which uses ModifiedPreceding with fixing days
        assumed positive meaning earlier, i.e. always stay in same
        month (relative to referencePeriodEnd).

        This is more sophisticated than an %IndexedCashFlow because it
        does date calculations itself.

        \todo we do not do any convexity adjustment for lags different
              to the natural ZCIIS lag that was used to create the
              forward inflation curve.
    */
    class CPICoupon : public InflationCoupon {
      public:
        CPICoupon(Real baseCPI, // user provided, could be arbitrary
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  Natural fixingDays,
                  const ext::shared_ptr<ZeroInflationIndex>& index,
                  const Period& observationLag,
                  CPI::InterpolationType observationInterpolation,
                  const DayCounter& dayCounter,
                  Real fixedRate, // aka gearing
                  Spread spread = 0.0,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const Date& exCouponDate = Date());

        //! \name Inspectors
        //@{
        //! fixed rate that will be inflated by the index ratio
        Real fixedRate() const;
        //! spread paid over the fixing of the underlying index
        Spread spread() const;

        //! adjusted fixing (already divided by the base fixing)
        Rate adjustedFixing() const;
        //! allows for a different interpolation from the index
        Rate indexFixing() const override;
        //! base value for the CPI index
        /*! \warning make sure that the interpolation used to create
                     this is what you are using for the fixing,
                     i.e. the observationInterpolation.
        */
        Rate baseCPI() const;
        //! how do you observe the index?  as-is, flat, linear?
        CPI::InterpolationType observationInterpolation() const;
        //! utility method, calls indexFixing
        Rate indexObservation(const Date& onDate) const;
        //! index used
        ext::shared_ptr<ZeroInflationIndex> cpiIndex() const;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Real baseCPI_;
        Real fixedRate_;
        Spread spread_;
        CPI::InterpolationType observationInterpolation_;

        bool checkPricerImpl(const ext::shared_ptr<InflationCouponPricer>&) const override;
        // use to calculate for fixing date, allows change of
        // interpolation w.r.t. index.  Can also be used ahead of time
        Rate indexFixing(const Date &) const;
    };


    //! Cash flow paying the performance of a CPI (zero inflation) index
    /*! It is NOT a coupon, i.e. no accruals. */
    class CPICashFlow : public IndexedCashFlow {
      public:
        CPICashFlow(Real notional,
                    const ext::shared_ptr<ZeroInflationIndex>& index,
                    const Date& baseDate,
                    Real baseFixing,
                    const Date& fixingDate,
                    const Date& paymentDate,
                    bool growthOnly = false,
                    CPI::InterpolationType interpolation = CPI::AsIndex,
                    const Frequency& frequency = QuantLib::NoFrequency)
        : IndexedCashFlow(notional, index, baseDate, fixingDate,
                          paymentDate, growthOnly),
          baseFixing_(baseFixing), interpolation_(interpolation),
          frequency_(frequency) {
            QL_REQUIRE(std::fabs(baseFixing_)>1e-16,
                       "|baseFixing|<1e-16, future divide-by-zero error");
            if (interpolation_ != CPI::AsIndex) {
                QL_REQUIRE(frequency_ != QuantLib::NoFrequency,
                           "non-index interpolation w/o frequency");
            }
        }

        //! value used on base date
        /*! This does not have to agree with index on that date. */
        virtual Real baseFixing() const;
        //! you may not have a valid date
        Date baseDate() const override;

        //! do you want linear/constant/as-index interpolation of future data?
        virtual CPI::InterpolationType interpolation() const {
            return interpolation_;
        }
        virtual Frequency frequency() const { return frequency_; }

        //! redefined to use baseFixing() and interpolation
        Real amount() const override;

      protected:
        Real baseFixing_;
        CPI::InterpolationType interpolation_;
        Frequency frequency_;
    };


    //! Helper class building a sequence of capped/floored CPI coupons.
    /*! Also allowing for the inflated notional at the end...
        especially if there is only one date in the schedule.
        If a fixedRate is zero you get a FixedRateCoupon, otherwise
        you get a ZeroInflationCoupon.

        payoff is: spread + fixedRate x index
    */
    class CPILeg {
      public:
        CPILeg(const Schedule& schedule,
               ext::shared_ptr<ZeroInflationIndex> index,
               Real baseCPI,
               const Period& observationLag);
        CPILeg& withNotionals(Real notional);
        CPILeg& withNotionals(const std::vector<Real>& notionals);
        CPILeg& withFixedRates(Real fixedRate);
        CPILeg& withFixedRates(const std::vector<Real>& fixedRates);
        CPILeg& withPaymentDayCounter(const DayCounter&);
        CPILeg& withPaymentAdjustment(BusinessDayConvention);
        CPILeg& withPaymentCalendar(const Calendar&);
        CPILeg& withFixingDays(Natural fixingDays);
        CPILeg& withFixingDays(const std::vector<Natural>& fixingDays);
        CPILeg& withObservationInterpolation(CPI::InterpolationType);
        CPILeg& withSubtractInflationNominal(bool);
        CPILeg& withSpreads(Spread spread);
        CPILeg& withSpreads(const std::vector<Spread>& spreads);
        CPILeg& withCaps(Rate cap);
        CPILeg& withCaps(const std::vector<Rate>& caps);
        CPILeg& withFloors(Rate floor);
        CPILeg& withFloors(const std::vector<Rate>& floors);
        CPILeg& withExCouponPeriod(const Period&,
                                         const Calendar&,
                                         BusinessDayConvention,
                                         bool endOfMonth = false);
        operator Leg() const;

      private:
        Schedule schedule_;
        ext::shared_ptr<ZeroInflationIndex> index_;
        Real baseCPI_;
        Period observationLag_;
        std::vector<Real> notionals_;
        std::vector<Real> fixedRates_;  // aka gearing
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        Calendar paymentCalendar_;
        std::vector<Natural> fixingDays_;
        CPI::InterpolationType observationInterpolation_;
        bool subtractInflationNominal_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_;
        bool exCouponEndOfMonth_;
    };


    // inline definitions

    inline Real CPICoupon::fixedRate() const {
        return fixedRate_;
    }

    inline Real CPICoupon::spread() const {
        return spread_;
    }

    inline Rate CPICoupon::adjustedFixing() const {
        return (rate()-spread())/fixedRate();
    }

    inline Rate CPICoupon::indexFixing() const {
        return indexFixing(fixingDate());
    }

    inline Rate CPICoupon::baseCPI() const {
        return baseCPI_;
    }

    inline CPI::InterpolationType CPICoupon::observationInterpolation() const {
        return observationInterpolation_;
    }

    inline Rate CPICoupon::indexObservation(const Date& onDate) const {
        return indexFixing(onDate);
    }

    inline ext::shared_ptr<ZeroInflationIndex> CPICoupon::cpiIndex() const {
        return ext::dynamic_pointer_cast<ZeroInflationIndex>(index());
    }

}

#endif
