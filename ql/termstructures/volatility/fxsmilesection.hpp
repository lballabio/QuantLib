#ifndef quantlib_axl_fx_smile_section_hpp
#define quantlib_axl_fx_smile_section_hpp

#include <ql/experimental/fx/blackdeltacalculator.hpp>
#include <ql/experimental/fx/deltavolquote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/pricingengines/blackcalculator.hpp>
#include <ql/quote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/option.hpp>
#include <ql/math/solvers1d/bisection.hpp>
#include <ql/math/solvers1d/brent.hpp>


namespace QuantLib {

    //! Helper for broker-fly / market-strangle calibration.
    /*! Models the rate-helper pattern: each helper corresponds to one
        delta level and computes the broker-fly implied by the current
        calibrated smile.  The residual flyError() = market broker fly
        minus implied broker fly is driven to zero by the outer solver.

        \warning Being a pointer and not a shared_ptr, the smile
                 section is not guaranteed to remain allocated for the
                 whole life of the strangle helper. It is the
                 responsibility of the programmer to ensure that the
                 pointer remains valid. It is advised that this method
                 is called only inside the fx smile section being
                 calibrated, setting the pointer to <b>this</b>,
                 i.e., the smile section itself.
    */
    template <class SS>
    class fxStrangleHelper : public Observer, public Observable {
      public:
        fxStrangleHelper(Handle<Quote> brokerFlyQuote, Real delta);
        fxStrangleHelper(Real brokerFlyQuote, Real delta);
        ~fxStrangleHelper() override = default;

        //! The market broker-fly quote.
        const Handle<Quote>& quote() const { return quote_; }

        //! The delta level this helper calibrates (e.g. 0.25).
        Real delta() const { return delta_; }

        //! Sets the smile section used for pricing.
        void setSmileSection(SS* ss);

        //! Precomputes market strangle strikes and price.
        /*! Must be called after setSmileSection() and after the smile
            section's forward / discount data are available (i.e. after
            calculateForward()).
        */
        void initialize();

        //! Broker fly implied by the current calibrated smile.
        /*! Analogous to BootstrapHelper::impliedQuote().  Computes the
            strangle price from the calibrated smile at the market
            strangle strikes and converts it back to an equivalent
            broker-fly volatility.
        */
        Real impliedQuote() const;

        //! Residual: market broker fly - implied broker fly.
        Real flyError() const { return quote_->value() - impliedQuote(); }

      private:
        Handle<Quote> quote_;
        Real delta_;
        SS* smileSection_;

        // Cached market strangle data (set by initialize())
        Real callStrike_;
        Real putStrike_;
        Real marketStranglePrice_;
        bool initialized_;
    };

    // ---------------------------------------------------------------
    //  fxStrangleHelper inline / template implementation
    // ---------------------------------------------------------------

    template <class SS>
    fxStrangleHelper<SS>::fxStrangleHelper(Handle<Quote> brokerFlyQuote, Real delta)
    : quote_(std::move(brokerFlyQuote)), delta_(delta), smileSection_(nullptr),
      callStrike_(0.0), putStrike_(0.0), marketStranglePrice_(0.0), initialized_(false) 
    {
        registerWith(quote_);
    }

    template <class SS>
    fxStrangleHelper<SS>::fxStrangleHelper(Real brokerFlyQuote, Real delta)
    : quote_(makeQuoteHandle(brokerFlyQuote)), delta_(delta), smileSection_(nullptr), 
      callStrike_(0.0), putStrike_(0.0), marketStranglePrice_(0.0), initialized_(false) 
    {}

    template <class SS>
    void fxStrangleHelper<SS>::setSmileSection(SS* ss) 
    {
        QL_REQUIRE(ss != nullptr, "null smile section given");
        smileSection_ = ss;
        initialized_ = false;
    }

