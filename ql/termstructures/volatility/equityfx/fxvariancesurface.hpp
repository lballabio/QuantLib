#ifndef quantlib_axl_fx_variance_surface_hpp
#define quantlib_axl_fx_variance_surface_hpp

#include <ql/termstructures/volatility/equityfx/blackvoltermstructure.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/tradingtimetermstructure.hpp>
#include <ql/time/calendars/weekendsonly.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/math/interpolation.hpp>
#include <ql/termstructures/volatility/fxsmilesection.hpp>
#include <ql/termstructures/volatility/fxcostsmilesection.hpp>
#include <ql/quote.hpp>

namespace QuantLib {

    template<class T>
    class fxVarianceSurface : public BlackVarianceTermStructure {
        /*
         * Implementation of a variance term structure for FXO trading. The aim is linearly
         * interpolate variance in trading time/market time as opposed to real time! This
         * allows the addition of events which are treated as jumps in trading time.
         *
         * All quotes first have to be converted into total variances using real time. We assume
         * a paramteric form for assigning trading time to each day (in the simplest case this
         * can be a piecewise constant function between pillar tenors) Events are treated as
         * multiples of the underlying trading time.
         *
         * This needs to be derived from a subclass of BlackVolTermStructure to allow the usage
         * of the AnalyticEuropeanEngine and therefore we derive from BlackVarianceTermStructure
         * as the interpolation is done in variance space.
         */

    static_assert(std::is_base_of_v<fxSmileSection, T>,
                      "Template parameter must be derived from fxSmileSection");

      public:
        fxVarianceSurface(const Date& referenceDate,
                          const Handle<Quote>& spot,
                          const std::vector<Date>& pillars,
                          const std::vector<Handle<Quote>>& atms,
                          const std::vector<std::vector<Handle<Quote>>>& rrs,
                          const std::vector<std::vector<Handle<Quote>>>& bfs,
                          const std::vector<Real>& deltas,
                          const Handle<YieldTermStructure>& forTs,
                          const Handle<YieldTermStructure>& domTs,
                          const Handle<tradingTimeTermStructure>& timesTs,
                          DeltaVolQuote::DeltaType deltaType,
                          DeltaVolQuote::AtmType atmType,
                          fxSmileSection::FlyType flyType,
                          const Calendar& cal = WeekendsOnly(),
                          BusinessDayConvention bdc = Following,
                          bool forceMonotoneVariance = true);

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        virtual Rate minStrike() const { return QL_MIN_POSITIVE_REAL; };
        virtual Rate maxStrike() const { return QL_MAX_REAL; };
        virtual Date maxDate() const { return maxDate_; };

        Real atmVar(Date d) const;  // atm variance
        Real atmVol(Date d) const;  // atm vol

        // helper functions
        Real fwd(Date d) const { return spot_->value() * forTs_->discount(d) / domTs_->discount(d); };
        Real fwd(Time t) const { return spot_->value() * forTs_->discount(t) / domTs_->discount(t); };

        // Introspection
        Handle<Quote> spot() const { return spot_; };
        Handle<YieldTermStructure> domesticDiscountCurve() const { return domTs_; };
        Handle<YieldTermStructure> foreignDiscountCurve() const { return forTs_; };
        
      private:
        void registerWithMarketData();
        void getTradingTimes();

        virtual Handle<T> interpolatedSmileSection(Time t, Real wt, T ssInit, T ssFinal) const = 0;
        Real blackVarianceImpl(Time t, Real strike) const;  // smile variance

        const Handle<Quote> spot_;
        const std::vector<Date> pillars_;
        const Handle<YieldTermStructure> forTs_;
        const Handle<YieldTermStructure> domTs_;
        const Handle<tradingTimeTermStructure> timeTs_;

        Date maxDate_;
        std::vector<Time> times_;

        mutable std::vector<T> smileSections_;

    };

