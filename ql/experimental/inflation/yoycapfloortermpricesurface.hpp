/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2009 Chris Kenyon
 Copyright (C) 2009 Bernd Engelmann

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

/*! \file yoyoptionlethelpers.hpp
    \brief yoy inflation cap and floor term-price structure
*/

#ifndef quantlib_yoy_capfloor_term_price_surface_hpp
#define quantlib_yoy_capfloor_term_price_surface_hpp

#include <ql/indexes/inflationindex.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/experimental/inflation/polynomial2Dspline.hpp>
#include <cmath>

namespace QuantLib {

    //! Abstract base class, inheriting from InflationTermStructure
    /*! Since this can create a yoy term structure it does take
        a YoY index.

        \todo deal with index interpolation.
    */
    class YoYCapFloorTermPriceSurface : public InflationTermStructure {
      public:
        YoYCapFloorTermPriceSurface(Natural fixingDays,
                                    const Period& yyLag,
                                    const std::shared_ptr<YoYInflationIndex>& yii,
                                    Rate baseRate,
                                    Handle<YieldTermStructure> nominal,
                                    const DayCounter& dc,
                                    const Calendar& cal,
                                    const BusinessDayConvention& bdc,
                                    const std::vector<Rate>& cStrikes,
                                    const std::vector<Rate>& fStrikes,
                                    const std::vector<Period>& cfMaturities,
                                    const Matrix& cPrice,
                                    const Matrix& fPrice);

        bool indexIsInterpolated() const;

        //! atm yoy swaps from put-call parity on cap/floor data
        /*! uses interpolation (on surface price data), yearly maturities. */
        virtual std::pair<std::vector<Time>, std::vector<Rate> >
        atmYoYSwapTimeRates() const = 0;
        virtual std::pair<std::vector<Date>, std::vector<Rate> >
        atmYoYSwapDateRates() const = 0;

        //! derived from yoy swap rates
        virtual std::shared_ptr<YoYInflationTermStructure> YoYTS() const = 0;
        //! index yoy is based on
        std::shared_ptr<YoYInflationIndex> yoyIndex() const { return yoyIndex_; }

        //! inspectors
        /*! \note you don't know if price() is a cap or a floor
                  without checking the YoYSwapATM level.
            \note atm cap/floor prices are generally
                  inaccurate because they are from extrapolation
                  and intersection.
        */
        //@{
        virtual BusinessDayConvention businessDayConvention() const {return bdc_;}
        virtual Natural fixingDays() const {return fixingDays_;}
        virtual Real price(const Date& d, Rate k) const = 0;
        virtual Real capPrice(const Date& d, Rate k) const = 0;
        virtual Real floorPrice(const Date& d, Rate k) const = 0;
        virtual Rate atmYoYSwapRate(const Date &d,
                                    bool extrapolate = true) const = 0;
        virtual Rate atmYoYRate(const Date &d,
                                const Period &obsLag = Period(-1,Days),
                                bool extrapolate = true) const = 0;

        virtual Real price(const Period& d, Rate k) const;
        virtual Real capPrice(const Period& d, Rate k) const;
        virtual Real floorPrice(const Period& d, Rate k) const;
        virtual Rate atmYoYSwapRate(const Period &d,
                                    bool extrapolate = true) const;
        virtual Rate atmYoYRate(const Period &d,
                                const Period &obsLag = Period(-1,Days),
                                bool extrapolate = true) const;

        virtual std::vector<Rate> strikes() const {return cfStrikes_;}
        virtual std::vector<Rate> capStrikes() const {return cStrikes_;}
        virtual std::vector<Rate> floorStrikes() const {return fStrikes_;}
        virtual std::vector<Period> maturities() const {return cfMaturities_;}
        virtual Rate minStrike() const {return cfStrikes_.front();};
        virtual Rate maxStrike() const {return cfStrikes_.back();};
        virtual Date minMaturity() const {return referenceDate()+cfMaturities_.front();}// \TODO deal with index interpolation
        virtual Date maxMaturity() const {return referenceDate()+cfMaturities_.back();}
        //@}

        virtual Date yoyOptionDateFromTenor(const Period& p) const;

      protected:
        virtual bool checkStrike(Rate K) {
            return ( minStrike() <= K && K <= maxStrike() );
        }
        virtual bool checkMaturity(const Date& d) {
            return ( minMaturity() <= d && d <= maxMaturity() );
        }

