/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2008 Roland Lichters

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

/*! \file riskybond.hpp
    \brief Defaultable bonds
*/

#ifndef quantlib_riskybond_hpp
#define quantlib_riskybond_hpp

#include <ql/instrument.hpp>
#include <ql/experimental/credit/issuer.hpp>
#include <ql/default.hpp>
#include <ql/time/schedule.hpp>
#include <ql/time/daycounter.hpp>
#include <ql/cashflow.hpp>
#include <ql/indexes/iborindex.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/experimental/credit/pool.hpp>
#include <ql/termstructures/defaulttermstructure.hpp>
#include <ql/currency.hpp>

namespace QuantLib {

    /*! Base class for default risky bonds
        \ingroup credit

        \deprecated Use RiskyBondEngine with regular bonds instead.
                    Deprecated in version 1.24.
    */
    class QL_DEPRECATED RiskyBond : public Instrument {
    public:
        /*! The value is contingent to survival, i.e., the knockout
            probability is considered.  To compute the npv given that
            the issuer has survived, use the riskfreeNPV().

            In each of the \f$n\f$ coupon periods, we can calculate the value
            in the case of survival and default, assuming that the issuer
            can only default in the middle of a coupon period. We denote this time
            \f$T_{i}^{mid}=\frac{T_{i-1}+T_{i}}{2}\f$.

            Given survival we receive the full cash flow (both coupons and notional).
            The time \f$t\f$ value of these payments are given by
            \f[
                \sum_{i=1}^{n}CF_{i}P(t,T_{i})Q(T_{i}<\tau)
            \f]
            where \f$P(t,T)\f$ is the time \f$T\f$ discount bond
            and \f$Q(T<\tau)\f$ is the time \f$T\f$ survival probability.
            \f$n\f$ is the number of coupon periods. This takes care of the payments
            in the case of survival.

            Given default we receive only a fraction of the notional at default.
            \f[
                \sum_{i=1}^{n}Rec N(T_{i}^{mid}) P(t,T_{i}^{mid})Q(T_{i-1}<\tau\leq T_{i})
            \f]
            where \f$Rec\f$ is the recovery rate and \f$N(T)\f$ is the time T notional. The default probability can be
            rewritten as
            \f[
                Q(T_{i-1}<\tau\leq T_{i})=Q(T_{i}<\tau)-Q(T_{i-1}<\tau)=(1-Q(T_{i}\geq\tau))-(1-Q(T_{i-1}\geq\tau))=Q(T_{i-1}\geq\tau)-Q(T_{i}\geq\tau)
            \f]
        */
      RiskyBond(std::string name,
                Currency ccy,
                Real recoveryRate,
                Handle<DefaultProbabilityTermStructure> defaultTS,
                Handle<YieldTermStructure> yieldTS,
                Natural settlementDays = 0,
                Calendar calendar = Calendar());
      ~RiskyBond() override = default;
      virtual std::vector<ext::shared_ptr<CashFlow> > cashflows() const = 0;
      std::vector<ext::shared_ptr<CashFlow> > expectedCashflows();
      virtual Real notional(Date date = Date::minDate()) const = 0;
      virtual Date effectiveDate() const = 0;
      virtual Date maturityDate() const = 0;
      virtual std::vector<ext::shared_ptr<CashFlow> > interestFlows() const = 0;
      virtual std::vector<ext::shared_ptr<CashFlow> > notionalFlows() const = 0;
      Real riskfreeNPV() const;
      Real totalFutureFlows() const;
      std::string name() const;
      Currency ccy() const;
      Handle<YieldTermStructure> yieldTS() const;
      Handle<DefaultProbabilityTermStructure> defaultTS() const;
      Real recoveryRate() const;
      //! \name Instrument interface
      //@{
      bool isExpired() const override;
      //@}
    protected:
      void setupExpired() const override;
      void performCalculations() const override;

    private:
        std::string name_;
        Currency ccy_;
        Real recoveryRate_;
        Handle<DefaultProbabilityTermStructure> defaultTS_;
        Handle<YieldTermStructure> yieldTS_;
    protected:
        // engines data
        Natural settlementDays_;
        Calendar calendar_;
    };