    template <class T>
    fxVarianceSurface<T>::fxVarianceSurface(const Date& referenceDate,
                                            const Handle<Quote>& spot,
                                            const std::vector<Date>& pillars,
                                            const std::vector<Handle<Quote>>& atms,
                                            const std::vector<std::vector<Handle<Quote>>>& rrs,
                                            const std::vector<std::vector<Handle<Quote>>>& bfs,
                                            const std::vector<Real>& deltas,
                                            const Handle<YieldTermStructure>& forTs,
                                            const Handle<YieldTermStructure>& domTs,
                                            const Handle<tradingTimeTermStructure>& timeTs,
                                            DeltaVolQuote::DeltaType deltaType,
                                            DeltaVolQuote::AtmType atmType,
                                            fxSmileSection::FlyType flyType,
                                            const Calendar& cal,
                                            BusinessDayConvention bdc,
                                            bool forceMonotoneVariance)
    : BlackVarianceTermStructure(referenceDate, cal, bdc, Actual365Fixed()), spot_(spot),
      pillars_(pillars), forTs_(forTs), domTs_(domTs), timeTs_(timeTs), maxDate_(pillars.back()) {
        QL_REQUIRE(pillars.size() == atms.size(), "mismatch between date vector and vol vector");
        QL_REQUIRE(pillars.size() == rrs.size(), "mismatch between date vector and rr vector");
        QL_REQUIRE(pillars.size() == bfs.size(), "mismatch between date vector and bf vector");

        // cannot have dates[0] = referenceDate, since vol at dates[0] must be 0!
        QL_REQUIRE(pillars[0] > referenceDate, "cannot have dates[0] <= referenceDate");

        smileSections_.clear();
        for (Size j = 0; j < atms.size(); j++) {
            // this will also register the smile section with spot, vol quotes and discount curves
            smileSections_.emplace_back(pillars[j], spot_, atms[j], rrs[j], bfs[j],
                                        deltas, forTs_, domTs_, deltaType, atmType, 
                                        flyType, Actual365Fixed(), referenceDate);
        }

        registerWithMarketData();
        getTradingTimes();
    }

    template<class T>
    inline void fxVarianceSurface<T>::update() 
    {
        BlackVarianceTermStructure::update();
        getTradingTimes();
    }

    template <class T>
    Real fxVarianceSurface<T>::atmVar(Date d) const {
        // quick routine for atm variance - avoids smile calibration!
        Time tau = timeTs_->tradingTime(referenceDate(), d);
        if (tau <= times_.back()) {
            // before final expiry...

            if (tau <= times_.front()) {
                // before first expiry
                Real vol = smileSections_.front().atm()->value();
                return (vol * vol * tau) / times_.front();
            } 
            else {
                // interpolate between two smiles
                Size i = 0;
                while (times_[i + 1] < tau)
                    i++;

                // times_ starts at 0 but there is no smile section
                // so adjust i accordingly when accessing smileSections_
                Real frontVol = smileSections_[i - 1].atm()->value();
                Real backVol = smileSections_[i].atm()->value();
                Real frontVar = frontVol * frontVol * times_[i];
                Real backVar = backVol * backVol * times_[i + 1];
                Real slopeVar = (backVar - frontVar) / (times_[i + 1] - times_[i]);
                return frontVar + slopeVar * (tau - times_[i]);
            }
        } 
        else {
            // extrapolate with flat vol in trading time!
            // there might be events beyond the last expiry - this will 
            // account for that as we are working in trading time!
            Real vol = smileSections_.back().atm()->value();
            return (vol * vol * tau) / times_.back();
        }
    }

    template <class T>
    Real fxVarianceSurface<T>::atmVol(Date d) const {
        return std::sqrt(atmVar(d) / timeFromReference(d));
    }

