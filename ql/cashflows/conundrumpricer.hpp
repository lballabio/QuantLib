/*
 Copyright (C) 2006 Giorgio Facchinetti
 Copyright (C) 2006 Mario Pucci

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.


 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details. */

/*! \file conundrumpricer.hpp
    \brief CMS-coupon pricer
*/

#ifndef quantlib_conundrum_pricer_hpp
#define quantlib_conundrum_pricer_hpp

#include <ql/cashflows/couponpricer.hpp>
#include <ql/instruments/payoffs.hpp>

namespace QuantLib {

    class CmsCoupon;
    class YieldTermStructure;
    class Quote;

    class VanillaOptionPricer {
      public:
        virtual ~VanillaOptionPricer() = default;
        virtual Real operator()(Real strike,
                                Option::Type optionType,
                                Real deflator) const = 0;
    };

    class BlackVanillaOptionPricer : public VanillaOptionPricer {
      public:
        BlackVanillaOptionPricer(
                Rate forwardValue,
                Date expiryDate,
                const Period& swapTenor,
                const ext::shared_ptr<SwaptionVolatilityStructure>&
                                                         volatilityStructure);

        Real operator()(Real strike, Option::Type optionType, Real deflator) const override;

      private:
        Rate forwardValue_;
        Date expiryDate_;
        Period swapTenor_;
        ext::shared_ptr<SwaptionVolatilityStructure> volatilityStructure_;
        ext::shared_ptr<SmileSection> smile_;
    };

    class GFunction {
      public:
        virtual ~GFunction() = default;
        virtual Real operator()(Real x) = 0;
        virtual Real firstDerivative(Real x) = 0;
        virtual Real secondDerivative(Real x) = 0;
    };

    class GFunctionFactory {
      public:
        enum YieldCurveModel { Standard,
                               ExactYield,
                               ParallelShifts,
                               NonParallelShifts
        };

        GFunctionFactory() = delete;

        static ext::shared_ptr<GFunction>
        newGFunctionStandard(Size q,
                             Real delta,
                             Size swapLength);
        static ext::shared_ptr<GFunction>
        newGFunctionExactYield(const CmsCoupon& coupon);
        static ext::shared_ptr<GFunction>
        newGFunctionWithShifts(const CmsCoupon& coupon,
                               const Handle<Quote>& meanReversion);
      private:
        class GFunctionStandard : public GFunction {
          public:
            GFunctionStandard(Size q,
                              Real delta,
                              Size swapLength)
            : q_(q), delta_(delta), swapLength_(swapLength) {}
            Real operator()(Real x) override;
            Real firstDerivative(Real x) override;
            Real secondDerivative(Real x) override;

          protected:
            /* number of period per year */
            const int q_;
            /* fraction of a period between the swap start date and
               the pay date  */
            Real delta_;
            /* length of swap*/
            Size swapLength_;
        };

        class GFunctionExactYield : public GFunction {
          public:
            GFunctionExactYield(const CmsCoupon& coupon);
            Real operator()(Real x) override;
            Real firstDerivative(Real x) override;
            Real secondDerivative(Real x) override;

          protected:
            /* fraction of a period between the swap start date and
               the pay date  */
            Real delta_;
            /* accruals fraction*/
            std::vector<Time> accruals_;
        };

        class GFunctionWithShifts : public GFunction {

            Time swapStartTime_;

            Time shapedPaymentTime_;
            std::vector<Time> shapedSwapPaymentTimes_;

            std::vector<Time> accruals_;
            std::vector<Real> swapPaymentDiscounts_;
            Real discountAtStart_, discountRatio_;

            Real swapRateValue_;
            Handle<Quote> meanReversion_;

            Real calibratedShift_, tmpRs_;
            const Real accuracy_;

            //* function describing the non-parallel shape of the curve shift*/
            Real shapeOfShift(Real s) const;
            //* calibration of shift*/
            Real calibrationOfShift(Real Rs);
            Real functionZ(Real x);
            Real derRs_derX(Real x);
            Real derZ_derX(Real x);
            Real der2Rs_derX2(Real x);
            Real der2Z_derX2(Real x);

            class ObjectiveFunction;
            friend class ObjectiveFunction;
            class ObjectiveFunction {
                const GFunctionWithShifts& o_;
                Real Rs_;
                mutable Real derivative_;
                public:
                  virtual ~ObjectiveFunction() = default;
                  ObjectiveFunction(const GFunctionWithShifts& o, const Real Rs) : o_(o), Rs_(Rs) {}
                  virtual Real operator()(const Real& x) const;
                  Real derivative(const Real& x) const;
                  void setSwapRateValue(Real x);
                  const GFunctionWithShifts& gFunctionWithShifts() const { return o_; }
            };

            ext::shared_ptr<ObjectiveFunction> objectiveFunction_;
          public:
            GFunctionWithShifts(const CmsCoupon& coupon, Handle<Quote> meanReversion);
            Real operator()(Real x) override;
            Real firstDerivative(Real x) override;
            Real secondDerivative(Real x) override;
        };

    };

    inline std::ostream& operator<<(std::ostream& out,
                                    GFunctionFactory::YieldCurveModel type) {
        switch (type) {
          case GFunctionFactory::Standard:
            return out << "Standard";
          case GFunctionFactory::ExactYield:
            return out << "ExactYield";
          case GFunctionFactory::ParallelShifts:
            return out << "ParallelShifts";
          case GFunctionFactory::NonParallelShifts:
            return out << "NonParallelShifts";
          default:
            QL_FAIL("unknown option type");
        }
    }