        // defaults, mostly used for building yoy-fwd curve from put-call parity
        //  std::shared_ptr<YieldTermStructure> nominal_;
        //  Period lag_;
        //  Calendar cal_;
        Natural fixingDays_;
        BusinessDayConvention bdc_;
        std::shared_ptr<YoYInflationIndex> yoyIndex_;
        Handle<YieldTermStructure> nominalTS_;
        // data
        std::vector<Rate> cStrikes_;
        std::vector<Rate> fStrikes_;
        std::vector<Period> cfMaturities_;
        mutable std::vector<Real> cfMaturityTimes_;
        Matrix cPrice_;
        Matrix fPrice_;
        bool indexIsInterpolated_;
        // constructed
        mutable std::vector<Rate> cfStrikes_;
        mutable std::shared_ptr<YoYInflationTermStructure> yoy_;
        mutable std::pair<std::vector<Time>, std::vector<Rate> > atmYoYSwapTimeRates_;
        mutable std::pair<std::vector<Date>, std::vector<Rate> > atmYoYSwapDateRates_;
    };


    template<class Interpolator2D, class Interpolator1D>
    class InterpolatedYoYCapFloorTermPriceSurface
        : public YoYCapFloorTermPriceSurface {
      public:
        InterpolatedYoYCapFloorTermPriceSurface(
                      Natural fixingDays,
                      const Period &yyLag,  // observation lag
                      const std::shared_ptr<YoYInflationIndex>& yii,
                      Rate baseRate,
                      const Handle<YieldTermStructure> &nominal,
                      const DayCounter &dc,
                      const Calendar &cal,
                      const BusinessDayConvention &bdc,
                      const std::vector<Rate> &cStrikes,
                      const std::vector<Rate> &fStrikes,
                      const std::vector<Period> &cfMaturities,
                      const Matrix &cPrice,
                      const Matrix &fPrice,
                      const Interpolator2D &interpolator2d = Interpolator2D(),
                      const Interpolator1D &interpolator1d = Interpolator1D());

        //! inflation term structure interface
        //@{
        Date maxDate() const override { return yoy_->maxDate(); }
        Date baseDate() const override { return yoy_->baseDate(); }
        //@}
        Natural fixingDays() const override { return fixingDays_; }

        //! \name YoYCapFloorTermPriceSurface interface
        //@{
        std::pair<std::vector<Time>, std::vector<Rate> > atmYoYSwapTimeRates() const override {
            return atmYoYSwapTimeRates_;
        }
        std::pair<std::vector<Date>, std::vector<Rate> > atmYoYSwapDateRates() const override {
            return atmYoYSwapDateRates_;
        }
        std::shared_ptr<YoYInflationTermStructure> YoYTS() const override { return yoy_; }
        Rate price(const Date& d, Rate k) const override;
        Real floorPrice(const Date& d, Rate k) const override;
        Real capPrice(const Date& d, Rate k) const override;
        Rate atmYoYSwapRate(const Date& d, bool extrapolate = true) const override {
            return atmYoYSwapRateCurve_(timeFromReference(d),extrapolate);
        }
        Rate atmYoYRate(const Date& d,
                        const Period& obsLag = Period(-1, Days),
                        bool extrapolate = true) const override {
            // work in terms of maturity-of-instruments
            // so ask for rate with observation lag
            // Third parameter = force linear interpolation of yoy
            return yoy_->yoyRate(d, obsLag, false, extrapolate);
        }
        //@}

        //! \name LazyObject interface
        //@{
        void update() override;
        void performCalculations() const;
        //@}

      protected:
        //! intersection of cap and floor price surfaces at given strikes
        void intersect() const;
        class ObjectiveFunction {
          public:
            ObjectiveFunction(Time t, const Interpolation2D&, const Interpolation2D&);
            Real operator()(Rate guess) const;
          protected:
            const Time t_;
            const Interpolation2D &a_, &b_; // work on references
        };

        //! mess of making it, i.e. create instruments from quotes and bootstrap
        void calculateYoYTermStructure() const;

        // data for surfaces and curve
        mutable std::vector<Rate> cStrikesB_;
        mutable std::vector<Rate> fStrikesB_;
        mutable Matrix cPriceB_;
        mutable Matrix fPriceB_;
        mutable Interpolation2D capPrice_, floorPrice_;
        mutable Interpolation2D floorPrice2_;
        mutable Interpolator2D interpolator2d_;
        mutable Interpolation atmYoYSwapRateCurve_;
        mutable Interpolator1D interpolator1d_;
    };