    template <class T>
    Real fxVarianceSurface<T>::blackVarianceImpl(Time t, Real strike) const {
        Real tau = timeTs_->tradingTime(t);
        if (tau <= times_.back()) {
            // before final expiry...

            if (t <= times_.front()) {
                // if its before first expiry - interpolate from 0!
                QL_FAIL("smile before first expiry - not implemented!");
            } 
            else {
                // interpolate between two smiles
                Size i = 0;
                while (times_[i + 1] < tau)
                    i++;

                // times_ starts at 0 but there is no smile section
                // so adjust i accordingly when accessing smileSections_

                Volatility vol;
                // check if the expiry is at one of the pillar dates!
                if ((tau - times_[i]) < (1. / (365 * 8))) {
                    vol = smileSections_[i - 1].volByStrike(strike);
                    return (vol * vol * t);
                } 

                if ((times_[i + 1] - tau) < (1. / (365 * 8))) {
                    vol = smileSections_[i].volByStrike(strike);
                    return (vol * vol * t);
                } 

                // now we have to interpolate!
                Real w = (tau - times_[i]) / (times_[i + 1] - times_[i]);
                Handle<T> ss = interpolatedSmileSection(t, w, smileSections_[i - 1], smileSections_[i]);
                vol = ss->volByStrike(strike);
                return (vol * vol * t);
            }
        } 
        else {
            // extrapolate with flat vol in trading time!
            // there might be events beyond the last expiry - this should
            // account for that as we are working in trading time!
            QL_FAIL("smile extrapolation - not implemented!");
        }
    }

    template <class T>
    void fxVarianceSurface<T>::registerWithMarketData() {
        for (Size i = 0; i < smileSections_.size(); i++) {
            registerWith(ext::make_shared<Observable>(smileSections_[i]));
        }
        registerWith(timeTs_);
    }

    template <class T>
    void fxVarianceSurface<T>::getTradingTimes() {
        times_ = std::vector<Time>(pillars_.size() + 1); // these are market times, NOT real times!
        times_[0] = 0.0;
        for (Size j = 1; j <= pillars_.size(); j++) {
            times_[j] = timeTs_->tradingTime(referenceDate(), pillars_[j - 1]);
        }
    }


    //! \name fxVarianceSurfaceClark
    //@{
    template<class T>
    class fxVarianceSurfaceClark : public fxVarianceSurface<T> {
      public:
        fxVarianceSurfaceClark(const Date& referenceDate,
                               const Handle<Quote>& spot,
                               const std::vector<Date>& pillars,
                               const std::vector<Handle<Quote>>& atms,
                               const std::vector<std::vector<Handle<Quote>>>& rrs,
                               const std::vector<std::vector<Handle<Quote>>>& bfs,
                               const std::vector<Real>& deltas,
                               const Handle<YieldTermStructure>& forTs,
                               const Handle<YieldTermStructure>& domTs,
                               const Handle<tradingTimeTermStructure>& timesTs,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               fxSmileSection::FlyType flyType,
                               const Calendar& cal = WeekendsOnly(),
                               BusinessDayConvention bdc = Following,
                               bool forceMonotoneVariance = true);

      private:
        Handle<T> interpolatedSmileSection(Time t, Real wt, T ssInit, T ssFinal) const;
    };

    template <class T>
    fxVarianceSurfaceClark<T>::fxVarianceSurfaceClark(const Date& referenceDate, const Handle<Quote>& spot,
                                                      const std::vector<Date>& pillars, 
                                                      const std::vector<Handle<Quote>>& atms,
                                                      const std::vector<std::vector<Handle<Quote>>>& rrs,
                                                      const std::vector<std::vector<Handle<Quote>>>& bfs,
                                                      const std::vector<Real>& deltas,
                                                      const Handle<YieldTermStructure>& forTs,
                                                      const Handle<YieldTermStructure>& domTs,
                                                      const Handle<tradingTimeTermStructure>& timeTs,
                                                      DeltaVolQuote::DeltaType deltaType,
                                                      DeltaVolQuote::AtmType atmType,
                                                      fxSmileSection::FlyType flyType,
                                                      const Calendar& cal,
                                                      BusinessDayConvention bdc,
                                                      bool forceMonotoneVariance)
    : fxVarianceSurface<T>(referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, cal, bdc, forceMonotoneVariance) {}


