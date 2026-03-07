#include <ql/math/distributions/normaldistribution.hpp>
#include <ql/math/optimization/constraint.hpp>
#include <ql/math/optimization/endcriteria.hpp>
#include <ql/math/optimization/levenbergmarquardt.hpp>
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
        Rate kmax = k0 * 10;

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
        QL_REQUIRE(!quotes_.empty(), "no delta-vol quotes to calibrate against");

        const Real htau = std::sqrt(exerciseTime());
        const Real spotVal = spot()->value();
        const Real fwd = fwd_;
        const Real ddom = ddom_;
        const Real dfor = dfor_;
        const DeltaVolQuote::DeltaType dt = deltaType();
        const Time tau = exerciseTime();

        // Precompute target vols and strikes from delta-vol quotes
        std::vector<Real> targetVols(quotes_.size());
        std::vector<Real> strikes(quotes_.size());

        for (Size i = 0; i < quotes_.size(); ++i) 
        {
            Real vol = quotes_[i]->value();
            Real w = vol * htau;
            targetVols[i] = vol;

            if (quotes_[i]->atmType() == DeltaVolQuote::AtmNull) 
            {
                Option::Type ot = (quotes_[i]->delta() < 0) ? Option::Put : Option::Call;
                strikes[i] = BlackDeltaCalculator(ot, dt, spotVal, ddom, dfor, w).strikeFromDelta(quotes_[i]->delta());
            } 
            else 
            {
                strikes[i] = BlackDeltaCalculator(Option::Call, dt, spotVal, ddom, dfor, w).atmStrike(quotes_[i]->atmType());
            }
        }

        // Cost function: residual = model_vol(strike_i) - target_vol_i
        auto costValues = [&](const Array& x) -> Array {
            std::vector<Real> p(x.begin(), x.end());
            Array residuals(quotes_.size());
            for (Size i = 0; i < quotes_.size(); ++i) 
            {
                residuals[i] = _volByStrike(strikes[i], fwd, tau, p) - targetVols[i];
            }
            return residuals;
        };

        SimpleCostFunction<decltype(costValues)> costFunction(costValues);
        NoConstraint constraint;
        Array guess = initialParams();

        Problem problem(costFunction, constraint, guess);
        LevenbergMarquardt lm;
        EndCriteria endCriteria(1000, 100, 1.0e-12, 1.0e-12, 1.0e-12);
        lm.minimize(problem, endCriteria);

        const Array& solution = problem.currentValue();
        params_.assign(solution.begin(), solution.end());
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
        params_.reserve(3);
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
        params_.reserve(3);
    }

    Array polynomialSmileSection::initialParams() const
    {
        // vol = exp(a*x^2 + b*x + c), at ATM x = Phi(0) = 0.5
        // with a=b=0, c = log(atm_vol)
        Array guess(3);
        guess[0] = 0.0;
        guess[1] = 0.0;
        guess[2] = std::log(atm_->value());
        return guess;
    }

    Volatility polynomialSmileSection::_volByStrike(Rate strike,
                                                    Real fwd,
                                                    Time tau,
                                                    const std::vector<Real>& params) const
    {
        CumulativeNormalDistribution f;
        Real x = f(std::log(fwd / strike) / (atm_->value() * std::sqrt(tau)));
        return std::exp(params[0] * x * x + params[1] * x + params[2]);
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

    Array fxSabrSmileSection::initialParams() const
    {
        // SABR params: alpha, nu, rho (beta fixed at 1)
        Array guess(3);
        guess[0] = atm_->value();  // alpha ~ atm vol for beta=1
        guess[1] = 0.5;            // nu
        guess[2] = 0.0;            // rho
        return guess;
    }

    Volatility fxSabrSmileSection::_volByStrike(Rate strike,
                                                Real fwd,
                                                Time tau,
                                                const std::vector<Real>& params) const
    {
        return unsafeShiftedSabrVolatility(strike, fwd, tau,
                                           params[0], 1.0, params[1], params[2],
                                           0.0, volatilityType());
    }
    //@}

}