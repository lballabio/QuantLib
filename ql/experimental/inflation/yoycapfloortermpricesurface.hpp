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

/*! \file yoyoptionlethelpers.hpp
    \brief yoy inflation cap and floor term-price structure
*/

#ifndef quantlib_yoy_capfloor_term_price_surface_hpp
#define quantlib_yoy_capfloor_term_price_surface_hpp

#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/inflation/inflationhelpers.hpp>
#include <ql/termstructures/inflation/piecewiseyoyinflationcurve.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/math/interpolations/interpolation2d.hpp>
#include <ql/math/solvers1d/brent.hpp>

namespace QuantLib {

    //! Abstract base class, inheriting from InflationTermStructure
    class YoYCapFloorTermPriceSurface : public InflationTermStructure {
      public:
        YoYCapFloorTermPriceSurface(Natural fixingDays,
                                    const Period &yyLag,
                                    Frequency frequency,
                                    Rate baseRate,
                                    const Handle<YieldTermStructure> &nominal,
                                    const DayCounter &dc,
                                    const Calendar &cal,
                                    const BusinessDayConvention &bdc,
                                    const std::vector<Rate> &cStrikes,
                                    const std::vector<Rate> &fStrikes,
                                    const std::vector<Time> &cfMaturities,
                                    const Matrix &cPrice,
                                    const Matrix &fPrice);

        //! inflation term structure interface
        //@{
        // virtual Date maxDate() { return yoy_->maxDate();}
        // virtual Date baseDate() { return yoy_->baseDate();}
        //@}

        //! atm yoy swaps from put-call parity on cap/floor data
        //! uses interpolation (on surface price data), yearly maturities
        virtual std::pair<std::vector<Time>, std::vector<Rate> >
        atmYoYSwapRates() const = 0;

        //! derived from yoy swap rates
        virtual boost::shared_ptr<YoYInflationTermStructure> YoYTS() const = 0;

        //! inspectors
        //@{
        //! N.B. you don't know if price() is a cap or a floor
        //! without checking the YoYSwapATM level.
        //! N.B. atm cap/floor prices are generally
        //! inaccurate because they are from extrapolation
        //! and intersection.
        virtual BusinessDayConvention businessDayConvention() const {return bdc_;}
        virtual Natural fixingDays() const {return fixingDays_;}
        virtual Real price(const Time &t, const Rate k) const = 0;
        virtual Real capPrice(const Time &t, const Rate k) const = 0;
        virtual Real floorPrice(const Time &t, const Rate k) const = 0;
        virtual Rate atmYoYSwapRate(const Time &,
                                    bool extrapolate = true) const = 0;
        virtual Rate atmYoYRate(const Time &,
                                bool extrapolate = true) const = 0;
        virtual std::vector<Rate> strikes() const {return cfStrikes_;}
        virtual std::vector<Rate> capStrikes() const {return cStrikes_;}
        virtual std::vector<Rate> floorStrikes() const {return fStrikes_;}
        virtual std::vector<Time> maturities() const {return cfMaturities_;}
        virtual Rate minStrike() const {return cfStrikes_.front();};
        virtual Rate maxStrike() const {return cfStrikes_.back();};
        virtual Time minMaturity() const {return cfMaturities_.front();}
        virtual Time maxMaturity() const {return cfMaturities_.back();}
        //@}

      protected:
        virtual bool checkStrike(Rate K) {
            return ( minStrike() <= K && K <= maxStrike() );
        }
        virtual bool checkMaturity(Time t) {
            return ( minMaturity() <= t && t <= maxMaturity() );
        }

        // defaults, mostly used for building yoy-fwd curve from put-call parity
        //  boost::shared_ptr<YieldTermStructure> nominal_;
        //  Period lag_;
        //  Calendar cal_;
        Natural fixingDays_;
        BusinessDayConvention bdc_;
        //  DayCounter dc_;
        //  Frequency frequency_;
        // data
        std::vector<Rate> cStrikes_;
        std::vector<Rate> fStrikes_;
        std::vector<Time> cfMaturities_;
        Matrix cPrice_;
        Matrix fPrice_;
        // constructed
        mutable std::vector<Rate> cfStrikes_;
        mutable boost::shared_ptr<YoYInflationTermStructure> yoy_;
        mutable std::pair<std::vector<Time>, std::vector<Rate> > atmYoYSwapRates_;
    };


