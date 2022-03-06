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

/*! \file yoyinflationcoupon.hpp
 \brief Coupon paying a yoy inflation index
 */

#ifndef quantlib_newyoy_coupon_hpp
#define quantlib_newyoy_coupon_hpp

#include <ql/cashflows/inflationcoupon.hpp>
#include <ql/indexes/inflationindex.hpp>
#include <ql/time/schedule.hpp>

namespace QuantLib {
    class YoYInflationCouponPricer;

    //! %Coupon paying a YoY-inflation type index
    class YoYInflationCoupon : public InflationCoupon {
    public:
        YoYInflationCoupon(const Date& paymentDate,
                        Real nominal,
                        const Date& startDate,
                        const Date& endDate,
                        Natural fixingDays,
                        const ext::shared_ptr<YoYInflationIndex>& index,
                        const Period& observationLag,
                        const DayCounter& dayCounter,
                        Real gearing = 1.0,
                        Spread spread = 0.0,
                        const Date& refPeriodStart = Date(),
                        const Date& refPeriodEnd = Date()
                        );

        //! \name Inspectors
        //@{
        //! index gearing, i.e. multiplicative coefficient for the index
        Real gearing() const { return gearing_; }
        //! spread paid over the fixing of the underlying index
        Spread spread() const { return spread_; }

        Rate adjustedFixing() const;

        const ext::shared_ptr<YoYInflationIndex>& yoyIndex() const;

        //@}
        //! \name Visitability
        //@{
        void accept(AcyclicVisitor&) override;
        //@}

    private:
        ext::shared_ptr<YoYInflationIndex> yoyIndex_;
    protected:

        Real gearing_;
        Spread spread_;
        bool checkPricerImpl(const ext::shared_ptr<InflationCouponPricer>&) const override;
    };

    inline const ext::shared_ptr<YoYInflationIndex>&
    YoYInflationCoupon::yoyIndex() const {
        return yoyIndex_;
    }

    inline Rate YoYInflationCoupon::adjustedFixing() const {
        return (rate()-spread())/gearing();
    }




    //! Helper class building a sequence of capped/floored yoy inflation coupons
    //! payoff is: spread + gearing x index
    class yoyInflationLeg {
    public:
      yoyInflationLeg(Schedule schedule,
                      Calendar cal,
                      ext::shared_ptr<YoYInflationIndex> index,
                      const Period& observationLag);
      yoyInflationLeg& withNotionals(Real notional);
      yoyInflationLeg& withNotionals(const std::vector<Real>& notionals);
      yoyInflationLeg& withPaymentDayCounter(const DayCounter&);
      yoyInflationLeg& withPaymentAdjustment(BusinessDayConvention);
      yoyInflationLeg& withFixingDays(Natural fixingDays);
      yoyInflationLeg& withFixingDays(const std::vector<Natural>& fixingDays);
      yoyInflationLeg& withGearings(Real gearing);
      yoyInflationLeg& withGearings(const std::vector<Real>& gearings);
      yoyInflationLeg& withSpreads(Spread spread);
      yoyInflationLeg& withSpreads(const std::vector<Spread>& spreads);
      yoyInflationLeg& withCaps(Rate cap);
      yoyInflationLeg& withCaps(const std::vector<Rate>& caps);
      yoyInflationLeg& withFloors(Rate floor);
      yoyInflationLeg& withFloors(const std::vector<Rate>& floors);
      operator Leg() const;
    private:
        Schedule schedule_;
        ext::shared_ptr<YoYInflationIndex> index_;
        Period observationLag_;
        std::vector<Real> notionals_;
        DayCounter paymentDayCounter_;
        BusinessDayConvention paymentAdjustment_;
        Calendar paymentCalendar_;
        std::vector<Natural> fixingDays_;
        std::vector<Real> gearings_;
        std::vector<Spread> spreads_;
        std::vector<Rate> caps_, floors_;
    };



}

#endif



#ifndef id_7c08cd490e1e55f9fd945f1e23aee7c0
#define id_7c08cd490e1e55f9fd945f1e23aee7c0
inline bool test_7c08cd490e1e55f9fd945f1e23aee7c0(const int* i) {
    return i != nullptr;
}
#endif