    template <class T>
    Handle<T> fxVarianceSurfaceClark<T>::interpolatedSmileSection(Time t,
                                                                  Real wt,
                                                                  T ssInit,
                                                                  T ssFinal) const {
        // Implementation of flat forward smile interpolation in variance [Clark].
        // Interpolate ATM, 25d and 10d strikes for a given weight (w) to get five
        // points on the interpolated smile. Calibrate and return the smile.
        auto flatFwdVar = [wt](Volatility v1, Volatility v2) {
            Volatility v = v1 * v1 * (1 - wt) + v2 * v2 * wt;
            return std::sqrt(v);
        };

        Handle<Quote> atm = makeQuoteHandle(flatFwdVar(ssInit.atm()->value(), 
                                                       ssFinal.atm()->value()));

        // 25 delta
        Volatility v25c = flatFwdVar(ssInit.volByDelta(0.25, Option::Call),
                                     ssFinal.volByDelta(0.25, Option::Call));
        Volatility v25p = flatFwdVar(ssInit.volByDelta(-0.25, Option::Put),
                                     ssFinal.volByDelta(-0.25, Option::Put));
        Handle<Quote> rr25 = makeQuoteHandle(v25c - v25p);
        Handle<Quote> bf25 = makeQuoteHandle((v25c + v25p) / 2. - atm->value());

        // 10 delta
        Volatility v10c = flatFwdVar(ssInit.volByDelta(0.10, Option::Call),
                                     ssFinal.volByDelta(0.10, Option::Call));
        Volatility v10p = flatFwdVar(ssInit.volByDelta(-0.10, Option::Put),
                                     ssFinal.volByDelta(-0.10, Option::Put));
        Handle<Quote> rr10 = makeQuoteHandle(v10c - v10p);
        Handle<Quote> bf10 = makeQuoteHandle((v10c + v10p) / 2. - atm->value());

        // create and return the smile section
        return Handle<T>(ext::make_shared<T>(t, this->spot(), atm, 
                                             std::vector<Handle<Quote>>{rr25, rr10},
                                             std::vector<Handle<Quote>>{bf25, bf10}, 
                                             std::vector<Real>{0.25, 0.10},
                                             this->foreignDiscountCurve(), 
                                             this->domesticDiscountCurve(), 
                                             ssInit.deltaType(), ssInit.atmType(), 
                                             ssInit.flyType(), this->dayCounter()));
    }
    //@}


    //! \name fxVarianceSurfaceNCP
    //@{
    template<class T>
    class fxVarianceSurfaceNCP : public fxVarianceSurface<T> {
      public:
        fxVarianceSurfaceNCP(const Date& referenceDate,
                             const Handle<Quote>& spot,
                             const std::vector<Date>& pillars,
                             const std::vector<Handle<Quote>>& atms,
                             const std::vector<std::vector<Handle<Quote>>>& rrs,
                             const std::vector<std::vector<Handle<Quote>>>& bfs,
                             const std::vector<Real>& deltas,
                             const Handle<YieldTermStructure>& forTs,
                             const Handle<YieldTermStructure>& domTs,
                             const Handle<tradingTimeTermStructure>& timesTs,
                             DeltaVolQuote::DeltaType deltaType,
                             DeltaVolQuote::AtmType atmType,
                             fxSmileSection::FlyType flyType,
                             const Calendar& cal = WeekendsOnly(),
                             BusinessDayConvention bdc = Following,
                             bool forceMonotoneVariance = true);

      private:
        Handle<T> interpolatedSmileSection(Time t, Real wt, T ssInit, T ssFinal) const;
    };


    template <class T>
    fxVarianceSurfaceNCP<T>::fxVarianceSurfaceNCP(const Date& referenceDate, const Handle<Quote>& spot,
                                                  const std::vector<Date>& pillars,
                                                  const std::vector<Handle<Quote>>& atms,
                                                  const std::vector<std::vector<Handle<Quote>>>& rrs,
                                                  const std::vector<std::vector<Handle<Quote>>>& bfs,
                                                  const std::vector<Real>& deltas,
                                                  const Handle<YieldTermStructure>& forTs,
                                                  const Handle<YieldTermStructure>& domTs,
                                                  const Handle<tradingTimeTermStructure>& timeTs,
                                                  DeltaVolQuote::DeltaType deltaType,
                                                  DeltaVolQuote::AtmType atmType,
                                                  fxSmileSection::FlyType flyType,
                                                  const Calendar& cal,
                                                  BusinessDayConvention bdc,
                                                  bool forceMonotoneVariance)
    : fxVarianceSurface<T>(referenceDate, spot, pillars, atms, rrs, bfs, deltas, forTs, domTs,
                           timeTs, deltaType, atmType, flyType, cal, bdc, forceMonotoneVariance) {}