    template<class Interpolator2D, class Interpolator1D>
    class InterpolatedYoYCapFloorTermPriceSurface
        : public YoYCapFloorTermPriceSurface {
      public:
        InterpolatedYoYCapFloorTermPriceSurface(
                      Natural fixingDays,
                      const Period &yyLag,
                      Frequency frequency,
                      Rate baseRate,
                      const Handle<YieldTermStructure> &nominal,
                      const DayCounter &dc,
                      const Calendar &cal,
                      const BusinessDayConvention &bdc,
                      const std::vector<Rate> &cStrikes,
                      const std::vector<Rate> &fStrikes,
                      const std::vector<Time> &cfMaturities,
                      const Matrix &cPrice,
                      const Matrix &fPrice,
                      const Interpolator2D &interpolator2d = Interpolator2D(),
                      const Interpolator1D &interpolator1d = Interpolator1D());

        //! inflation term structure interface
        //@{
        virtual Date maxDate() const {return yoy_->maxDate();}
        virtual Date baseDate() const {return yoy_->baseDate();}
        //@}
        virtual Natural fixingDays() const {return fixingDays_;}

        //! \name YoYCapFloorTermPriceSurface interface
        //@{
        virtual std::pair<std::vector<Time>, std::vector<Rate> >
        atmYoYSwapRates() const { return atmYoYSwapRates_; }
        virtual boost::shared_ptr<YoYInflationTermStructure>
        YoYTS() const { return yoy_; }
        virtual Rate price(const Time &t, const Rate k) const;
        virtual Real floorPrice(const Time &t, const Rate k) const;
        virtual Real capPrice(const Time &t, const Rate k) const;
        virtual Rate atmYoYSwapRate(const Time &t,
                                    bool extrapolate = true) const {
            return atmYoYSwapRateCurve_(t,extrapolate);
        }
        virtual Rate atmYoYRate(const Time &t,
                                bool extrapolate = true) const {
            return yoy_->yoyRate(t);
        }
        //@}

        //! \name LazyObject interface
        //@{
        void update();
        void performCalculations() const;
        //@}

      protected:
        //! intersection of cap and floor price surfaces at given strikes
        void intersect() const;
        class ObjectiveFunction {
          public:
            ObjectiveFunction(const Time t,
                              const Interpolation2D &,
                              const Interpolation2D &);
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



    // template definitions

    template<class I2D, class I1D>
    InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    InterpolatedYoYCapFloorTermPriceSurface(
                                    Natural fixingDays,
                                    const Period &yyLag,
                                    Frequency frequency,
                                    Rate baseRate,
                                    const Handle<YieldTermStructure> &nominal,
                                    const DayCounter &dc,
                                    const Calendar &cal,
                                    const BusinessDayConvention &bdc,
                                    const std::vector<Rate> &cStrikes,
                                    const std::vector<Rate> &fStrikes,
                                    const std::vector<Time> &cfMaturities,
                                    const Matrix &cPrice,
                                    const Matrix &fPrice,
                                    const I2D &interpolator2d,
                                    const I1D &interpolator1d)
    : YoYCapFloorTermPriceSurface(fixingDays, yyLag, frequency, baseRate,
                                  nominal, dc, cal, bdc,
                                  cStrikes, fStrikes, cfMaturities,
                                  cPrice, fPrice),
      interpolator2d_(interpolator2d), interpolator1d_(interpolator1d) {
        performCalculations();
    }


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
    price(const Time &t, const Rate k) const {

        Rate atm = atmYoYSwapRate(t);
        return k > atm ? capPrice_(t,k): floorPrice_(t,k);
    }


    template<class I2D, class I1D>
    Rate InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    capPrice(const Time &t, const Rate k) const {

        return capPrice_(t,k);
    }


    template<class I2D, class I1D>
    Rate InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    floorPrice(const Time &t, const Rate k) const {

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

        // generally we must EXTrapolate at least
        // one of these curves to get an intersection
        // Two possibilities:
        // 1) use put-call parity from the other curve
        //    BUT that requires a swap that we don't have
        // 2) create our own extension rather than
        //    leaving it up to the Interpolator
        //    because we know the 2nd derivative is important
        //    and typically this will be missed.
        // We choose 2) matching the first two derivatives
        // calculated as a discrete approximation.

        // only do this if have at least 3 points for each

        Size nc = cStrikes_.size();
        Size ncB = nc + 2;
        Size nc0 = nc - 1;
        Size nc1 = nc - 2;
        Size nc2 = nc - 3;
        Size nf = fStrikes_.size();
        Size nfB = nf + 2;
        Size nM = cfMaturities_.size();
        // two extra strikes in each matrix, so create bigger matrices
        cPriceB_ = Matrix(ncB,nM);
        fPriceB_ = Matrix(nfB,nM);
        for(Size i = 0; i < nM; i++) { // leave empty space on the low/high side
            for(Size j = 0; j < nc; j++) cPriceB_[j+2][i] = cPrice_[j][i];
            for(Size j = 0; j < nf; j++) fPriceB_[j][i] = fPrice_[j][i];
        }

        Rate dc1 = cStrikes_[1] - cStrikes_[0];
        Rate df1 = fStrikes_[nc1] - fStrikes_[nc0];
        Rate dc2 = cStrikes_[2] - cStrikes_[1];
        Rate df2 = fStrikes_[nc2] - fStrikes_[nc1];
        Rate cX1 =  cStrikes_[0] - dc1;
        Rate fX1 =  fStrikes_[nc0] - df1;
        Rate cX2 =  cStrikes_[0] - 2*dc1;
        Rate fX2 =  fStrikes_[nc0] - 2*df1;

        cStrikesB_.clear();
        cStrikesB_.resize(nc+2,0.0);
        for(Size j = 0; j < nc; j++) cStrikesB_[j+2] = cStrikes_[j];
        cStrikesB_[0] = cX2;
        cStrikesB_[1] = cX1;
        fStrikesB_.clear();
        fStrikesB_.resize(nf+2,0.0);
        for(Size j = 0; j < nf; j++) fStrikesB_[j] = fStrikes_[j];
        fStrikesB_[nf] = fX1;
        fStrikesB_[nf+1] = fX2;

        // put in the extra prices
        for(Size i = 0; i < nM; i++) {
            Real gc1 =  (cPrice_[1][i] - cPrice_[0][i]) / dc1;
            Real gc2a =  gc1 + 1 * ( gc1 - (cPrice_[2][i] - cPrice_[1][i]) / dc2);
            Real gc2b =  gc1 + 2 * ( gc1 - (cPrice_[2][i] - cPrice_[1][i]) / dc2);

            Real gf1 =  (fPrice_[nc1][i] - fPrice_[nc0][i]) / df1;
            Real gf2a =  gf1 + 1 * ( gf1 - (fPrice_[nc2][i] - fPrice_[nc1][i]) / df2);
            Real gf2b =  gf1 + 2 * ( gf1 - (fPrice_[nc2][i] - fPrice_[nc1][i]) / df2);

            cPriceB_[1][i] = std::max(cPriceB_[2][i] - gc2a * dc1, 0.0);
            cPriceB_[0][i] = std::max(cPriceB_[1][i] - gc2b * dc1, 0.0);

            fPriceB_[nf][i] = std::max(fPriceB_[nf-1][i] - gf2a * df1, 0.0);
            fPriceB_[nf+1][i] = std::max(fPriceB_[nf][i] - gf2b * df1, 0.0);
        }

        capPrice_ =
            interpolator2d_.interpolate(
                                    cfMaturities_.begin(),cfMaturities_.end(),
                                    cStrikesB_.begin(), cStrikesB_.end(),
                                    cPriceB_);

        floorPrice_ =
            interpolator2d_.interpolate(
                                    cfMaturities_.begin(),cfMaturities_.end(),
                                    fStrikesB_.begin(), fStrikesB_.end(),
                                    fPriceB_);

        // Get all intersections ... conservatively because the
        // extrapolation may be poor, i.e. start with brackets close
        // then move the brackets out one step at a time (just twice -
        // checking that there is space).
        atmYoYSwapRates_ = std::pair<std::vector<Time>, std::vector<Rate> >();
        Brent solver;
        Real solverTolerance_ = 1e-7;
        Real lo,hi,guess;
        for (Size i = 0; i < cfMaturities_.size(); i++) {
            Time t = cfMaturities_[i];
            lo = fStrikes_[fStrikes_.size()-2];
            hi = cStrikes_[1];
            guess = (hi+lo)/2.0;
            Rate kI = -999.999;
            try {
                kI = solver.solve(ObjectiveFunction(t, capPrice_, floorPrice_),
                                  solverTolerance_, guess, lo, hi );
            } catch( std::exception &e) {
                if(fStrikes_.size()<3 || cStrikes_.size()<3)
                    QL_FAIL("cap/floor intersection finding failed at t = "
                            << t << ", error msg: "<< e.what());
                try {
                    // move the bracket out one step (we know there is space)
                    lo = fStrikes_[fStrikes_.size()-3];
                    hi = cStrikes_[2];
                    guess = (hi+lo)/2.0;
                    kI = solver.solve(ObjectiveFunction(t, capPrice_,
                                                        floorPrice_),
                                      solverTolerance_, guess, lo, hi );
                } catch( std::exception &f) {
                    if(fStrikes_.size()<4 || cStrikes_.size()<4)
                        QL_FAIL("cap/floor intersection finding failed at t = "
                                << t << ", error msg: "<< f.what());
                    try {
                        // move the bracket out one step (we know
                        // there is still space)
                        lo = fStrikes_[fStrikes_.size()-4];
                        hi = cStrikes_[3];
                        guess = (hi+lo)/2.0;
                        kI = solver.solve(ObjectiveFunction(t, capPrice_,
                                                            floorPrice_),
                                          solverTolerance_, guess, lo, hi );
                    } catch( std::exception &g) {
                        QL_FAIL("cap/floor intersection finding failed at t = "
                                << t << ", error msg: "<< g.what());
                    }
                }
            }
            atmYoYSwapRates_.first.push_back(t);
            atmYoYSwapRates_.second.push_back(kI);
        }

        // create the swap curve using the factory
        atmYoYSwapRateCurve_ =
            interpolator1d_.interpolate(atmYoYSwapRates_.first.begin(),
                                        atmYoYSwapRates_.first.end(),
                                        atmYoYSwapRates_.second.begin());
    }


    template<class I2D, class I1D>
    void InterpolatedYoYCapFloorTermPriceSurface<I2D,I1D>::
    calculateYoYTermStructure() const {

        // which yoy-swap points to use in building the yoy-fwd curve?
        // for now pick every year
        Size nYears = (Size)floor(cfMaturities_.back());

        std::vector<boost::shared_ptr<BootstrapHelper<YoYInflationTermStructure> > > YYhelpers;
        for (Size i=1; i<=nYears; i++) {
            Date maturity = nominalTermStructure_->referenceDate() + Period(i,Years);
            Handle<Quote> quote(boost::shared_ptr<Quote>(
                               new SimpleQuote( atmYoYSwapRate( (Time)i ) )));
            boost::shared_ptr<BootstrapHelper<YoYInflationTermStructure> >
            anInstrument(
                new YyiisInflationHelper(
                                quote, lag_, maturity,
                                nominalTermStructure_->settlementDays(),
                                calendar(), bdc_, dayCounter(), frequency_));
            YYhelpers.push_back (anInstrument);
        }

        // usually this base rate is known
        // however for the data to be self-consistent
        // we pick this as the end of the curve
        Rate baseYoYRate = atmYoYSwapRate( (Time)0.0 );
        Handle<YieldTermStructure> nominalH( nominalTermStructure_ );
        // Linear is OK because we have every year
        boost::shared_ptr<PiecewiseYoYInflationCurve<Linear> >   pYITS(
              new PiecewiseYoYInflationCurve<Linear>(
                      nominalTermStructure_->referenceDate(),
                      calendar(), dayCounter(), lag_, frequency_, baseYoYRate,
                      nominalH, YYhelpers));
        pYITS->recalculate();
        yoy_ = pYITS;   // store

        // check that helpers are repriced
        const Real eps = 0.000001;
        for (Size i=0; i<YYhelpers.size(); i++) {
            Rate original = atmYoYSwapRate( (Time)(i+1) );
            QL_REQUIRE(abs(YYhelpers[i]->impliedQuote() - original ) < eps,
                       "could not reprice helper "<< i
                       << ", data " << original
                       << ", implied quote " << YYhelpers[i]->impliedQuote());
        }
    }

}


#endif

