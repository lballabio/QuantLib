#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/sabr.hpp>
#include <ql/termstructures/volatility/fxsmilesectionbystrike.hpp>

namespace QuantLib {

    fxSmileSectionByStrike::fxSmileSectionByStrike(const Date& exerciseDate,
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
                                                   const DayCounter& dayCounter,
                                                   const Date& referenceDate)
    : fxSmileSection(exerciseDate, spot, atm, rrs, bfs, deltas,
                     foreignDiscount, domesticDiscount,
                     deltaType, atmType, flyType, dayCounter, referenceDate),
      params_() {}

    fxSmileSectionByStrike::fxSmileSectionByStrike(Time exerciseTime,
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
                                                   const DayCounter& dayCounter)
    : fxSmileSection(exerciseTime, spot, atm, rrs, bfs, deltas,
                     foreignDiscount, domesticDiscount,
                     deltaType, atmType, flyType, dayCounter),
      params_() {}

    fxSmileSectionByStrike::fxSmileSectionByStrike(const Date& exerciseDate,
                                                   const Handle<Quote>& spot,
                                                   const std::vector<Handle<DeltaVolQuote>>& quotes,
                                                   const Handle<YieldTermStructure>& foreignDiscount,
                                                   const Handle<YieldTermStructure>& domesticDiscount,
                                                   DeltaVolQuote::DeltaType deltaType,
                                                   DeltaVolQuote::AtmType atmType,
                                                   fxSmileSection::FlyType flyType,
                                                   const DayCounter& dayCounter,
                                                   const Date& referenceDate)
    : fxSmileSection(exerciseDate, spot, quotes,
                     foreignDiscount, domesticDiscount,
                     deltaType, atmType, flyType, dayCounter, referenceDate),
      params_() {}

    fxSmileSectionByStrike::fxSmileSectionByStrike(Time exerciseTime,
                                                   const Handle<Quote>& spot,
                                                   const std::vector<Handle<DeltaVolQuote>>& quotes,
                                                   const Handle<YieldTermStructure>& foreignDiscount,
                                                   const Handle<YieldTermStructure>& domesticDiscount,
                                                   DeltaVolQuote::DeltaType deltaType,
                                                   DeltaVolQuote::AtmType atmType,
                                                   fxSmileSection::FlyType flyType,
                                                   const DayCounter& dayCounter)
    : fxSmileSection(exerciseTime, spot, quotes,
                     foreignDiscount, domesticDiscount,
                     deltaType, atmType, flyType, dayCounter),
      params_() {}

    Volatility fxSmileSectionByStrike::volByStrike(Rate strike) const 
    {
        calculate();
        return _volByStrike(strike, fwd_, exerciseTime(), params_);
    }

    Volatility fxSmileSectionByStrike::volByDelta(Real delta, Option::Type parity) const 
    {
        calculate();
        return volByStrike(strikeByDelta(delta, parity));
    }

    Real fxSmileSectionByStrike::deltaByStrike(Rate strike, Option::Type parity) const 
    {
        calculate();

        Volatility v = volByStrike(strike);
        return BlackDeltaCalculator(parity, deltaType(), spot()->value(), ddom_, dfor_,
                                    v * sqrt(exerciseTime()))
            .deltaFromStrike(strike);
    }

    Rate fxSmileSectionByStrike::strikeByDelta(Real delta, Option::Type parity) const 
    {
        calculate();

        if (parity == Option::Call && premiumAdjust()) {
            Real maxCallDelta = deltaByStrike(minStrike(), Option::Call);
            QL_REQUIRE(delta <= maxCallDelta + QL_EPSILON, "Call delta out of range");
            if (std::fabs(delta - maxCallDelta) <= QL_EPSILON) {
                return minStrike();
            }
        }

        Rate k0 = BlackDeltaCalculator(parity, deltaType(), spot()->value(), ddom_, dfor_,
                                       atm()->value() * sqrt(exerciseTime()))
                      .strikeFromDelta(delta);
        Rate kmin = (premiumAdjust() && parity==Option::Call) ? minStrike() : QL_EPSILON;
        Rate kmax = k0 * 10; // maxStrike();

        auto deltaError = [&](Real strike) {
            Volatility v = volByStrike(strike);
            Real d = BlackDeltaCalculator(parity, deltaType(), spot()->value(), ddom_, dfor_,
                                          v * sqrt(exerciseTime()))
                         .deltaFromStrike(strike);
            return d - delta;
        };

        Brent solver;
        solver.setMaxEvaluations(10000);
        Rate k = solver.solve([&](Real strike) { return deltaError(strike); }, 
                              1e-12, k0, kmin, kmax);
        return k;
    }

    void fxSmileSectionByStrike::calibrate() const 
    {
        // TODO: 
        // Uses deltaVolQuotes_ to calibrate the params!
        // Called repeatedly when ajsuting smile strangles!
        // 1. get initial parameters - define another abstract function?
        // 2. minimize the error - what optimization to use?
    }


