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

/*! \file capfloor.hpp
 \brief inflation cap and floor class, just year-on-year variety for now
 */

#ifndef quantlib_instruments_inflationcapfloor_hpp
#define quantlib_instruments_inflationcapfloor_hpp

#include <ql/instrument.hpp>
#include <ql/cashflows/yoyinflationcoupon.hpp>
#include <ql/handle.hpp>

namespace QuantLib {

    class YieldTermStructure;

    //! Base class for yoy inflation cap-like instruments
    /*! \ingroup instruments

        Note that the standard YoY inflation cap/floor defined here is
        different from nominal, because in nominal world standard
        cap/floors do not have the first optionlet.  This is because
        they set in advance so there is no point.  However, yoy
        inflation generally sets (effectively) in arrears, (actually
        in arrears vs lag of a few months) thus the first optionlet is
        relevant.  Hence we can do a parity test without a special
        definition of the YoY cap/floor instrument.

        \test
        - the relationship between the values of caps, floors and the
          resulting collars is checked.
        - the put-call parity between the values of caps, floors and
          swaps is checked.
        - the correctness of the returned value is tested by checking
          it against a known good value.
     */
    class YoYInflationCapFloor : public Instrument {
      public:
        enum Type { Cap, Floor, Collar };
        class arguments;
        class engine;
        YoYInflationCapFloor(Type type,
                             Leg yoyLeg,
                             std::vector<Rate> capRates,
                             std::vector<Rate> floorRates);
        YoYInflationCapFloor(Type type, Leg yoyLeg, const std::vector<Rate>& strikes);
        //! \name Instrument interface
        //@{
        bool isExpired() const override;
        void setupArguments(PricingEngine::arguments*) const override;
        //@}
        //! \name Inspectors
        //@{
        Type type() const { return type_; }
        const std::vector<Rate>& capRates() const { return capRates_; }
        const std::vector<Rate>& floorRates() const { return floorRates_; }
        const Leg& yoyLeg() const { return yoyLeg_; }

        Date startDate() const;
        Date maturityDate() const;
        ext::shared_ptr<YoYInflationCoupon> lastYoYInflationCoupon() const;
        //! Returns the n-th optionlet as a cap/floor with only one cash flow.
        ext::shared_ptr<YoYInflationCapFloor> optionlet(Size n) const;
        //@}
        virtual Rate atmRate(const YieldTermStructure& discountCurve) const;
        //! implied term volatility
        virtual Volatility impliedVolatility(
                            Real price,
                            const Handle<YoYInflationTermStructure>& yoyCurve,
                            Volatility guess,
                            Real accuracy = 1.0e-4,
                            Natural maxEvaluations = 100,
                            Volatility minVol = 1.0e-7,
                            Volatility maxVol = 4.0) const;
      private:
        Type type_;
        Leg yoyLeg_;
        std::vector<Rate> capRates_;
        std::vector<Rate> floorRates_;
    };

    //! Concrete YoY Inflation cap class
    /*! \ingroup instruments */
    class YoYInflationCap : public YoYInflationCapFloor {
      public:
        YoYInflationCap(const Leg& yoyLeg,
            const std::vector<Rate>& exerciseRates)
        : YoYInflationCapFloor(YoYInflationCapFloor::Cap, yoyLeg,
                   exerciseRates, std::vector<Rate>()) {}
    };

    //! Concrete YoY Inflation floor class
    /*! \ingroup instruments */
    class YoYInflationFloor : public YoYInflationCapFloor {
      public:
        YoYInflationFloor(const Leg& yoyLeg,
              const std::vector<Rate>& exerciseRates)
        : YoYInflationCapFloor(YoYInflationCapFloor::Floor, yoyLeg,
                   std::vector<Rate>(), exerciseRates) {}
    };

    //! Concrete YoY Inflation collar class
    /*! \ingroup instruments */
    class YoYInflationCollar : public YoYInflationCapFloor {
      public:
        YoYInflationCollar(const Leg& yoyLeg,
               const std::vector<Rate>& capRates,
               const std::vector<Rate>& floorRates)
        : YoYInflationCapFloor(YoYInflationCapFloor::Collar, yoyLeg,
                               capRates, floorRates) {}
    };


    //! %Arguments for YoY Inflation cap/floor calculation
    class YoYInflationCapFloor::arguments
        : public virtual PricingEngine::arguments {
      public:
        arguments() : type(YoYInflationCapFloor::Type(-1)) {}
        YoYInflationCapFloor::Type type;
        ext::shared_ptr<YoYInflationIndex> index;
        Period observationLag;
        std::vector<Date> startDates;
        std::vector<Date> fixingDates;
        std::vector<Date> payDates;
        std::vector<Time> accrualTimes;
        std::vector<Rate> capRates;
        std::vector<Rate> floorRates;
        std::vector<Real> gearings;
        std::vector<Real> spreads;
        std::vector<Real> nominals;
        void validate() const override;
    };

    //! base class for cap/floor engines
    class YoYInflationCapFloor::engine
    : public GenericEngine<YoYInflationCapFloor::arguments,
                           YoYInflationCapFloor::results> {};

    std::ostream& operator<<(std::ostream&, YoYInflationCapFloor::Type);

    // inline

    inline Volatility YoYInflationCapFloor::impliedVolatility(
                            Real,
                            const Handle<YoYInflationTermStructure>&,
                            Volatility,
                            Real,
                            Natural,
                            Volatility,
                            Volatility) const {
            QL_FAIL("not implemented yet");
        }

}

#endif


#ifndef id_ae9c189edf3a9ca6cfca1740511f01e4
#define id_ae9c189edf3a9ca6cfca1740511f01e4
inline bool test_ae9c189edf3a9ca6cfca1740511f01e4(int* i) { return i != 0; }
#endif