    // inline definitions

    inline bool YoYCapFloorTermPriceSurface::indexIsInterpolated() const {
        return indexIsInterpolated_;
    }

    // template definitions

    #ifndef __DOXYGEN__

    template<class I2D, class I1D>
    InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    InterpolatedYoYCapFloorTermPriceSurface(
                                    Natural fixingDays,
                                    const Period &yyLag,
                                    const std::shared_ptr<YoYInflationIndex>& yii,
                                    Rate baseRate,
                                    const Handle<YieldTermStructure> &nominal,
                                    const DayCounter &dc,
                                    const Calendar &cal,
                                    const BusinessDayConvention &bdc,
                                    const std::vector<Rate> &cStrikes,
                                    const std::vector<Rate> &fStrikes,
                                    const std::vector<Period> &cfMaturities,
                                    const Matrix &cPrice,
                                    const Matrix &fPrice,
                                    const I2D &interpolator2d,
                                    const I1D &interpolator1d)
    : YoYCapFloorTermPriceSurface(fixingDays, yyLag, yii,
                                  baseRate, nominal, dc, cal, bdc,
                                  cStrikes, fStrikes, cfMaturities,
                                  cPrice, fPrice),
      interpolator2d_(interpolator2d), interpolator1d_(interpolator1d) {
        performCalculations();
    }

    #endif

    template<class I2D, class I1D>
    void InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    update() {
        notifyObservers();
    }


    template<class I2D, class I1D>
    void InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    performCalculations() const {
        // calculate all the useful things
        // ... first the intersection of the cap and floor surfs
        intersect();

        // ... then the yoy term structure, which requires instruments
        // and a bootstrap
        calculateYoYTermStructure();
    }


    template<class I2D, class I1D>
    InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::ObjectiveFunction::
    ObjectiveFunction(const Time t,
                      const Interpolation2D &a,
                      const Interpolation2D &b)
    : t_(t), a_(a), b_(b) {
        // do nothing more
    }


    template<class I2D, class I1D>
    Rate InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    price(const Date &d, const Rate k) const {
        Rate atm = atmYoYSwapRate(d);
        return k > atm ? capPrice(d,k): floorPrice(d,k);
    }


    template<class I2D, class I1D>
    Rate InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    capPrice(const Date &d, const Rate k) const {
        Time t = timeFromReference(d);
        return capPrice_(t,k);
    }


    template<class I2D, class I1D>
    Rate InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    floorPrice(const Date &d, const Rate k) const {
        Time t = timeFromReference(d);
        return floorPrice_(t,k);
    }


    template<class I2D, class I1D>
    Real InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::ObjectiveFunction::
    operator()(Rate guess) const {
        // allow extrapolation because the overlap is typically insufficient
        // looking for a zero
        return ( a_(t_,guess,true) - b_(t_,guess,true) );
    }