    //! \name Polynomial smile section
    //@{
    polynomialSmileSection::polynomialSmileSection(const Date& exerciseDate,
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
                                                   const DayCounter& dayCounter,
                                                   const Date& referenceDate)
    : fxSmileSectionByStrike(exerciseDate, spot, atm, rrs, bfs, deltas,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter, referenceDate) 
    {
        params_.reserve(3);
    }

    polynomialSmileSection::polynomialSmileSection(Time exerciseTime,
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
                                                   const DayCounter& dayCounter)
    : fxSmileSectionByStrike(exerciseTime, spot, atm, rrs, bfs, deltas,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter) 
    {
        params_.reserve(3);
    }

    polynomialSmileSection::polynomialSmileSection(const Date& exerciseDate,
                                                   const Handle<Quote>& spot,
                                                   const std::vector<Handle<DeltaVolQuote>>& quotes,
                                                   const Handle<YieldTermStructure>& foreignDiscount,
                                                   const Handle<YieldTermStructure>& domesticDiscount,
                                                   DeltaVolQuote::DeltaType deltaType,
                                                   DeltaVolQuote::AtmType atmType,
                                                   fxSmileSection::FlyType flyType,
                                                   const DayCounter& dayCounter,
                                                   const Date& referenceDate)
    : fxSmileSectionByStrike(exerciseDate, spot, quotes,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter, referenceDate)
    {
        params_.reserve(4);
    }

    polynomialSmileSection::polynomialSmileSection(Time exerciseTime,
                                                   const Handle<Quote>& spot,
                                                   const std::vector<Handle<DeltaVolQuote>>& quotes,
                                                   const Handle<YieldTermStructure>& foreignDiscount,
                                                   const Handle<YieldTermStructure>& domesticDiscount,
                                                   DeltaVolQuote::DeltaType deltaType,
                                                   DeltaVolQuote::AtmType atmType,
                                                   fxSmileSection::FlyType flyType,
                                                   const DayCounter& dayCounter)
    : fxSmileSectionByStrike(exerciseTime, spot, quotes,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter)
    {
        params_.reserve(4);
    }

    Volatility polynomialSmileSection::_volByStrike(Rate strike, 
                                                    Real fwd, 
                                                    Time tau, 
                                                    const std::vector<Real>& params) const 
    {
        CumulativeNormalDistribution f;
        Real x = f(std::log(fwd / strike) / (params[0] * std::sqrt(tau)));
        return std::exp(params[1] * x * x + params[2] * x + params[3]);
    }
    //@}


    //! \name SABR smile section
    //@{
    fxSabrSmileSection::fxSabrSmileSection(const Date& exerciseDate,
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
                                           const DayCounter& dayCounter,
                                           const Date& referenceDate)
    : fxSmileSectionByStrike(exerciseDate, spot, atm, rrs, bfs, deltas,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter, referenceDate) 
    {
        params_.reserve(3);
    }

    fxSabrSmileSection::fxSabrSmileSection(Time exerciseTime,
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
                                           const DayCounter& dayCounter)
    : fxSmileSectionByStrike(exerciseTime, spot, atm,  rrs, bfs, deltas,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter) 
    {
        params_.reserve(3);
    }

    fxSabrSmileSection::fxSabrSmileSection(const Date& exerciseDate,
                                           const Handle<Quote>& spot,
                                           const std::vector<Handle<DeltaVolQuote>>& quotes,
                                           const Handle<YieldTermStructure>& foreignDiscount,
                                           const Handle<YieldTermStructure>& domesticDiscount,
                                           DeltaVolQuote::DeltaType deltaType,
                                           DeltaVolQuote::AtmType atmType,
                                           fxSmileSection::FlyType flyType,
                                           const DayCounter& dayCounter,
                                           const Date& referenceDate)
    : fxSmileSectionByStrike(exerciseDate, spot, quotes,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter, referenceDate) 
    {
        params_.reserve(3);
    }

    fxSabrSmileSection::fxSabrSmileSection(Time exerciseTime,
                                           const Handle<Quote>& spot,
                                           const std::vector<Handle<DeltaVolQuote>>& quotes,
                                           const Handle<YieldTermStructure>& foreignDiscount,
                                           const Handle<YieldTermStructure>& domesticDiscount,
                                           DeltaVolQuote::DeltaType deltaType,
                                           DeltaVolQuote::AtmType atmType,
                                           fxSmileSection::FlyType flyType,
                                           const DayCounter& dayCounter)
    : fxSmileSectionByStrike(exerciseTime, spot, quotes,
                             foreignDiscount, domesticDiscount,
                             deltaType, atmType, flyType, dayCounter) 
    {
        params_.reserve(3);
    }

    Volatility fxSabrSmileSection::_volByStrike(Rate strike, 
                                                Real fwd, 
                                                Time tau, 
                                                const std::vector<Real>& params) const 
    {
        // TODO: ensure parameters are good!
        return unsafeShiftedSabrVolatility(strike, fwd, tau, 
                                           params[0], 1.0, params[1], params[2],
                                           0.0, volatilityType());
        
    }
    //@}

}