    template <class SS>
    void fxStrangleHelper<SS>::initialize() 
    {
        QL_REQUIRE(smileSection_ != nullptr, "smile section not set");

        Real atmVol = smileSection_->atm_->value();
        Real strdVol = atmVol + quote_->value();
        Real htau = std::sqrt(smileSection_->exerciseTime());
        Real w = strdVol * htau;

        Real spotVal = smileSection_->spot()->value();
        Real ddom = smileSection_->ddom_;
        Real dfor = smileSection_->dfor_;
        Real fwd = smileSection_->fwd_;
        DeltaVolQuote::DeltaType dt = smileSection_->deltaType();

        callStrike_ = BlackDeltaCalculator(Option::Call, dt, spotVal, ddom, dfor, w)
                          .strikeFromDelta(delta_);
        putStrike_ = BlackDeltaCalculator(Option::Put, dt, spotVal, ddom, dfor, w)
                         .strikeFromDelta(-delta_);

        marketStranglePrice_ = BlackCalculator(Option::Call, callStrike_, fwd, w).value() +
                                 BlackCalculator(Option::Put, putStrike_, fwd, w).value();

        initialized_ = true;
    }

    template <class SS>
    Real fxStrangleHelper<SS>::impliedQuote() const 
    {
        QL_REQUIRE(initialized_, "fxStrangleHelper not initialized");

        Real htau = std::sqrt(smileSection_->exerciseTime());
        Real fwd = smileSection_->fwd_;
        Real atmVol = smileSection_->atm_->value();

        // Price the strangle using the calibrated smile
        Real vc = smileSection_->volByStrike(callStrike_);
        Real vp = smileSection_->volByStrike(putStrike_);

        Real smileStranglePrice = BlackCalculator(Option::Call, callStrike_, fwd, vc * htau).value() +
                                    BlackCalculator(Option::Put, putStrike_, fwd, vp * htau).value();

        // Convert back to a broker-fly vol: find sigma_bf such that
        // strangle priced at atm + sigma_bf = smileStranglePrice
        auto priceError = [&](Real bf) {
            Real w = (atmVol + bf) * htau;
            return BlackCalculator(Option::Call, callStrike_, fwd, w).value() +
                   BlackCalculator(Option::Put, putStrike_, fwd, w).value() -
                   smileStranglePrice;
        };

        Brent solver;
        solver.setMaxEvaluations(1000);
        Real guess = quote_->value();
        return solver.solve(priceError, 1.0e-12, guess, guess * 0.5, guess * 2.0);
    }


    class fxSmileSection : public SmileSection, public LazyObject {
      public:
        enum FlyType {
            SmileStrangle, // Market fly
            MarketStrangle // Broker Fly
        };

        // ctor from market quotes for specific date
        fxSmileSection(const Date& exerciseDate,
                       const Handle<Quote>& spot,
                       const Handle<Quote>& atm,
                       const std::vector<Handle<Quote>>& rrs,
                       const std::vector<Handle<Quote>>& bfs,
                       const std::vector<Real>& deltas,
                       const Handle<YieldTermStructure>& foreignDiscount,
                       const Handle<YieldTermStructure>& domesticDiscount,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       FlyType flyType,
                       const DayCounter& dayCounter = DayCounter(),
                       const Date& referenceDate = Date());

        // ctor from market quotes with expiry time - floats with evaluation date
        fxSmileSection(Time exerciseTime,
                       const Handle<Quote>& spot,
                       const Handle<Quote>& atm,
                       const std::vector<Handle<Quote>>& rrs,
                       const std::vector<Handle<Quote>>& bfs,
                       const std::vector<Real>& deltas,
                       const Handle<YieldTermStructure>& foreignDiscount,
                       const Handle<YieldTermStructure>& domesticDiscount,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       FlyType flyType,
                       const DayCounter& dayCounter = DayCounter());

        // ctor from derived quotes for specific date
        fxSmileSection(const Date& exerciseDate,
                       const Handle<Quote>& spot,
                       const std::vector<Handle<DeltaVolQuote>>& quotes,
                       const Handle<YieldTermStructure>& foreignDiscount,
                       const Handle<YieldTermStructure>& domesticDiscount,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       FlyType flyType,
                       const DayCounter& dayCounter = DayCounter(),
                       const Date& referenceDate = Date());

