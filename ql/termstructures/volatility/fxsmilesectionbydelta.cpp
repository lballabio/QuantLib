#include <ql/math/solvers1d/brent.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/termstructures/volatility/fxsmilesectionbydelta.hpp>

namespace QuantLib {

    fxSmileSectionByDelta::fxSmileSectionByDelta(const Date& exerciseDate,
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

    fxSmileSectionByDelta::fxSmileSectionByDelta(Time exerciseTime,
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

    fxSmileSectionByDelta::fxSmileSectionByDelta(const Date& exerciseDate,
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

    fxSmileSectionByDelta::fxSmileSectionByDelta(Time exerciseTime,
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

    Volatility fxSmileSectionByDelta::volByStrike(Rate strike) const 
    {
        calculate();

        Real delta = deltaByStrike(strike, Option::Type::Put);
        return volByDelta(delta, Option::Type::Put);
    }

    Volatility fxSmileSectionByDelta::volByDelta(Real delta, Option::Type parity) const 
    {
        calculate();

        if (parity == Option::Call) {
            // call delta needs to be converted to put delta
            switch (deltaType()) { 
            
            case DeltaVolQuote::Spot:
                QL_REQUIRE(std::fabs(delta) <= dfor_, "Spot delta out of range.");
                delta -= dfor_;
                break;

            case DeltaVolQuote::Fwd:
                QL_REQUIRE(std::fabs(delta) <= 1.0, "Spot delta out of range.");
                delta -= 1;
                break;

            case DeltaVolQuote::PaSpot:
            case DeltaVolQuote::PaFwd:
                Volatility v = volByStrike(minStrike());
                Real maxCallDelta = BlackDeltaCalculator(Option::Call, deltaType(), spot()->value(),
                                                         ddom_, dfor_, v * sqrt(exerciseTime()))
                                        .deltaFromStrike(minStrike());
                QL_REQUIRE(delta <= maxCallDelta + QL_EPSILON, "Call delta out of range");
                if (std::fabs(delta - maxCallDelta) <= QL_EPSILON) {
                    return v;
                }

                // for other strikes we need to adjust the call delta to a put delta
                // however, we do not know the strike at which this conversion takes 
                // place, so estimate that through another root finding procedure
                Real pdx = delta;
                if (deltaType() == DeltaVolQuote::PaSpot) {
                    pdx -= dfor_ * minStrike() / fwd_;
                
                } else {
                    pdx -= minStrike() / fwd_;
                }

                auto deltaError = [&](Real d) {
                    Volatility v = volByDelta(d, Option::Type::Put);
                    Real k = BlackDeltaCalculator(Option::Type::Put, deltaType(), spot()->value(),
                                                ddom_, dfor_, v * sqrt(exerciseTime()))
                               .strikeFromDelta(d);
                    if (deltaType() == DeltaVolQuote::PaSpot) {
                        return delta - d - dfor_ * k / fwd_;

                    } else {
                        return delta - d - k / fwd_;
                    }
                };

                Brent solver;
                delta = solver.solve([&](Real d) { return deltaError(d); }, 1e-12, pdx,
                                      2. * pdx, pdx / 2.);
            }
        }

        // got vol as a function of delta
        return _volByDelta(delta, fwd_, exerciseTime(), params_);
    }

    Rate fxSmileSectionByDelta::strikeByDelta(Real delta, Option::Type parity) const 
    {
        calculate();

        Volatility v = volByDelta(delta, parity);
        BlackDeltaCalculator bdc(parity, deltaType(), spot()->value(), ddom_, dfor_,
                                 v * sqrt(exerciseTime()));
        return bdc.strikeFromDelta(delta);
    }

    Real fxSmileSectionByDelta::deltaByStrike(Rate strike, Option::Type parity) const 
    {
        calculate();

        // Since the slice is parameterized by put deltas, ignore parity and get
        // the put delta at the specified strike! This requires a root finding
        // procedure as we know the strike but not the vol!
        Rate d0 = BlackDeltaCalculator(Option::Type::Put, deltaType(), spot()->value(), ddom_,
                                       dfor_, atm()->value() * sqrt(exerciseTime()))
                      .deltaFromStrike(strike);
        Rate dmin = 0.0;
        switch (deltaType()) {
            case DeltaVolQuote::Spot:
                dmin = -dfor_ + QL_EPSILON;
                break;
            case DeltaVolQuote::Fwd:
                dmin = -1 + QL_EPSILON;
                break;
            case DeltaVolQuote::PaSpot:
            case DeltaVolQuote::PaFwd:
                dmin = 10000. * d0;
        }

        //Rate dmax = -0.001;

        auto strikeError = [&](Real delta) {
            Volatility v = volByDelta(delta, Option::Type::Put);
            return BlackDeltaCalculator(Option::Type::Put, deltaType(), spot()->value(), ddom_,
                                        dfor_, v * sqrt(exerciseTime()))
                       .strikeFromDelta(delta) -
                   strike;
        };

        Brent solver;
        Real d = solver.solve([&](Real delta) { return strikeError(delta); }, 1e-12, d0, dmin,
                              -QL_EPSILON);

        if (parity == Option::Type::Call) {
            Volatility v = volByDelta(d, Option::Type::Put);
            d = BlackDeltaCalculator(Option::Type::Call, deltaType(), spot()->value(), ddom_, dfor_,
                                     v * sqrt(exerciseTime()))
                    .deltaFromStrike(strike);
        }

        return d;
    }

    void fxSmileSectionByDelta::calibrate() const 
    {
        // TODO:
        // Uses deltaVolQuotes_ to calibrate the params!
        // Called repeatedly when ajsuting smile strangles!
        // 1. get initial parameters - define another abstract function?
        // 2. minimize the error - what optimization to use?
    }

}