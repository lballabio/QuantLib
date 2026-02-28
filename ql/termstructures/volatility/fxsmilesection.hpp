#ifndef quantlib_axl_fx_smile_section_hpp
#define quantlib_axl_fx_smile_section_hpp

#include <ql/experimental/fx/blackdeltacalculator.hpp>
#include <ql/experimental/fx/deltavolquote.hpp>
#include <ql/patterns/lazyobject.hpp>
#include <ql/quote.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <ql/termstructures/volatility/smilesection.hpp>
#include <ql/option.hpp>
#include <ql/math/solvers1d/bisection.hpp>


namespace QuantLib {

    template <class SS>
    class fxStrangleHelper : public Observer, public Observable {
      public:
        explicit fxStrangleHelper(Handle<Quote> quote);
        explicit fxStrangleHelper(Real quote);
        ~fxStrangleHelper() override = default;

        const Handle<Quote>& quote() const { return quote_; }
        virtual Real brokerFly();
        Real flyError() const { return quote_->value() - brokerFly(); }

        //! sets the smile section to be used for pricing
        /*! \warning Being a pointer and not a shared_ptr, the smile
                     section is not guaranteed to remain allocated
                     for the whole life of the strangle helper. It is
                     responsibility of the programmer to ensure that
                     the pointer remains valid. It is advised that
                     this method is called only inside the fx smile
                     section being calibrated, setting the pointer
                     to <b>this</b>, i.e., the term structure itself.
        */
        virtual void setSmileSection(SS* ss);

      protected:
        Handle<Quote> quote_;
        SS* smileSection_;
    };

    template <class SS>
    fxStrangleHelper<SS>::fxStrangleHelper(Handle<Quote> quote)
    : quote_(std::move(quote)), smileSection_(nullptr) {
        registerWith(quote_);
    }

    template <class SS>
    fxStrangleHelper<SS>::fxStrangleHelper(Real quote)
    : quote_(makeQuoteHandle(quote)), smileSection_(nullptr) {}

    template <class SS>
    void fxStrangleHelper<SS>::setSmileSection(SS* ss) {
        QL_REQUIRE(ss != nullptr, "null smile section gives");
        smileSection_ = ss;
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
        Handle<Quote> atm() const { return atm_; };
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

        friend class fxStrangleHelper<fxSmileSection>;

      protected:
        mutable Real ddom_;
        mutable Real dfor_;
        mutable Real fwd_;

        mutable Real atmStrike_;
        mutable Real maxStrike_;
        mutable Real minStrike_;

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