        // ctor form derived quotes for expiry time - floats with evaluation date
        fxSmileSection(Time exerciseTime,
                       const Handle<Quote>& spot,
                       const std::vector<Handle<DeltaVolQuote>>& quotes,
                       const Handle<YieldTermStructure>& foreignDiscount,
                       const Handle<YieldTermStructure>& domesticDiscount,
                       DeltaVolQuote::DeltaType deltaType,
                       DeltaVolQuote::AtmType atmType,
                       FlyType flyType,
                       const DayCounter& dayCounter = DayCounter());

        //! \name Observer interface
        //@{
        void update() override;
        //@}

        //! \name SmileSection interface
        //@{
        virtual Real minStrike() const { calculate(); return minStrike_; };
        virtual Real maxStrike() const { calculate(); return maxStrike_; };
        virtual Real atmLevel() const { calculate(); return atmStrike_; };
        //@}

        // Conventions
        DeltaVolQuote::DeltaType deltaType() const { return deltaType_; };
        DeltaVolQuote::AtmType atmType() const { return atmType_; };
        FlyType flyType() const { return flyType_; };
        bool premiumAdjust() const {
            return (deltaType_ == DeltaVolQuote::PaSpot || deltaType_ == DeltaVolQuote::PaFwd);
        };

        // Introspection
        Handle<Quote> spot() const { return spot_; };
        Handle<Quote> atm() const { calculate(); return atm_; };
        Real forward() const {calculate(); return fwd_; };
        Handle<YieldTermStructure> foriegnDiscount() const { return foreignDiscount_; };
        Handle<YieldTermStructure> domesticDiscount() const { return domesticDiscount_; };
        bool isDeltaVolQuote() const { return isDeltaVolQuote_; };

        // Calibration
        virtual Volatility volByStrike(Rate strike) const = 0;
        virtual Volatility volByDelta(Real delta, Option::Type parity) const = 0;
        virtual Real deltaByStrike(Rate strike, Option::Type parity) const = 0;
        virtual Rate strikeByDelta(Real delta, Option::Type parity) const = 0;

        // Interpolation
        Real normedCallPrice(Rate strike) const;
        Real normedProbability(Rate strike, Real eps=1.0e-10) const;
        Rate strikeFromNormProb(Real q) const;

      private:
        //! \name LazyObject interface
        //@{
        void performCalculations() const override;
        //@}

        void registerWithMarketData();
        void calculateForward() const;
        void calculateAtm() const;
        void stripDeltaVolQuotes() const;
        virtual void adjustMinStrike() const;
        virtual void calibrate() const = 0;
        
        virtual Volatility volatilityImpl(Rate strike) const { return volByStrike(strike); };

        DeltaVolQuote::DeltaType deltaType_;
        DeltaVolQuote::AtmType atmType_;
        FlyType flyType_;
        bool isDeltaVolQuote_;

        Handle<Quote> spot_;
        std::vector<Handle<Quote>> rrs_;
        std::vector<Handle<Quote>> bfs_;
        std::vector<Real> deltas_;
        Handle<YieldTermStructure> foreignDiscount_;
        Handle<YieldTermStructure> domesticDiscount_;

        // Immutable inputs: set once at construction, never modified.
        // atmInput_ holds the market ATM quote for the RR/BF input path.
        // quotesInput_ holds the full set of DeltaVolQuotes for the DeltaVolQuote input path.
        const Handle<Quote> atmInput_;
        const std::vector<Handle<DeltaVolQuote>> quotesInput_;

        friend class fxStrangleHelper<fxSmileSection>;

      protected:
        mutable Real ddom_;
        mutable Real dfor_;
        mutable Real fwd_;

        mutable Real atmStrike_;
        mutable Real maxStrike_;
        mutable Real minStrike_;

        // Computed state: rebuilt on every calibration in stripDeltaVolQuotes().
        // atm_ is always set from atmInput_ (RR/BF path) or by calculateAtm() (DeltaVolQuote path).
        // quotes_ is always a workspace populated before each call to calibrate().
        mutable Handle<Quote> atm_;
        mutable std::vector<Handle<DeltaVolQuote>> quotes_;

    };

    inline void fxSmileSection::update() {
        SmileSection::update();
        LazyObject::update();
    }

    typedef boost::shared_ptr<fxSmileSection> fxSmileSectionPtr;

}  // namespace QuantLib

#endif