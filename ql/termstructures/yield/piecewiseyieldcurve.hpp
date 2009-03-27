/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2007, 2008, 2009 Ferdinando Ametrano
 Copyright (C) 2007 Chris Kenyon

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

/*! \file piecewiseyieldcurve.hpp
    \brief piecewise-interpolated term structure
*/

#ifndef quantlib_piecewise_yield_curve_hpp
#define quantlib_piecewise_yield_curve_hpp

#include <ql/termstructures/iterativebootstrap.hpp>
#include <ql/termstructures/localbootstrap.hpp>
#include <ql/termstructures/yield/bootstraptraits.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    //! Piecewise yield term structure
    /*! This term structure is bootstrapped on a number of interest
        rate instruments which are passed as a vector of handles to
        RateHelper instances. Their maturities mark the boundaries of
        the interpolated segments.

        Each segment is determined sequentially starting from the
        earliest period to the latest and is chosen so that the
        instrument whose maturity marks the end of such segment is
        correctly repriced on the curve.

        \warning The bootstrapping algorithm will raise an exception if
                 any two instruments have the same maturity date.

        \ingroup yieldtermstructures

        \test
        - the correctness of the returned values is tested by
          checking them against the original inputs.
        - the observability of the term structure is tested.
    */
    template <class Traits, class Interpolator,
              template <class> class Bootstrap = IterativeBootstrap>
    class PiecewiseYieldCurve
        : public Traits::template curve<Interpolator>::type,
          public LazyObject {
      private:
        typedef typename Traits::template curve<Interpolator>::type base_curve;
        typedef PiecewiseYieldCurve<Traits,Interpolator,Bootstrap> this_curve;
      public:
        typedef Traits traits_type;
        typedef Interpolator interpolator_type;
        //! \name Constructors
        //@{
        PiecewiseYieldCurve(
               const Date& referenceDate,
               const std::vector<boost::shared_ptr<typename Traits::helper> >&
                                                                  instruments,
               const DayCounter& dayCounter,
               const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
               const std::vector<Date>& jumpDates = std::vector<Date>(),
               Real accuracy = 1.0e-12,
               const Interpolator& i = Interpolator(),
               const Bootstrap<this_curve>& bootstrap = Bootstrap<this_curve>())
        : base_curve(referenceDate, dayCounter, i),
          instruments_(instruments),
          jumps_(jumps), jumpDates_(jumpDates), jumpTimes_(jumpDates.size()),
          nJumps_(jumps_.size()),
          accuracy_(accuracy), bootstrap_(bootstrap) {
            setJumps();
            for (Size i=0; i<jumps.size(); ++i)
                registerWith(jumps_[i]);
            bootstrap_.setup(this);
        }
        PiecewiseYieldCurve(
               Natural settlementDays,
               const Calendar& calendar,
               const std::vector<boost::shared_ptr<typename Traits::helper> >&
                                                                  instruments,
               const DayCounter& dayCounter,
               const std::vector<Handle<Quote> >& jumps = std::vector<Handle<Quote> >(),
               const std::vector<Date>& jumpDates = std::vector<Date>(),
               Real accuracy = 1.0e-12,
               const Interpolator& i = Interpolator(),
               const Bootstrap<this_curve>& bootstrap = Bootstrap<this_curve>())
        : base_curve(settlementDays, calendar, dayCounter, i),
          instruments_(instruments),
          jumps_(jumps), jumpDates_(jumpDates), jumpTimes_(jumpDates.size()),
          nJumps_(jumps_.size()),
          accuracy_(accuracy), bootstrap_(bootstrap) {
            setJumps();
            for (Size i=0; i<nJumps_; ++i)
                registerWith(jumps_[i]);
            bootstrap_.setup(this);
        }
        //@}
        //! \name YieldTermStructure interface
        //@{
        Date maxDate() const;
        //@}
        //! \name Inspectors
        //@{
        const std::vector<Time>& times() const;
        const std::vector<Date>& dates() const;
        const std::vector<Real>& data() const;
        std::vector<std::pair<Date, Real> > nodes() const;

        const std::vector<Date>& jumpDates() const;
        const std::vector<Time>& jumpTimes() const;
        //@}
        //! \name Observer interface
        //@{
        void update();
        //@}
      private:
        // methods
        void performCalculations() const;
        DiscountFactor discountImpl(Time) const;
        void setJumps();
        // data members
        std::vector<boost::shared_ptr<typename Traits::helper> > instruments_;
        std::vector<Handle<Quote> > jumps_;
        std::vector<Date> jumpDates_;
        std::vector<Time> jumpTimes_;
        Size nJumps_;
        Real accuracy_;
        Date latestReference_;

        // bootstrapper classes are declared as friend to manipulate
        // the curve data. They might be passed the data instead, but
        // it would increase the complexity---which is high enough
        // already.
        friend class Bootstrap<this_curve>;
        friend class BootstrapError<this_curve> ;
        friend class PenaltyFunction<this_curve>;
        Bootstrap<this_curve> bootstrap_;
    };


    // inline definitions

    template <class C, class I, template <class> class B>
    inline Date PiecewiseYieldCurve<C,I,B>::maxDate() const {
        calculate();
        return base_curve::maxDate();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Time>& PiecewiseYieldCurve<C,I,B>::times() const {
        calculate();
        return base_curve::times();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Date>& PiecewiseYieldCurve<C,I,B>::dates() const {
        calculate();
        return base_curve::dates();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Real>& PiecewiseYieldCurve<C,I,B>::data() const {
        calculate();
        return base_curve::data();
    }

    template <class C, class I, template <class> class B>
    inline std::vector<std::pair<Date, Real> >
    PiecewiseYieldCurve<C,I,B>::nodes() const {
        calculate();
        return base_curve::nodes();
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Date>& PiecewiseYieldCurve<C,I,B>::jumpDates() const {
        calculate();
        return this->jumpDates_;
    }

    template <class C, class I, template <class> class B>
    inline const std::vector<Time>& PiecewiseYieldCurve<C,I,B>::jumpTimes() const {
        calculate();
        return this->jumpTimes_;
    }

    template <class C, class I, template <class> class B>
    inline void PiecewiseYieldCurve<C,I,B>::update() {
        base_curve::update();
        LazyObject::update();
        if (base_curve::referenceDate() != latestReference_)
            setJumps();
    }

    template <class C, class I, template <class> class B>
    inline
    DiscountFactor PiecewiseYieldCurve<C,I,B>::discountImpl(Time t) const {
        calculate();

        if (!jumps_.empty()) {
            DiscountFactor jumpEffect = 1.0;
            for (Size i=0; i<nJumps_ && jumpTimes_[i]<t; ++i) {
                QL_REQUIRE(jumps_[i]->isValid(),
                           "invalid " << io::ordinal(i+1) << " jump quote");
                DiscountFactor thisJump = jumps_[i]->value();
                QL_REQUIRE(thisJump > 0.0 && thisJump <= 1.0,
                           "invalid " << io::ordinal(i+1) << " jump value: " <<
                           thisJump);
                jumpEffect *= thisJump;
            }
            return jumpEffect * base_curve::discountImpl(t);
        }

        return base_curve::discountImpl(t);
    }

    template <class C, class I, template <class> class B>
    inline void PiecewiseYieldCurve<C,I,B>::setJumps() {
        Date referenceDate = base_curve::referenceDate();
        if (jumpDates_.empty() && !jumps_.empty()) { // turn of year dates
            jumpDates_.resize(nJumps_);
            jumpTimes_.resize(nJumps_);
            for (Size i=0; i<nJumps_; ++i)
                jumpDates_[i] = Date(31, December, referenceDate.year()+i);
        } else { // fixed dats
            QL_REQUIRE(jumpDates_.size()==nJumps_,
                       "mismatch between number of jumps (" << nJumps_ <<
                       ") and jump dates (" << jumpDates_.size() << ")");
        }
        for (Size i=0; i<nJumps_; ++i)
            jumpTimes_[i] = base_curve::timeFromReference(jumpDates_[i]);
        latestReference_ = referenceDate;
    }


    // template definitions

    template <class C, class I, template <class> class B>
    void PiecewiseYieldCurve<C,I,B>::performCalculations() const {
        // just delegate to the bootstrapper
        bootstrap_.calculate();
    }

}

#endif
