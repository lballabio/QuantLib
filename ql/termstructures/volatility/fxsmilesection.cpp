#include <ql/termstructures/volatility/fxsmilesection.hpp>

namespace QuantLib {

    fxSmileSection::fxSmileSection(const Date& exerciseDate,
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
                                   const DayCounter& dayCounter,
                                   const Date& referenceDate)
    : SmileSection(exerciseDate, dayCounter, referenceDate, ShiftedLognormal, 0.0), 
      spot_(spot), atm_(atm), rrs_(rrs), bfs_(bfs), deltas_(deltas), 
      foreignDiscount_(foreignDiscount), domesticDiscount_(domesticDiscount), 
      deltaType_(deltaType), atmType_(atmType), flyType_(flyType), isDeltaVolQuote_(false),
      quotes_(), maxStrike_(QL_MAX_REAL), minStrike_(QL_EPSILON)
    {
        QL_REQUIRE(rrs.size() == deltas.size(),
                   "risk reversal quotes must be the same size as deltas");
        QL_REQUIRE(bfs.size() == deltas.size(), "butterfly quotes must be the same size as deltas");
        registerWithMarketData();
    }

    fxSmileSection::fxSmileSection(Time exerciseTime,
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
                                   const DayCounter& dayCounter)
    : SmileSection(exerciseTime, dayCounter, ShiftedLognormal, 0.0), 
      spot_(spot), atm_(atm), rrs_(rrs), bfs_(bfs), deltas_(deltas), 
      foreignDiscount_(foreignDiscount), domesticDiscount_(domesticDiscount), 
      deltaType_(deltaType), atmType_(atmType), flyType_(flyType), isDeltaVolQuote_(false),
      quotes_(), maxStrike_(QL_MAX_REAL), minStrike_(QL_EPSILON)
    {
        QL_REQUIRE(rrs.size() == deltas.size(),
                   "risk reversal quotes must be the same size as deltas");
        QL_REQUIRE(bfs.size() == deltas.size(), "butterfly quotes must be the same size as deltas");
        registerWithMarketData();
    }

    fxSmileSection::fxSmileSection(const Date& exerciseDate,
                                   const Handle<Quote>& spot,
                                   const std::vector<Handle<DeltaVolQuote>>& quotes,
                                   const Handle<YieldTermStructure>& foreignDiscount,
                                   const Handle<YieldTermStructure>& domesticDiscount,
                                   DeltaVolQuote::DeltaType deltaType,
                                   DeltaVolQuote::AtmType atmType,
                                   FlyType flyType,
                                   const DayCounter& dayCounter,
                                   const Date& referenceDate)
    : SmileSection(exerciseDate, dayCounter, referenceDate, ShiftedLognormal, 0.0), 
      spot_(spot), quotes_(quotes), 
      foreignDiscount_(foreignDiscount), domesticDiscount_(domesticDiscount), 
      deltaType_(deltaType), atmType_(atmType), flyType_(flyType), isDeltaVolQuote_(true), 
      maxStrike_(QL_MAX_REAL), minStrike_(QL_EPSILON) 
    {
        registerWithMarketData();
    }

    fxSmileSection::fxSmileSection(Time exerciseTime,
                                   const Handle<Quote>& spot,
                                   const std::vector<Handle<DeltaVolQuote>>& quotes,
                                   const Handle<YieldTermStructure>& foreignDiscount,
                                   const Handle<YieldTermStructure>& domesticDiscount,
                                   DeltaVolQuote::DeltaType deltaType,
                                   DeltaVolQuote::AtmType atmType,
                                   FlyType flyType,
                                   const DayCounter& dayCounter)
    : SmileSection(exerciseTime, dayCounter, ShiftedLognormal, 0.0), 
      spot_(spot), quotes_(quotes), 
      foreignDiscount_(foreignDiscount), domesticDiscount_(domesticDiscount), 
      deltaType_(deltaType), atmType_(atmType), flyType_(flyType), isDeltaVolQuote_(true), 
      maxStrike_(QL_MAX_REAL), minStrike_(QL_EPSILON) 
    {
        registerWithMarketData();
    }

    void fxSmileSection::registerWithMarketData() 
    {
        registerWith(spot_);
        registerWith(foreignDiscount_);
        registerWith(domesticDiscount_);
        
        if (isDeltaVolQuote()) {
            for (auto& q : quotes_) registerWith(q);
        } 
        else {
            registerWith(atm_);
            for (auto& r : rrs_) registerWith(r);
            for (auto& b : bfs_) registerWith(b);
        }
    }

    void fxSmileSection::calculateForward() const {
        ddom_ = domesticDiscount_->discount(exerciseTime());
        dfor_ = foreignDiscount_->discount(exerciseTime());
        fwd_ = spot_->value() * dfor_ / ddom_;
    }

    void fxSmileSection::calculateAtm() const {
        calculate(); // should not be necc but force calibration!

        Real spot = spot_->value();
        Real ddom = domesticDiscount_->discount(exerciseTime());
        Real dfor = foreignDiscount_->discount(exerciseTime());
        Real fwd = spot * dfor / ddom;

        auto atmStrkikeError = [&](Real strike) {
            Volatility v = volByStrike(strike);
            Real k_atm = BlackDeltaCalculator(Option::Call, deltaType(), spot, ddom, dfor,
                                              v * sqrt(exerciseTime()))
                             .atmStrike(atmType());
            return strike - k_atm;
        };

        Brent solver;
        solver.setMaxEvaluations(10000);
        Rate k = solver.solve([&](Real strike) { return atmStrkikeError(strike); }, 1e-12, fwd, fwd / 10, 10 * fwd);

        atm_ = makeQuoteHandle(volByStrike(k));
    }