    //! CMS-coupon pricer
    /*! Base class for the pricing of a CMS coupon via static replication
        as in Hagan's "Conundrums..." article
    */
    class HaganPricer: public CmsCouponPricer, public MeanRevertingPricer {
      public:
        /* */
        Real swapletPrice() const override = 0;
        Rate swapletRate() const override;
        Real capletPrice(Rate effectiveCap) const override;
        Rate capletRate(Rate effectiveCap) const override;
        Real floorletPrice(Rate effectiveFloor) const override;
        Rate floorletRate(Rate effectiveFloor) const override;
        /* */
        Real meanReversion() const override;
        void setMeanReversion(const Handle<Quote>& meanReversion) override {
            unregisterWith(meanReversion_);
            meanReversion_ = meanReversion;
            registerWith(meanReversion_);
            update();
        };

      protected:
        HaganPricer(const Handle<SwaptionVolatilityStructure>& swaptionVol,
                    GFunctionFactory::YieldCurveModel modelOfYieldCurve,
                    Handle<Quote> meanReversion);
        void initialize(const FloatingRateCoupon& coupon) override;

        virtual Real optionletPrice(Option::Type optionType,
                                    Real strike) const = 0;

        ext::shared_ptr<YieldTermStructure> rateCurve_;
        GFunctionFactory::YieldCurveModel modelOfYieldCurve_;
        ext::shared_ptr<GFunction> gFunction_;
        const CmsCoupon* coupon_;
        Date paymentDate_, fixingDate_;
        Rate swapRateValue_;
        DiscountFactor discount_;
        Real annuity_;
        Real gearing_;
        Spread spread_;
        Real spreadLegValue_;
        Rate cutoffForCaplet_, cutoffForFloorlet_;
        Handle<Quote> meanReversion_;
        Period swapTenor_;
        ext::shared_ptr<VanillaOptionPricer> vanillaOptionPricer_;
    };


    //! CMS-coupon pricer
    /*! Prices a cms coupon via static replication as in Hagan's
        "Conundrums..." article via numerical integration based on
        prices of vanilla swaptions
    */
    class NumericHaganPricer : public HaganPricer {
      public:
        NumericHaganPricer(
            const Handle<SwaptionVolatilityStructure>& swaptionVol,
            GFunctionFactory::YieldCurveModel modelOfYieldCurve,
            const Handle<Quote>& meanReversion,
            Rate lowerLimit = 0.0,
            Rate upperLimit = 1.0,
            Real precision = 1.0e-6,
            Real hardUpperLimit = QL_MAX_REAL);

        Real upperLimit() const { return upperLimit_; }
        Real stdDeviations() const { return stdDeviationsForUpperLimit_; }

        // private:
        class Function {
          public:
            typedef Real argument_type;
            typedef Real result_type;
            virtual ~Function() = default;
            virtual Real operator()(Real x) const = 0;
        };

        class ConundrumIntegrand : public Function {
            friend class NumericHaganPricer;
          public:
            ConundrumIntegrand(ext::shared_ptr<VanillaOptionPricer> o,
                               const ext::shared_ptr<YieldTermStructure>& rateCurve,
                               ext::shared_ptr<GFunction> gFunction,
                               Date fixingDate,
                               Date paymentDate,
                               Real annuity,
                               Real forwardValue,
                               Real strike,
                               Option::Type optionType);
            Real operator()(Real x) const override;

          protected:
            Real functionF(Real x) const;
            Real firstDerivativeOfF(Real x) const;
            Real secondDerivativeOfF(Real x) const;

            Real strike() const;
            Real annuity() const;
            Date fixingDate() const;
            void setStrike(Real strike);

            const ext::shared_ptr<VanillaOptionPricer> vanillaOptionPricer_;
            const Real forwardValue_, annuity_;
            const Date fixingDate_, paymentDate_;
            Real strike_;
            const Option::Type optionType_;
            ext::shared_ptr<GFunction> gFunction_;
        };

        Real integrate(Real a,
                       Real b,
                       const ConundrumIntegrand& Integrand) const;
        Real optionletPrice(Option::Type optionType, Rate strike) const override;
        Real swapletPrice() const override;
        Real resetUpperLimit(Real stdDeviationsForUpperLimit) const;
        Real refineIntegration(Real integralValue, const ConundrumIntegrand& integrand) const;

        mutable Real upperLimit_, stdDeviationsForUpperLimit_;
        const Real lowerLimit_, requiredStdDeviations_, precision_, refiningIntegrationTolerance_;
        const Real hardUpperLimit_;
    };

    //! CMS-coupon pricer
    class AnalyticHaganPricer : public HaganPricer {
      public:
        AnalyticHaganPricer(
            const Handle<SwaptionVolatilityStructure>& swaptionVol,
            GFunctionFactory::YieldCurveModel modelOfYieldCurve,
            const Handle<Quote>& meanReversion);
      protected:
        Real optionletPrice(Option::Type optionType, Real strike) const override;
        Real swapletPrice() const override;
    };

}


#endif


#ifndef id_3f00d7a0266478746a743c90d025ab99
#define id_3f00d7a0266478746a743c90d025ab99
inline bool test_3f00d7a0266478746a743c90d025ab99(const int* i) {
    return i != nullptr;
}
#endif
