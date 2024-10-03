/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2011 Chris Kenyon
 Copyright (C) 2022 Quaternion Risk Management Ltd

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
        //! \name Constructors
        //@{
        /*! This constructor takes the base CPI to be used in the calculations. */
        CPICoupon(Real baseCPI,
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const ext::shared_ptr<ZeroInflationIndex>& index,
                  const Period& observationLag,
                  CPI::InterpolationType observationInterpolation,
                  const DayCounter& dayCounter,
                  Real fixedRate,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const Date& exCouponDate = Date());

        /*! This constructor takes a base date; the coupon will use it
            to retrieve the base CPI to be used in the calculations.
        */
        CPICoupon(const Date& baseDate,
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const ext::shared_ptr<ZeroInflationIndex>& index,
                  const Period& observationLag,
                  CPI::InterpolationType observationInterpolation,
                  const DayCounter& dayCounter,
                  Real fixedRate,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const Date& exCouponDate = Date());

        /*! This constructor takes both a base CPI and a base date.
            If both are passed, the base CPI is used in the calculations.
        */
        CPICoupon(Real baseCPI,
                  const Date& baseDate,
                  const Date& paymentDate,
                  Real nominal,
                  const Date& startDate,
                  const Date& endDate,
                  const ext::shared_ptr<ZeroInflationIndex>& index,
                  const Period& observationLag,
                  CPI::InterpolationType observationInterpolation,
                  const DayCounter& dayCounter,
                  Real fixedRate,
                  const Date& refPeriodStart = Date(),
                  const Date& refPeriodEnd = Date(),
                  const Date& exCouponDate = Date());
        //@}

        //! \name Inspectors
        //@{
        //! fixed rate that will be inflated by the index ratio
        Real fixedRate() const;

        //! base value for the CPI index
        /*! \warning make sure that the interpolation used to create
                     this is what you are using for the fixing,
                     i.e. the observationInterpolation.
        */
        Rate baseCPI() const;

        //! base date for the base fixing of the CPI index
        Date baseDate() const;

        //! how do you observe the index?  as-is, flat, linear?
        CPI::InterpolationType observationInterpolation() const;

        //! index used
        ext::shared_ptr<ZeroInflationIndex> cpiIndex() const;
        //@}

        //! \name Calculations
        //@{
        Real accruedAmount(const Date&) const override;

        //! the index value observed (with a lag) at the end date
        Rate indexFixing() const override;

        //! the ratio between the index fixing at the passed date and the base CPI
        /*! No adjustments are applied */
        Rate indexRatio(Date d) const;

        //! the ratio between the end index fixing and the base CPI
        /*! This might include adjustments calculated by the pricer */
        Rate adjustedIndexGrowth() const;
        //@}

        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}
      protected:
        Real baseCPI_;
        Real fixedRate_;
        CPI::InterpolationType observationInterpolation_;
        Date baseDate_;

        bool checkPricerImpl(const ext::shared_ptr<InflationCouponPricer>&) const override;
    };


    //! Cash flow paying the performance of a CPI (zero inflation) index
    /*! It is NOT a coupon, i.e. no accruals. */
    class CPICashFlow : public IndexedCashFlow {
      public:
        CPICashFlow(Real notional,
                    const ext::shared_ptr<ZeroInflationIndex>& index,
                    const Date& baseDate,
                    Real baseFixing,
                    const Date& observationDate,
                    const Period& observationLag,
                    CPI::InterpolationType interpolation,
                    const Date& paymentDate,
                    bool growthOnly = false);

        //! value used on base date
        /*! This does not have to agree with index on that date. */
        Real baseFixing() const override;
        //! you may not have a valid date
        Date baseDate() const override;

        Date observationDate() const { return observationDate_; }
        Period observationLag() const { return observationLag_; }
        //! do you want linear/constant/as-index interpolation of future data?
        virtual CPI::InterpolationType interpolation() const {
            return interpolation_;
        }
        virtual Frequency frequency() const { return frequency_; }

        ext::shared_ptr<ZeroInflationIndex> cpiIndex() const;

        Real indexFixing() const override;

      protected:
        Real baseFixing_;
        Date observationDate_;
        Period observationLag_;
        CPI::InterpolationType interpolation_;
        Frequency frequency_;
    };


    //! Helper class building a sequence of capped/floored CPI coupons.
    /*! Also allowing for the inflated notional at the end...
        especially if there is only one date in the schedule.
        If the fixed rate is zero you get a FixedRateCoupon, otherwise
        you get a ZeroInflationCoupon.
    */
    class CPILeg {
      public:
        CPILeg(Schedule schedule,
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
        CPILeg& withObservationInterpolation(CPI::InterpolationType);
        CPILeg& withSubtractInflationNominal(bool);
        CPILeg& withCaps(Rate cap);
        CPILeg& withCaps(const std::vector<Rate>& caps);
        CPILeg& withFloors(Rate floor);
        CPILeg& withFloors(const std::vector<Rate>& floors);
        CPILeg& withExCouponPeriod(const Period&,
                                         const Calendar&,
                                         BusinessDayConvention,
                                         bool endOfMonth = false);
        CPILeg& withBaseDate(const Date& baseDate);

        operator Leg() const;

      private:
        Schedule schedule_;
        ext::shared_ptr<ZeroInflationIndex> index_;
        Real baseCPI_;
        Period observationLag_;
        std::vector<Real> notionals_;
        std::vector<Real> fixedRates_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_ = ModifiedFollowing;
        Calendar paymentCalendar_;
        CPI::InterpolationType observationInterpolation_ = CPI::AsIndex;
        bool subtractInflationNominal_ = true;
        std::vector<Rate> caps_, floors_;
        Period exCouponPeriod_;
        Calendar exCouponCalendar_;
        BusinessDayConvention exCouponAdjustment_ = Following;
        bool exCouponEndOfMonth_ = false;
        Date baseDate_ = Date();
    };


    // inline definitions

    inline Real CPICoupon::fixedRate() const {
        return fixedRate_;
    }

    inline Rate CPICoupon::adjustedIndexGrowth() const {
        return rate()/fixedRate();
    }

    inline Rate CPICoupon::indexFixing() const {
        return CPI::laggedFixing(cpiIndex(), accrualEndDate(), observationLag(), observationInterpolation());
    }

    inline Rate CPICoupon::baseCPI() const {
        return baseCPI_;
    }

    inline Date CPICoupon::baseDate() const {
        return baseDate_;
    }

    inline CPI::InterpolationType CPICoupon::observationInterpolation() const {
        return observationInterpolation_;
    }

    inline ext::shared_ptr<ZeroInflationIndex> CPICoupon::cpiIndex() const {
        return ext::dynamic_pointer_cast<ZeroInflationIndex>(index());
    }


    inline ext::shared_ptr<ZeroInflationIndex> CPICashFlow::cpiIndex() const {
        return ext::dynamic_pointer_cast<ZeroInflationIndex>(index());
    }

}

#endif