    inline std::string RiskyBond::name() const {
        return name_;
    }

    inline Currency RiskyBond::ccy() const {
        return ccy_;
    }

    inline Handle<YieldTermStructure> RiskyBond::yieldTS() const {
        return yieldTS_;
    }

    inline Handle<DefaultProbabilityTermStructure>
    RiskyBond::defaultTS() const {
        return defaultTS_;
    }

    inline Real RiskyBond::recoveryRate() const {
        return recoveryRate_;
    }

    QL_DEPRECATED_DISABLE_WARNING

    /*! Default risky fixed bond
        \ingroup credit

        \deprecated Use RiskyBondEngine with regular bonds instead.
                    Deprecated in version 1.24.
    */
    class QL_DEPRECATED RiskyFixedBond : public RiskyBond {
    public:
      RiskyFixedBond(const std::string& name,
                     const Currency& ccy,
                     Real recoveryRate,
                     const Handle<DefaultProbabilityTermStructure>& defaultTS,
                     const Schedule& schedule,
                     Real rate,
                     DayCounter dayCounter,
                     BusinessDayConvention paymentConvention,
                     std::vector<Real> notionals,
                     const Handle<YieldTermStructure>& yieldTS,
                     Natural settlementDays = 0);
      std::vector<ext::shared_ptr<CashFlow> > cashflows() const override;
      Real notional(Date date = Date::minDate()) const override;
      Date effectiveDate() const override;
      Date maturityDate() const override;
      std::vector<ext::shared_ptr<CashFlow> > interestFlows() const override;
      std::vector<ext::shared_ptr<CashFlow> > notionalFlows() const override;

    private:
      Schedule schedule_;
      Real rate_;
      DayCounter dayCounter_;
      // BusinessDayConvention paymentConvention_;
      std::vector<Real> notionals_;
      std::vector<ext::shared_ptr<CashFlow> > leg_;
      std::vector<ext::shared_ptr<CashFlow> > interestLeg_;
      std::vector<ext::shared_ptr<CashFlow> > redemptionLeg_;
    };


    /*! Default risky floating bonds
        \ingroup credit

        \deprecated Use RiskyBondEngine with regular bonds instead.
                    Deprecated in version 1.24.
    */
    class QL_DEPRECATED RiskyFloatingBond : public RiskyBond {
    public:
      RiskyFloatingBond(const std::string& name,
                        const Currency& ccy,
                        Real recoveryRate,
                        const Handle<DefaultProbabilityTermStructure>& defaultTS,
                        const Schedule& schedule,
                        ext::shared_ptr<IborIndex> index,
                        Integer fixingDays,
                        Real spread,
                        std::vector<Real> notionals,
                        const Handle<YieldTermStructure>& yieldTS,
                        Natural settlementDays = 0);
      std::vector<ext::shared_ptr<CashFlow> > cashflows() const override;
      Real notional(Date date = Date::minDate()) const override;
      Date effectiveDate() const override;
      Date maturityDate() const override;
      std::vector<ext::shared_ptr<CashFlow> > interestFlows() const override;
      std::vector<ext::shared_ptr<CashFlow> > notionalFlows() const override;

    private:
        Schedule schedule_;
        ext::shared_ptr<IborIndex> index_;
        DayCounter dayCounter_;
        Integer fixingDays_;
        Real spread_;
        // BusinessDayConvention paymentConvention_;
        std::vector<Real> notionals_;
        std::vector<ext::shared_ptr<CashFlow> > leg_;
        std::vector<ext::shared_ptr<CashFlow> > interestLeg_;
        std::vector<ext::shared_ptr<CashFlow> > redemptionLeg_;
    };

    QL_DEPRECATED_ENABLE_WARNING

}


#endif


#ifndef id_39481a4ccd3c05a36f7560a027a8a5d6
#define id_39481a4ccd3c05a36f7560a027a8a5d6
inline bool test_39481a4ccd3c05a36f7560a027a8a5d6(int* i) { return i != 0; }
#endif