    void fxSmileSection::stripDeltaVolQuotes() const {

        if (isDeltaVolQuote()) {
            // if the input is a set of delta vol quotes (as opposed to RR and flies)
            // then simply call the calibration routine!
            calibrate();

            // when calibrating from quotes the atm is not set. so explicitly set it
            calculateAtm();
        } 
        else if (flyType() == FlyType::MarketStrangle) {
            // calibrate form RR and flies but flies are broker flies!
            // Broker flies - run a three/five point calibration to get the smile strangles
            if (deltas_.size() == 1) {
                // three point calibration
            } 
            else {
                // five point calibration
            }
        } 
        else {
            // Calibrate from RRs and flies, where the flies are smile strangles
            // This is easily handled algebraically. Convert to a set of delta-vol
            // quotes and then call the calibration routine!
            quotes_.clear();

            // handle the atm
            quotes_.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
                DeltaVolQuote(atm(), deltaType(), exerciseTime(), atmType()))));

            for (Size i = 0; i < deltas_.size(); ++i) {
                Real d = std::fabs(deltas_[i]);
                Real rr = rrs_[i]->value();
                Real bf = bfs_[i]->value();

                Volatility cVol = atm_->value() + bf + rr / 2.;
                Volatility pVol = atm_->value() + bf - rr / 2.;

                quotes_.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
                    DeltaVolQuote(d, makeQuoteHandle(cVol), exerciseTime(), deltaType_))));
                quotes_.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
                    DeltaVolQuote(-d, makeQuoteHandle(pVol), exerciseTime(), deltaType_))));
            }

            calibrate();
        }
        
    }

    void fxSmileSection::adjustMinStrike() const {
        if (premiumAdjust()) {
            calculate();  // should not be necc but force calibration!

            CumulativeNormalDistribution f;

            auto ddelta_dk = [&](Real strike) {
                Volatility w = volByStrike(strike) * std::sqrt(exerciseTime());
                Real d = std::log(fwd_ / strike) / w - w / 2.;
                return f(d) - f.derivative(d) / w;
            };

            QL_ASSERT((ddelta_dk(fwd_) < 0), "call delta should be well defined at the fwd");

            Real k_min = fwd_ * std::exp(-atm()->value() * exerciseTime());
            while (ddelta_dk(k_min) < 0) {
                k_min = 0.95 * k_min;
            }

            Brent solver;
            Rate k = solver.solve([&](Real strike) { return ddelta_dk(strike); }, 
                                  1e-12, (k_min + fwd_) / 2., k_min, fwd_);

            minStrike_ = k;
        }

        atmStrike_ = BlackDeltaCalculator(Option::Call, deltaType(), spot()->value(),
                                          domesticDiscount_->discount(exerciseTime()),
                                          foreignDiscount_->discount(exerciseTime()),
                                          atm()->value() * sqrt(exerciseTime()))
                         .atmStrike(atmType());

    }

    void fxSmileSection::performCalculations() const {
        calculateForward();
        stripDeltaVolQuotes();
        adjustMinStrike();
    }

    Real fxSmileSection::normedCallPrice(Rate strike) const {
        calculate();

        Real w = volByStrike(strike) * std::sqrt(exerciseTime());
        BlackCalculator bc = BlackCalculator(Option::Call, strike, fwd_, w);
        return bc.value() / fwd_;
    }

    Real fxSmileSection::normedProbability(Rate strike, Real eps) const {
        QL_REQUIRE((eps > 0) && (eps < 1.), "eps should be between 0 and 1");

        calculate();
        
        Real ncp_dn = normedCallPrice(strike - fwd_ * eps);
        Real ncp_up = normedCallPrice(strike + fwd_ * eps);
        return (ncp_dn - ncp_up) / (2. * eps);
    }

    Rate fxSmileSection::strikeFromNormProb(Real q) const {
        QL_REQUIRE((q > 0.) && (q < 1.), "q should be between 0 and 1.");

        calculate();

        auto normProbError = [&](Rate strike) { 
            return 100 * (normedProbability(strike) - abs(q));
        };

        Bisection solver;
        solver.setMaxEvaluations(10000);
        return solver.solve([&](Rate strike) { return normProbError(strike); }, 
                                1e-12, fwd_, fwd_ / 10., fwd_ * 10.);
    }

    template <class SS>
    Real fxStrangleHelper<SS>::brokerFly() {

        // TODO: This requires work!
        QL_REQUIRE(smileSection_ != nullptr, "smile section not set");

        smileSection_->quotes_.clear();

        // handle the atm
        smileSection_->quotes_.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
            DeltaVolQuote(smileSection_->atm(), smileSection_->deltaType(),
                          smileSection_->exerciseTime(), smileSection_->atmType()))));

        Real alpha = quote_->value() / smileSection_->bfs_[0]->value();

        for (Size i = 0; i < deltas_.size(); ++i) {
            Real d = std::fabs(smileSection_->deltas_[i]);
            Real rr = smileSection_->rrs_[i]->value();
            Real bf = smileSection_->bfs_[i]->value();

            Volatility cVol = smileSection_->atm_->value() + alpha * bf + rr / 2.;
            Volatility pVol = smileSection_->atm_->value() + alpha * bf - rr / 2.;

            smileSection_->quotes_.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
                DeltaVolQuote(d, makeQuoteHandle(cVol), exerciseTime(), deltaType_))));
            smileSection_->quotes_.push_back(Handle<DeltaVolQuote>(ext::make_shared<DeltaVolQuote>(
                DeltaVolQuote(-d, makeQuoteHandle(pVol), exerciseTime(), deltaType_))));
        }

        smileSection_->calibrate();

    }

}