#ifndef quantlib_axl_fx_smile_section_strike_hpp
#define quantlib_axl_fx_smile_section_strike_hpp

#include <ql/termstructures/volatility/fxsmilesection.hpp>

namespace QuantLib {

    class fxSmileSectionByStrike : public fxSmileSection {
      public:
        // ctor from market quotes for specific date
        fxSmileSectionByStrike(const Date& exerciseDate,
                               const Handle<Quote>& spot,
                               const Handle<Quote>& atm,
                               const std::vector<Handle<Quote>>& rrs,
                               const std::vector<Handle<Quote>>& bfs,
                               const std::vector<Real>& deltas,
                               const Handle<YieldTermStructure>& foreignDiscount,
                               const Handle<YieldTermStructure>& domesticDiscount,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               fxSmileSection::FlyType flyType,
                               const DayCounter& dayCounter = DayCounter(),
                               const Date& referenceDate = Date());

        // ctor from market quotes with expiry time - floats with evaluation date
        fxSmileSectionByStrike(Time exerciseTime,
                               const Handle<Quote>& spot,
                               const Handle<Quote>& atm,
                               const std::vector<Handle<Quote>>& rrs,
                               const std::vector<Handle<Quote>>& bfs,
                               const std::vector<Real>& deltas,
                               const Handle<YieldTermStructure>& foreignDiscount,
                               const Handle<YieldTermStructure>& domesticDiscount,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               fxSmileSection::FlyType flyType,
                               const DayCounter& dayCounter = DayCounter());

        // ctor from derived quotes for specific date
        fxSmileSectionByStrike(const Date& exerciseDate,
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
        fxSmileSectionByStrike(Time exerciseTime,
                               const Handle<Quote>& spot,
                               const std::vector<Handle<DeltaVolQuote>>& quotes,
                               const Handle<YieldTermStructure>& foreignDiscount,
                               const Handle<YieldTermStructure>& domesticDiscount,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               FlyType flyType,
                               const DayCounter& dayCounter = DayCounter());

        //! \name fxSmileSection interface
        //@{
        Volatility volByStrike(Rate strike) const;
        Volatility volByDelta(Real delta, Option::Type parity) const;
        Real deltaByStrike(Rate strike, Option::Type parity) const;
        Rate strikeByDelta(Real delta, Option::Type parity) const;
        //@}

      private:
        virtual Volatility _volByStrike(Real strike, 
                                        Real fwd, 
                                        Time tau, 
                                        const std::vector<Real>& params) const = 0;

        //! \name fxSmileSection interface
        //@{
        virtual void calibrate() const;
        //@}

      protected:
        mutable std::vector<Real> params_;

    };

    typedef boost::shared_ptr<fxSmileSectionByStrike> fxSmileSectionByStrikePtr;



    class polynomialSmileSection : public fxSmileSectionByStrike {
      public:
        // ctor from market quotes for specific date
        polynomialSmileSection(const Date& exerciseDate,
                               const Handle<Quote>& spot,
                               const Handle<Quote>& atm,
                               const std::vector<Handle<Quote>>& rrs,
                               const std::vector<Handle<Quote>>& bfs,
                               const std::vector<Real>& deltas,
                               const Handle<YieldTermStructure>& foreignDiscount,
                               const Handle<YieldTermStructure>& domesticDiscount,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               fxSmileSection::FlyType flyType,
                               const DayCounter& dayCounter = DayCounter(),
                               const Date& referenceDate = Date());

        // ctor from market quotes with expiry time - floats with evaluation date
        polynomialSmileSection(Time exerciseTime,
                               const Handle<Quote>& spot,
                               const Handle<Quote>& atm,
                               const std::vector<Handle<Quote>>& rrs,
                               const std::vector<Handle<Quote>>& bfs,
                               const std::vector<Real>& deltas,
                               const Handle<YieldTermStructure>& foreignDiscount,
                               const Handle<YieldTermStructure>& domesticDiscount,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               fxSmileSection::FlyType flyType,
                               const DayCounter& dayCounter = DayCounter());

        // ctor from derived quotes for specific date
        polynomialSmileSection(const Date& exerciseDate,
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
        polynomialSmileSection(Time exerciseTime,
                               const Handle<Quote>& spot,
                               const std::vector<Handle<DeltaVolQuote>>& quotes,
                               const Handle<YieldTermStructure>& foreignDiscount,
                               const Handle<YieldTermStructure>& domesticDiscount,
                               DeltaVolQuote::DeltaType deltaType,
                               DeltaVolQuote::AtmType atmType,
                               FlyType flyType,
                               const DayCounter& dayCounter = DayCounter());

        // Introspection
        Real a() const { return params_[0]; };
        Real b() const { return params_[1]; };
        Real c() const { return params_[2]; };

      private : 
        //! \name fxSmileSectionByStrike interface
        //@{
        virtual Volatility _volByStrike(Real strike,
                                        Real fwd,
                                        Time tau,
                                        const std::vector<Real>& params) const override;
        //@}
    };


    class fxSabrSmileSection : public fxSmileSectionByStrike {
      public:
        // ctor from market quotes for specific date
        fxSabrSmileSection(const Date& exerciseDate,
                           const Handle<Quote>& spot,
                           const Handle<Quote>& atm,
                           const std::vector<Handle<Quote>>& rrs,
                           const std::vector<Handle<Quote>>& bfs,
                           const std::vector<Real>& deltas,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           fxSmileSection::FlyType flyType,
                           const DayCounter& dayCounter = DayCounter(),
                           const Date& referenceDate = Date());

        // ctor from market quotes with expiry time - floats with evaluation date
        fxSabrSmileSection(Time exerciseTime,
                           const Handle<Quote>& spot,
                           const Handle<Quote>& atm,
                           const std::vector<Handle<Quote>>& rrs,
                           const std::vector<Handle<Quote>>& bfs,
                           const std::vector<Real>& deltas,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           fxSmileSection::FlyType flyType,
                           const DayCounter& dayCounter = DayCounter());

        // ctor from derived quotes for specific date
        fxSabrSmileSection(const Date& exerciseDate,
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
        fxSabrSmileSection(Time exerciseTime,
                           const Handle<Quote>& spot,
                           const std::vector<Handle<DeltaVolQuote>>& quotes,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           FlyType flyType,
                           const DayCounter& dayCounter = DayCounter());

        // Introspection
        Real alpha() const { return params_[0]; };
        Real beta() const { return 1.0; };
        Real nu() const { return params_[1]; };
        Real rho() const { return params_[2]; };

      private:
        //! \name fxSmileSectionByStrike interface
        //@{
        virtual Volatility _volByStrike(Real strike,
                                        Real fwd,
                                        Time tau,
                                        const std::vector<Real>& params) const override;
        //@}

    };

} // namespace QuantLib

#endif