    template<class I2D, class I1D>
    void InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    intersect() const {


        // TODO: define the constants outside the code
        const Real maxSearchRange = 0.0201;
        const Real maxExtrapolationMaturity = 5.01;
        const Real searchStep = 0.0050;
        const Real intrinsicValueAddOn = 0.001;

        std::vector<bool> validMaturity(cfMaturities_.size(),false);

        cfMaturityTimes_.clear();
        for (Size i=0; i<cfMaturities_.size();i++) {
            cfMaturityTimes_.push_back(timeFromReference(
                    yoyOptionDateFromTenor(cfMaturities_[i])));
        }

        capPrice_ = interpolator2d_.interpolate(
                            cfMaturityTimes_.begin(),cfMaturityTimes_.end(),
                            cStrikes_.begin(), cStrikes_.end(),
                            cPrice_
                            );
        capPrice_.enableExtrapolation();

        floorPrice_ = interpolator2d_.interpolate(
                            cfMaturityTimes_.begin(),cfMaturityTimes_.end(),
                            fStrikes_.begin(), fStrikes_.end(),
                            fPrice_
                            );
        floorPrice_.enableExtrapolation();

        atmYoYSwapDateRates_.first.clear();
        atmYoYSwapDateRates_.second.clear();
        atmYoYSwapTimeRates_.first.clear();
        atmYoYSwapTimeRates_.second.clear();
        Brent solver;
        Real solverTolerance_ = 1e-7;
        Real lo,hi,guess;
        std::vector<Real> minSwapRateIntersection(cfMaturityTimes_.size());
        std::vector<Real> maxSwapRateIntersection(cfMaturityTimes_.size());
        std::vector<Time> tmpSwapMaturities;
        std::vector<Rate> tmpSwapRates;
        for (Size i = 0; i < cfMaturities_.size(); i++) {
            Time t = cfMaturityTimes_[i];
            // determine the sum of discount factors
            Size numYears = (Size)std::lround(t);
            Real sumDiscount = 0.0;
            for (Size j=0; j<numYears; ++j)
                sumDiscount += nominalTS_->discount(j + 1.0);
            // determine the minimum value of the ATM swap point
            Real tmpMinSwapRateIntersection = -1.e10;
            Real tmpMaxSwapRateIntersection = 1.e10;
            for (Size j=0; j<fStrikes_.size(); ++j) {
                Real price = floorPrice_(t,fStrikes_[j]);
                Real minSwapRate = fStrikes_[j] - price / (sumDiscount * 10000);
                if (minSwapRate > tmpMinSwapRateIntersection)
                    tmpMinSwapRateIntersection = minSwapRate;
            }
            for (Size j=0; j<cStrikes_.size(); ++j) {
                Real price = capPrice_(t,cStrikes_[j]);
                Real maxSwapRate = cStrikes_[j] + price / (sumDiscount * 10000);
                if (maxSwapRate < tmpMaxSwapRateIntersection)
                    tmpMaxSwapRateIntersection = maxSwapRate;
            }
            maxSwapRateIntersection[i] = tmpMaxSwapRateIntersection;
            minSwapRateIntersection[i] = tmpMinSwapRateIntersection;

            // find the interval where the intersection lies
            bool trialsExceeded = false;
            int numTrials = (int)(maxSearchRange / searchStep);
            if ( floorPrice_(t,fStrikes_.back()) > capPrice_(t,fStrikes_.back()) ) {
                int counter = 1;
                bool stop = false;
                Real strike = 0.0;
                while (!stop) {
                    strike = fStrikes_.back() - counter * searchStep;
                    if (floorPrice_(t, strike) < capPrice_(t, strike))
                        stop = true;
                    counter++;
                    if (counter == numTrials + 1) {
                        if (!stop) {
                            stop = true;
                            trialsExceeded = true;
                        }
                    }
                }
                lo = strike;
                hi = strike + searchStep;
            } else {
                int counter = 1;
                bool stop = false;
                Real strike = 0.0;
                while (!stop) {
                    strike = fStrikes_.back() + counter * searchStep;
                    if (floorPrice_(t, strike) > capPrice_(t, strike))
                        stop = true;
                    counter++;
                    if (counter == numTrials + 1) {
                        if (!stop) {
                            stop = true;
                            trialsExceeded = true;
                        }
                    }
                }
                lo = strike - searchStep;
                hi = strike;
            }

            guess = (hi+lo)/2.0;
            Rate kI = -999.999;

            if (!trialsExceeded) {
                try{
                    kI = solver.solve(  ObjectiveFunction(t, capPrice_, floorPrice_), solverTolerance_, guess, lo, hi );
                } catch( std::exception &e) {
                    QL_FAIL("cap/floor intersection finding failed at t = " << t << ", error msg: "<< e.what());
                }
                // error message if kI is economically nonsensical (only if t is large)
                if (kI <= minSwapRateIntersection[i]) {
                    if (t > maxExtrapolationMaturity)
                        QL_FAIL("cap/floor intersection finding failed at t = " << t <<
                                ", error msg: intersection value is below the arbitrage free lower bound "
                                << minSwapRateIntersection[i]);
                }
                else
                {
                    tmpSwapMaturities.push_back(t);
                    tmpSwapRates.push_back(kI);
                    validMaturity[i] = true;
                }
            }
            else
            {
                // error message if t is too large
                if (t > maxExtrapolationMaturity)
                    QL_FAIL("cap/floor intersection finding failed at t = " << t <<
                            ", error msg: no interection found inside the admissible range");
            }
        }

        // extrapolation of swap rates if necessary
        //Polynomial2D tmpInterpol;
        //Interpolation interpol = tmpInterpol.interpolate(tmpSwapMaturities.begin(), tmpSwapMaturities.end(), tmpSwapRates.begin());
        //interpol.enableExtrapolation();
        int counter = 0;
        for (Size i=0; i<cfMaturities_.size(); ++i) {
            if ( !validMaturity[i] ) {
                atmYoYSwapDateRates_.first.push_back(referenceDate()+cfMaturities_[i]);
                atmYoYSwapTimeRates_.first.push_back(timeFromReference(referenceDate()+cfMaturities_[i]));
                // atmYoYSwapRates_->second.push_back(interpol((*cfMaturities_)[i]));
                // Heuristic: overwrite the the swap rate with a value that guarantees that the
                // intrinsic value of all options is lower than the price
                Real newSwapRate = minSwapRateIntersection[i] + intrinsicValueAddOn;
                if (newSwapRate > maxSwapRateIntersection[i])
                    newSwapRate = 0.5 * (minSwapRateIntersection[i] + maxSwapRateIntersection[i]);
                atmYoYSwapTimeRates_.second.push_back(newSwapRate);
                atmYoYSwapDateRates_.second.push_back(newSwapRate);
            } else {
                atmYoYSwapTimeRates_.first.push_back(tmpSwapMaturities[counter]);
                atmYoYSwapTimeRates_.second.push_back(tmpSwapRates[counter]);
                atmYoYSwapDateRates_.first.push_back(
                    yoyOptionDateFromTenor(cfMaturities_.at(counter)));
                atmYoYSwapDateRates_.second.push_back(tmpSwapRates[counter]);
                counter++;
            }
        }

        // create the swap curve using the factory
        atmYoYSwapRateCurve_ =
            interpolator1d_.interpolate(atmYoYSwapTimeRates_.first.begin(),
                                        atmYoYSwapTimeRates_.first.end(),
                                        atmYoYSwapTimeRates_.second.begin());
    }