    template <class T>
    Handle<T> fxVarianceSurfaceNCP<T>::interpolatedSmileSection(Time t, 
                                                                Real wt, 
                                                                T ssInit, 
                                                                T ssFinal) const {
        // Implementation of interpolation in probability space using normed call
        // prices [Gope, Fries 2011].

        // calculate the forward for the intrpolated smile
        Real ddom = this->domesticDiscountCurve()->discount(t);
        Real dfor = this->foreignDiscountCurve()->discount(t);
        Real spt = this->spot()->value();

        auto interpNcp = [t, wt, ssInit, ssFinal, spt, ddom, dfor](Real k2) {
            Real np = ssFinal.normedProbability(k2); // normed probability
            Real k1 = ssInit.strikeFromNormProb(np); // strike on first smile with same norm prob

            Real ncp1 = ssInit.normedCallPrice(k1);  // ncp at shorter smile
            Real ncp2 = ssFinal.normedCallPrice(k2); // ncp at longer smile
            Real m1 = k1 / ssInit.forward();         // short moneyness
            Real m2 = k2 / ssFinal.forward();        // long moneyness

            // interpolated values
            Real fwd = spt * dfor / ddom;
            Real k = fwd * ((1. - wt) * m1 + wt * m2);     // strike
            Real c = fwd * ((1. - wt) * ncp1 + wt * ncp2); // prem

            Brent solver;
            solver.setMaxEvaluations(10000);
            Real iw = solver.solve([&](Real w) { return BlackCalculator(Option::Call, k, fwd, w).value() - c; }, 
                                    1e-12, 0.1, 0.01, 0.5);

            Real v = iw / std::sqrt(t); // black calculator uses total vol so scale by sqrt(time)

            // convert the (strike, vol) to a (delta, vol) to create a DeltaVolQuote
            Real d;
            if (k > fwd) {
                // create a call
                d = BlackDeltaCalculator(Option::Call, ssFinal.deltaType(), spt, ddom, dfor, iw)
                        .deltaFromStrike(k);
            } else {
                d = BlackDeltaCalculator(Option::Put, ssFinal.deltaType(), spt, ddom, dfor, iw)
                        .deltaFromStrike(k);
            }
            return DeltaVolQuote(d, makeQuoteHandle(v), t, ssFinal.deltaType());
        };

        std::vector<Handle<DeltaVolQuote>> quotes;

        // 10d put
        Real k_p10 = ssFinal.strikeByDelta(-0.10, Option::Put);
        quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(interpNcp(k_p10))));

        // 25d put
        Real k_p25 = ssFinal.strikeByDelta(-0.25, Option::Put);
        quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(interpNcp(k_p25))));

        // atm
        Real k_atm = ssFinal.atmLevel();
        quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(interpNcp(k_atm))));

        // 25c call
        Real k_c25 = ssFinal.strikeByDelta(0.25, Option::Call);
        quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(interpNcp(k_c25))));

        // 10c call
        Real k_c10 = ssFinal.strikeByDelta(0.10, Option::Call);
        quotes.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(interpNcp(k_c10))));

        // create and return the smile section
        return Handle<T>(ext::make_shared<T>(t, this->spot(), quotes, 
                                             this->foreignDiscountCurve(),
                                             this->domesticDiscountCurve(), 
                                             ssInit.deltaType(), ssInit.atmType(), 
                                             ssInit.flyType(), this->dayCounter()));
    }
    //@}


}  // namespace QuantLib

#endif
