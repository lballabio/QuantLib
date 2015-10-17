/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2015 Peter Caspers

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

/*! \file fxtarf.hpp
    \brief FX TARF instrument
*/

#ifndef quantlib_fxtarf_hpp
#define quantlib_fxtarf_hpp

#include <ql/experimental/fx/fxindex.hpp>
#include <ql/experimental/fx/proxyinstrument.hpp>
#include <ql/instrument.hpp>
#include <ql/time/schedule.hpp>
#include <ql/option.hpp>
#include <ql/instruments/payoffs.hpp>
#include <ql/utilities/disposable.hpp>

namespace QuantLib {

class FxTarf : public Instrument, public ProxyInstrument {
  public:
    //! coupon types
    enum CouponType { none, capped, full };

    //! forward declarations
    class arguments;
    class results;
    class engine;

    //! proxy description
    struct Proxy : ProxyDescription {
        struct ProxyFunction {
            // function fx spot => npv
            virtual Real operator()(const Real spot) const = 0;
            // this should somehow represent the "trusted" region
            virtual std::pair<Real,Real> coreRegion() const = 0;
        };
        // open fixing dates
        std::vector<Date> openFixingDates;
        // original evaluation date
        Date origEvalDate;
        // last payment date, the npvs are forward npvs w.r.t. this date
        Date lastPaymentDate;
        // buckets for accumulated amonut, e.g.
        // 0.0, 0.1, 0.2, 0.3, 0.4 means
        // [0.0,0.1) has index 0
        // [0.1,0.2) has index 1
        // ...
        // [0.4,target] has index 4
        std::vector<Real> accBucketLimits;
        // proxy functions
        // first index is openFixings-1
        // second index is accAmountIndex
        // A function F should implement
        // operator()(Real spot) = npv
        std::vector<std::vector<boost::shared_ptr<ProxyFunction> > > functions;
        void validate() const {
            QL_REQUIRE(functions.size() == openFixingDates.size(),
                       "number of open fixing dates ("
                           << openFixingDates.size()
                           << ") must be equal to function rows ("
                           << functions.size() << ")");
            for (Size i = 0; i < functions.size(); ++i) {
                QL_REQUIRE(functions[i].size() == accBucketLimits.size(),
                           "number of acc amount buckets ("
                               << accBucketLimits.size()
                               << ") must be equal to function columns ("
                               << functions[i].size() << ") in row " << i);
            }
        }
    };

    //! \name Constructors
    //@{
    /*! If the accumulatedAmount is not null, no past fixings are
        used to calculate the accumulated amount, but exactly this
        number is assumed to represent this amount. The last amount
        must then be fixed to the last fixed amount in order to
        get consistent npvs between fixing and payment date (to be
        precise the last amount is only used between a fixing and
        a payment date, otherwise it can be left empty).

        Note that the accumulatedAmount should always assume a full
        coupon (this is only used to check the target trigger and
        the coupon type none would lead to false results then).

        Note that both the accumulatedAmount and lastAmount are given
        in relative terms (i.e. they are multiplied with the source
        nominal to get the actual amount).
    */
    FxTarf(const Schedule schedule, const boost::shared_ptr<FxIndex> &index,
           const Real sourceNominal,
           const boost::shared_ptr<StrikedTypePayoff> &shortPositionPayoff,
           const boost::shared_ptr<StrikedTypePayoff> &longPositionPayoff,
           const Real target, const CouponType couponType = capped,
           const Real shortPositionGearing = 1.0,
           const Real longPositionGearing = 1.0,
           const Handle<Quote> accumulatedAmount = Handle<Quote>(),
           const Handle<Quote> lastAmount = Handle<Quote>());

    //@}
    //! \name Instrument interface
    //@{
    // the tarf is expired iff accumulated amount >= target
    // and this amount is settled
    bool isExpired() const;
    void setupArguments(PricingEngine::arguments *) const;
    void fetchResults(const PricingEngine::results *) const;

    //@}
    //! \name Additional interface
    //@{
    Date startDate() const;
    Date maturityDate() const;
    Disposable<std::vector<Date> > fixingDates() const;
    const boost::shared_ptr<FxIndex> index() const;

    /*! this is the accumulated amount, but always assuming
        the coupon type full
     */
    Real accumulatedAmount() const {
        return accumulatedAmountAndSettlement().first;
    }
    Real lastAmount() const;
    bool lastAmountSettled() const {
        return accumulatedAmountAndSettlement().second;
    }
    Real target() const { return target_; }
    Real sourceNominal() const { return sourceNominal_; }

    //! description for proxy pricing
    boost::shared_ptr<ProxyDescription> proxy() const;

    /*! payout in domestic currency (for nominal 1) */
    Real payout(const Real fixing) const;

    /*! same as above, but assuming the given accumulated amount,
        which is in addition updated to the new value after the
        fixing */
    Real payout(const Real fixing, Real &accumulatedAmount) const;

  protected:
    //! \name Instrument interface
    //@{
    void setupExpired() const;
    //@}

  private:
    /* payout assuming a full coupon and the given accumulated amount,
       which is updated at the same time (for nominal 1) */
    Real nakedPayout(const Real fixing, Real &accumulatedAmount) const;

    std::pair<Real, bool> accumulatedAmountAndSettlement() const;

    // termsheet data
    const Schedule schedule_;
    const boost::shared_ptr<FxIndex> index_;
    const Real sourceNominal_;
    const boost::shared_ptr<StrikedTypePayoff> shortPositionPayoff_,
        longPositionPayoff_;
    const Real target_;
    const CouponType couponType_;
    const Real shortPositionGearing_, longPositionGearing_;

    // additional data
    mutable std::vector<Date> openFixingDates_, openPaymentDates_;
    Handle<Quote> accumulatedAmount_, lastAmount_;

    // proxy pricing information
    mutable boost::shared_ptr<ProxyDescription> proxy_;
};

class FxTarf::arguments : public virtual PricingEngine::arguments {
  public:
    Schedule schedule;
    std::vector<Date> openFixingDates, openPaymentDates;
    boost::shared_ptr<FxIndex> index;
    Real target, sourceNominal;
    Option::Type longPositionType;
    Real accumulatedAmount, lastAmount;
    bool isLastAmountSettled;
    const FxTarf *instrument;
    void validate() const;
};

class FxTarf::results : public Instrument::results {
  public:
    void reset();
    boost::shared_ptr<FxTarf::Proxy> proxy;
};

class FxTarf::engine
    : public GenericEngine<FxTarf::arguments, FxTarf::results> {};

} // namespace QuantLib

#endif