    template<class I2D, class I1D>
    void InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    calculateYoYTermStructure() const {

        // which yoy-swap points to use in building the yoy-fwd curve?
        // for now pick every year
        Size nYears = (Size)std::lround(timeFromReference(referenceDate()+cfMaturities_.back()));

        std::vector<std::shared_ptr<BootstrapHelper<YoYInflationTermStructure> > > YYhelpers;
        for (Size i=1; i<=nYears; i++) {
            Date maturity = nominalTS_->referenceDate() + Period(i,Years);
            Handle<Quote> quote(std::shared_ptr<Quote>(
                               new SimpleQuote( atmYoYSwapRate( maturity ) )));//!
            std::shared_ptr<BootstrapHelper<YoYInflationTermStructure> >
            anInstrument(
                new YearOnYearInflationSwapHelper(
                                quote, observationLag(), maturity,
                                calendar(), bdc_, dayCounter(),
                                yoyIndex(), nominalTS_));
            YYhelpers.push_back (anInstrument);
        }

        // usually this base rate is known
        // however for the data to be self-consistent
        // we pick this as the end of the curve
        Rate baseYoYRate = atmYoYSwapRate( referenceDate() );//!

        // Linear is OK because we have every year
        std::shared_ptr<PiecewiseYoYInflationCurve<Linear> >   pYITS(
              new PiecewiseYoYInflationCurve<Linear>(
                      nominalTS_->referenceDate(),
                      calendar(), dayCounter(), observationLag(), yoyIndex()->frequency(),
                      yoyIndex()->interpolated(), baseYoYRate,
                      YYhelpers));
        pYITS->recalculate();
        yoy_ = pYITS;   // store

        // check that helpers are repriced
        const Real eps = 1e-5;
        for (Size i=0; i<YYhelpers.size(); i++) {
            Rate original = atmYoYSwapRate( yoyOptionDateFromTenor(Period(i+1,Years)) );
            QL_REQUIRE(fabs(YYhelpers[i]->impliedQuote() - original) <eps,
                       "could not reprice helper "<< i
                       << ", data " << original
                       << ", implied quote " << YYhelpers[i]->impliedQuote()
            );
        }
    }

}


#endif
