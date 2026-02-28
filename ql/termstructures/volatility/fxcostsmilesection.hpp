#ifndef quantlib_axl_cost_fx_smile_section_hpp
#define quantlib_axl_cost_fx_smile_section_hpp

#include <ql/math/polynomialmathfunction.hpp>
#include <ql/math/quadratic.hpp>
#include <ql/termstructures/volatility/fxsmilesectionbystrike.hpp>

namespace QuantLib {

    class cubic : public PolynomialFunction {
      public:
          cubic(const std::vector<Real>& coeff): PolynomialFunction(coeff) {
            QL_REQUIRE(coeff.size() == 4,
                       "cubic requires four coefficients to initialize");

        }

        Integer roots(std::vector<Real>& zeros);
    };

    class quartic : public PolynomialFunction {
      public:
        quartic(const std::vector<Real>& coeff) : PolynomialFunction(coeff) {
            QL_REQUIRE(coeff.size() == 5, "quartic requires five coefficients to initialize");
        }

        Integer roots(std::vector<Real>& zeros);
    };

    class fxCostSmileSection : public fxSmileSectionByStrike {
      public:
        // ctor from market quotes for specific date
        fxCostSmileSection(const Date& exerciseDate,
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
                           const Date& referenceDate = Date(),
                           bool weightedCalibrationFlag = true);

        // ctor from market quotes with expiry time - floats with evaluation date
        fxCostSmileSection(Time exerciseTime,
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
                           bool weightedCalibrationFlag = true);

        // ctor from derived quotes for specific date
        fxCostSmileSection(const Date& exerciseDate,
                           const Handle<Quote>& spot,
                           const std::vector<Handle<DeltaVolQuote>>& quotes,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           FlyType flyType,
                           const DayCounter& dayCounter = DayCounter(),
                           const Date& referenceDate = Date(),
                           bool weightedCalibrationFlag = true);

        // ctor form derived quotes for expiry time - floats with evaluation date
        fxCostSmileSection(Time exerciseTime,
                           const Handle<Quote>& spot,
                           const std::vector<Handle<DeltaVolQuote>>& quotes,
                           const Handle<YieldTermStructure>& foreignDiscount,
                           const Handle<YieldTermStructure>& domesticDiscount,
                           DeltaVolQuote::DeltaType deltaType,
                           DeltaVolQuote::AtmType atmType,
                           FlyType flyType,
                           const DayCounter& dayCounter = DayCounter(),
                           bool weightedCalibrationFlag = true);

        bool weightedCalibration() const { return weightedCalibrationFlag_; };

      private:
        //! \name fxSmileSection interface
        //@{
        // force child classes to redefine the calibration routine!
        // for cost models this can be done epxlicitly while the base
        // class uses optimization routines. So redefine explicitly!
        virtual void calibrate() const = 0;
        //@}

        bool weightedCalibrationFlag_;

    };


    class fxCostSmileSectionFlatDynamics : public fxCostSmileSection {
      public:
        // ctor from market quotes for specific date
        fxCostSmileSectionFlatDynamics(const Date& exerciseDate,
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
                                       const Date& referenceDate = Date(),
                                       bool weightedCalibrationFlag = true);

        // ctor from market quotes with expiry time - floats with evaluation date
        fxCostSmileSectionFlatDynamics(Time exerciseTime,
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
                                       bool weightedCalibrationFlag = true);

        // ctor from derived quotes for specific date
        fxCostSmileSectionFlatDynamics(const Date& exerciseDate,
                                       const Handle<Quote>& spot,
                                       const std::vector<Handle<DeltaVolQuote>>& quotes,
                                       const Handle<YieldTermStructure>& foreignDiscount,
                                       const Handle<YieldTermStructure>& domesticDiscount,
                                       DeltaVolQuote::DeltaType deltaType,
                                       DeltaVolQuote::AtmType atmType,
                                       FlyType flyType,
                                       const DayCounter& dayCounter = DayCounter(),
                                       const Date& referenceDate = Date(),
                                       bool weightedCalibrationFlag = true);

        // ctor form derived quotes for expiry time - floats with evaluation date
        fxCostSmileSectionFlatDynamics(Time exerciseTime,
                                       const Handle<Quote>& spot,
                                       const std::vector<Handle<DeltaVolQuote>>& quotes,
                                       const Handle<YieldTermStructure>& foreignDiscount,
                                       const Handle<YieldTermStructure>& domesticDiscount,
                                       DeltaVolQuote::DeltaType deltaType,
                                       DeltaVolQuote::AtmType atmType,
                                       FlyType flyType,
                                       const DayCounter& dayCounter = DayCounter(),
                                       bool weightedCalibrationFlag = true);

      private:
        //! \name fxSmileSectionByStrike interface
        //@{
        virtual Volatility _volByStrike(Real strike,
                                        Real fwd,
                                        Time tau,
                                        const std::vector<Real>& params) const;
        //@}

        //! \name fxCostSmileSection interface
        //@{
        virtual void calibrate() const;
        //@}

    };

    class fxCostSmileSectionScaledDynamics : public fxCostSmileSection {
      public:
        // ctor from market quotes for specific date
        fxCostSmileSectionScaledDynamics(const Date& exerciseDate,
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
                                         const Date& referenceDate = Date(),
                                         bool weightedCalibrationFlag = true);

        // ctor from market quotes with expiry time - floats with evaluation date
        fxCostSmileSectionScaledDynamics(Time exerciseTime,
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
                                         bool weightedCalibrationFlag = true);

        // ctor from derived quotes for specific date
        fxCostSmileSectionScaledDynamics(const Date& exerciseDate,
                                         const Handle<Quote>& spot,
                                         const std::vector<Handle<DeltaVolQuote>>& quotes,
                                         const Handle<YieldTermStructure>& foreignDiscount,
                                         const Handle<YieldTermStructure>& domesticDiscount,
                                         DeltaVolQuote::DeltaType deltaType,
                                         DeltaVolQuote::AtmType atmType,
                                         FlyType flyType,
                                         const DayCounter& dayCounter = DayCounter(),
                                         const Date& referenceDate = Date(),
                                         bool weightedCalibrationFlag = true);

        // ctor form derived quotes for expiry time - floats with evaluation date
        fxCostSmileSectionScaledDynamics(Time exerciseTime,
                                         const Handle<Quote>& spot,
                                         const std::vector<Handle<DeltaVolQuote>>& quotes,
                                         const Handle<YieldTermStructure>& foreignDiscount,
                                         const Handle<YieldTermStructure>& domesticDiscount,
                                         DeltaVolQuote::DeltaType deltaType,
                                         DeltaVolQuote::AtmType atmType,
                                         FlyType flyType,
                                         const DayCounter& dayCounter = DayCounter(),
                                         bool weightedCalibrationFlag = true);

      private:
        //! \name fxSmileSectionByStrike interface
        //@{
        virtual Volatility _volByStrike(Real strike, 
                                        Real fwd, 
                                        Time tau, 
                                        const std::vector<Real>& params) const;
        //@}

        //! \name fxCostSmileSection interface
        //@{
        virtual void calibrate() const;
        //@}
    };

} // namespace QuantLib

#endif
