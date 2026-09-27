/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/currencies/america.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/errors.hpp>
#include <ql/experimental/commodities/commoditysettings.hpp>
#include <ql/experimental/commodities/energybasisswap.hpp>
#include <ql/experimental/commodities/petroleumunitsofmeasure.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/time/calendars/nullcalendar.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace {

    class SavedCommoditySettings {
      public:
        SavedCommoditySettings()
        : currency_(CommoditySettings::instance().currency()),
          unitOfMeasure_(CommoditySettings::instance().unitOfMeasure()) {}

        ~SavedCommoditySettings() {
            CommoditySettings::instance().currency() = currency_;
            CommoditySettings::instance().unitOfMeasure() = unitOfMeasure_;
        }

      private:
        Currency currency_;
        UnitOfMeasure unitOfMeasure_;
    };

    class EnergyBasisSwapTestFixture : public TopLevelFixture {
      public:
        EnergyBasisSwapTestFixture()
        : evaluationDate_(2, January, 2025), calendar_(NullCalendar()),
          commodityType_(NullCommodityType()) {
            Settings::instance().evaluationDate() = evaluationDate_;
            CommoditySettings::instance().currency() = USDCurrency();
            CommoditySettings::instance().unitOfMeasure() = BarrelUnitOfMeasure();
        }

        ext::shared_ptr<CommodityIndex> makeIndex(
            const std::string& name,
            const Currency& currency,
            const UnitOfMeasure& unitOfMeasure,
            std::vector<Date> dates,
            std::vector<Real> prices) const {
            auto curve = ext::make_shared<CommodityCurve>(
                name, commodityType_, currency, unitOfMeasure, calendar_, dates, prices);
            auto index = ext::make_shared<CommodityIndex>(
                name, commodityType_, currency, unitOfMeasure, calendar_, 1.0, curve,
                ext::shared_ptr<ExchangeContracts>(), 0);
            index->addFixing(evaluationDate_, prices.front());
            return index;
        }

        PricingPeriods periods(
            const Date& startDate,
            const Date& endDate,
            const Date& paymentDate,
            const Quantity& quantity) const {
            return {ext::make_shared<PricingPeriod>(
                startDate, endDate, paymentDate, quantity)};
        }

        CommodityUnitCost noBasis(
            const UnitOfMeasure& unitOfMeasure = BarrelUnitOfMeasure()) const {
            return CommodityUnitCost(Money(0.0, USDCurrency()), unitOfMeasure);
        }

        Handle<YieldTermStructure> flatCurve(Rate rate) const {
            ext::shared_ptr<YieldTermStructure> curve =
                ext::make_shared<FlatForward>(evaluationDate_, rate, Actual365Fixed());
            return Handle<YieldTermStructure>(curve);
        }

        ext::shared_ptr<YieldTermStructure> flatCurvePtr(Rate rate) const {
            return ext::make_shared<FlatForward>(evaluationDate_, rate, Actual365Fixed());
        }

        ext::shared_ptr<EnergyBasisSwap> makeSwap(
            const ext::shared_ptr<CommodityIndex>& payIndex,
            const ext::shared_ptr<CommodityIndex>& receiveIndex,
            const PricingPeriods& pricingPeriods,
            const CommodityUnitCost& basis,
            bool spreadToPayLeg = true,
            const ext::shared_ptr<SecondaryCosts>& secondaryCosts = nullptr) const {
            Handle<YieldTermStructure> zeroCurve = flatCurve(0.0);
            return makeSwapWithCurves(
                payIndex, receiveIndex, pricingPeriods, basis, spreadToPayLeg,
                secondaryCosts, zeroCurve, zeroCurve, zeroCurve);
        }

        ext::shared_ptr<EnergyBasisSwap> makeSwapWithCurves(
            const ext::shared_ptr<CommodityIndex>& payIndex,
            const ext::shared_ptr<CommodityIndex>& receiveIndex,
            const PricingPeriods& pricingPeriods,
            const CommodityUnitCost& basis,
            bool spreadToPayLeg,
            const ext::shared_ptr<SecondaryCosts>& secondaryCosts,
            const Handle<YieldTermStructure>& payCurve,
            const Handle<YieldTermStructure>& receiveCurve,
            const Handle<YieldTermStructure>& discountCurve) const {
            return ext::make_shared<EnergyBasisSwap>(
                calendar_, payIndex, payIndex, receiveIndex, spreadToPayLeg,
                USDCurrency(), USDCurrency(), pricingPeriods, basis, commodityType_,
                secondaryCosts, payCurve, receiveCurve, discountCurve);
        }

        const Date evaluationDate_;
        const Calendar calendar_;
        const CommodityType commodityType_;

      private:
        SavedCommoditySettings savedCommoditySettings_;
    };

}

BOOST_FIXTURE_TEST_SUITE(QuantLibTests, EnergyBasisSwapTestFixture)
BOOST_AUTO_TEST_SUITE(EnergyBasisSwapTests)

BOOST_AUTO_TEST_CASE(testBasisFxConvertsIntoCommodityCurrency) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("basis-fx-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("basis-fx-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {100.0, 100.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));

    ExchangeRateManager::instance().add(ExchangeRate(EURCurrency(), USDCurrency(), 2.0),
                                        evaluationDate_, Date(31, December, 2025));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods,
                         CommodityUnitCost(Money(10.0, EURCurrency()), BarrelUnitOfMeasure()));

    BOOST_CHECK_SMALL(swap->NPV() + 20.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testBasisPriceConvertsFromMegabarrelsToBarrels) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("basis-mb-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("basis-mb-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {0.0, 0.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    CommodityUnitCost basis(Money(10.0, USDCurrency()), MBUnitOfMeasure());
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, basis);

    BOOST_CHECK_SMALL(swap->NPV() + 0.01, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testBasisPriceConvertsFromBarrelsToMegabarrels) {
    CommoditySettings::instance().unitOfMeasure() = MBUnitOfMeasure();
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("basis-bbl-pay", USDCurrency(), MBUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("basis-bbl-receive", USDCurrency(), MBUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {0.0, 0.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, MBUnitOfMeasure(), 1.0));
    CommodityUnitCost basis(Money(10.0, USDCurrency()), BarrelUnitOfMeasure());
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, basis);

    BOOST_CHECK_SMALL(swap->NPV() + 10000.0, 1.0e-8);
}

BOOST_AUTO_TEST_CASE(testUnsupportedBasisPriceUnitRaisesAnError) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("unsupported-basis-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("unsupported-basis-receive", USDCurrency(),
                                  BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {0.0, 0.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    CommodityUnitCost basis(Money(10.0, USDCurrency()), MTUnitOfMeasure());
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, basis);

    BOOST_CHECK_THROW(swap->NPV(), Error);
    BOOST_CHECK(!swap->pricingErrors().empty());
    BOOST_CHECK_EQUAL(swap->pricingErrors().back().errorLevel, PricingError::Error);
}

BOOST_AUTO_TEST_CASE(testEmptyPricingPeriodsAreRejected) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("empty-periods-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("empty-periods-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {90.0, 90.0});

    BOOST_CHECK_THROW(makeSwap(payIndex, receiveIndex, {}, noBasis()), Error);
}

BOOST_AUTO_TEST_CASE(testQuantityIsConvertedFromPeriodUnitToBaseUnit) {
    CommoditySettings::instance().unitOfMeasure() = MBUnitOfMeasure();
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("quantity-pay", USDCurrency(), MBUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("quantity-receive", USDCurrency(), MBUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {1.0, 1.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1000.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis(MBUnitOfMeasure()), false);

    BOOST_CHECK_SMALL(swap->NPV() - 1.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testQuantityConversionUsesSourceDirection) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("forward-quantity-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("forward-quantity-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {1.0, 1.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, MBUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis(), false);

    BOOST_CHECK_SMALL(swap->NPV() - 1000.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testPayIndexPriceConvertsFromMegabarrelsToBarrels) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("pay-price-mb", USDCurrency(), MBUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {1.0, 1.0});
    auto receiveIndex = makeIndex("receive-price-bbl", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {0.0, 0.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1000.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_SMALL(swap->NPV() + 1.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testReceiveIndexPriceConvertsFromMegabarrelsToBarrels) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("pay-price-bbl", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("receive-price-mb", USDCurrency(), MBUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {1.0, 1.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1000.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis(), false);

    BOOST_CHECK_SMALL(swap->NPV() - 1.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testUnsupportedQuantityConversionRaisesAnError) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("unsupported-quantity-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("unsupported-quantity-receive", USDCurrency(),
                                  BarrelUnitOfMeasure(), {periodDate, periodDate + 1}, {1.0, 1.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, MTUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis(), false);

    BOOST_CHECK_THROW(swap->NPV(), Error);
    BOOST_CHECK(!swap->pricingErrors().empty());
    BOOST_CHECK_EQUAL(swap->pricingErrors().back().errorLevel, PricingError::Error);
}

BOOST_AUTO_TEST_CASE(testYieldCurveChangesInvalidateCachedResults) {
    const Date periodDate(6, January, 2025);
    const Date paymentDate(10, January, 2025);
    auto payIndex = makeIndex("curve-update-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("curve-update-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {90.0, 90.0});
    auto pricingPeriods = periods(periodDate, periodDate, paymentDate,
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));

    RelinkableHandle<YieldTermStructure> payCurve, receiveCurve, discountCurve;
    payCurve.linkTo(flatCurvePtr(0.0));
    receiveCurve.linkTo(flatCurvePtr(0.0));
    discountCurve.linkTo(flatCurvePtr(0.0));
    auto swap = makeSwapWithCurves(payIndex, receiveIndex, pricingPeriods, noBasis(), true, nullptr,
                                   payCurve, receiveCurve, discountCurve);

    const Real initialNpv = swap->NPV();
    payCurve.linkTo(flatCurvePtr(0.25));
    const Real afterPayCurveRelink = swap->NPV();
    BOOST_CHECK(std::fabs(afterPayCurveRelink - initialNpv) > 1.0e-8);

    receiveCurve.linkTo(flatCurvePtr(0.35));
    const Real afterReceiveCurveRelink = swap->NPV();
    BOOST_CHECK(std::fabs(afterReceiveCurveRelink - afterPayCurveRelink) > 1.0e-8);

    const Real oldDiscountFactor = swap->paymentCashFlows().at(paymentDate)->discountFactor();
    discountCurve.linkTo(flatCurvePtr(0.45));
    swap->NPV();
    const Real newDiscountFactor = swap->paymentCashFlows().at(paymentDate)->discountFactor();
    BOOST_CHECK(std::fabs(newDiscountFactor - oldDiscountFactor) > 1.0e-8);
}

BOOST_AUTO_TEST_CASE(testDuplicatePaymentDatesAreRejected) {
    const Date firstDate(6, January, 2025);
    const Date secondDate(7, January, 2025);
    const Date paymentDate(10, January, 2025);
    auto payIndex = makeIndex("duplicate-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {firstDate, secondDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("duplicate-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {firstDate, secondDate + 1}, {90.0, 90.0});
    PricingPeriods pricingPeriods = {
        ext::make_shared<PricingPeriod>(firstDate, firstDate, paymentDate,
                                        Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0)),
        ext::make_shared<PricingPeriod>(secondDate, secondDate, paymentDate,
                                        Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0))};

    BOOST_CHECK_THROW(makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis()), Error);
}

BOOST_AUTO_TEST_CASE(testEachIndexUsesItsOwnLastQuoteDate) {
    const Date firstDate(3, January, 2025);
    const Date secondDate(4, January, 2025);
    const Date paymentDate(10, January, 2025);
    Settings::instance().evaluationDate() = Date(6, January, 2025);
    auto payIndex = makeIndex("fixing-cutoff-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {firstDate, secondDate}, {1.0, 1.0});
    auto receiveIndex = makeIndex("fixing-cutoff-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {firstDate, secondDate}, {10.0, 5.0});
    payIndex->addFixing(firstDate, 10.0);
    payIndex->addFixing(secondDate, 20.0);
    receiveIndex->addFixing(firstDate, 10.0);
    auto pricingPeriods = periods(firstDate, secondDate, paymentDate,
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 2.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_SMALL(swap->NPV() + 15.0, 1.0e-10);
    BOOST_CHECK_EQUAL(swap->pricingErrors().size(), 2U);
    for (const PricingError& error : swap->pricingErrors())
        BOOST_CHECK_EQUAL(error.errorLevel, PricingError::Warning);
}

BOOST_AUTO_TEST_CASE(testBasisCanBeAppliedToReceiveLeg) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("receive-basis-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("receive-basis-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {100.0, 100.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap =
        makeSwap(payIndex, receiveIndex, pricingPeriods,
                 CommodityUnitCost(Money(10.0, USDCurrency()), BarrelUnitOfMeasure()), false);

    BOOST_CHECK_SMALL(swap->NPV() - 10.0, 1.0e-10);
    BOOST_CHECK_EQUAL(swap->dailyPositions().at(periodDate).receiveLegPrice, 110.0);
}

BOOST_AUTO_TEST_CASE(testMultiplePeriodsSecondaryCostsAndDailyPositions) {
    const Date firstDate(6, January, 2025);
    const Date secondDate(7, January, 2025);
    const Date firstPaymentDate(10, January, 2025);
    const Date secondPaymentDate(11, January, 2025);
    auto payIndex = makeIndex("multiple-periods-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {firstDate, secondDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("multiple-periods-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {firstDate, secondDate + 1}, {90.0, 90.0});
    PricingPeriods pricingPeriods = {
        ext::make_shared<PricingPeriod>(firstDate, firstDate, firstPaymentDate,
                                        Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0)),
        ext::make_shared<PricingPeriod>(secondDate, secondDate, secondPaymentDate,
                                        Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0))};
    auto secondaryCosts = ext::make_shared<SecondaryCosts>();
    (*secondaryCosts)["fixed fee"] = Money(2.0, USDCurrency());
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis(), true, secondaryCosts);

    BOOST_CHECK_SMALL(swap->NPV() + 22.0, 1.0e-10);
    BOOST_CHECK_EQUAL(swap->paymentCashFlows().size(), 2U);
    BOOST_CHECK_EQUAL(swap->secondaryCostAmounts().at("fixed fee").value(), 2.0);
    BOOST_CHECK_EQUAL(swap->dailyPositions().size(), 2U);
    BOOST_CHECK(swap->dailyPositions().at(firstDate).unrealized);
    BOOST_CHECK(swap->dailyPositions().at(secondDate).unrealized);
    BOOST_CHECK_SMALL(swap->dailyPositions().at(firstDate).riskDelta + 10.0, 1.0e-10);
}

BOOST_AUTO_TEST_CASE(testZeroQuotesAreReportedAsWarnings) {
    const Date periodDate(6, January, 2025);
    auto payIndex = makeIndex("zero-quote-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {0.0, 0.0});
    auto receiveIndex = makeIndex("zero-quote-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {0.0, 0.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_SMALL(swap->NPV(), 1.0e-10);
    BOOST_CHECK_GE(swap->pricingErrors().size(), 2U);
    BOOST_CHECK_EQUAL(swap->pricingErrors().front().errorLevel, PricingError::Warning);
}

BOOST_AUTO_TEST_CASE(testMissingCommodityQuotesRaiseAnError) {
    const Date periodDate(6, January, 2025);
    auto emptyCurve =
        ext::make_shared<CommodityCurve>("missing-quotes", commodityType_, USDCurrency(),
                                         BarrelUnitOfMeasure(), calendar_, Actual365Fixed());
    auto payIndex = ext::make_shared<CommodityIndex>(
        "missing-quotes", commodityType_, USDCurrency(), BarrelUnitOfMeasure(), calendar_, 1.0,
        emptyCurve, ext::shared_ptr<ExchangeContracts>(), 0);
    auto receiveIndex = makeIndex("available-quotes", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {90.0, 90.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_THROW(swap->NPV(), Error);
    BOOST_CHECK(!swap->pricingErrors().empty());
    BOOST_CHECK_EQUAL(swap->pricingErrors().back().errorLevel, PricingError::Error);
}

BOOST_AUTO_TEST_CASE(testEmptyIndexWithoutForwardCurveRaisesAnError) {
    const Date periodDate(6, January, 2025);
    auto payIndex = ext::make_shared<CommodityIndex>(
        "empty-pay-no-forward", commodityType_, USDCurrency(), BarrelUnitOfMeasure(),
        calendar_, 1.0, ext::shared_ptr<CommodityCurve>(),
        ext::shared_ptr<ExchangeContracts>(), 0);
    auto receiveIndex = makeIndex("empty-receive-with-forward", USDCurrency(),
                                  BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {90.0, 90.0});
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_THROW(swap->NPV(), Error);
    BOOST_CHECK(!swap->pricingErrors().empty());
    BOOST_CHECK_EQUAL(swap->pricingErrors().back().errorLevel, PricingError::Error);
}

BOOST_AUTO_TEST_CASE(testForwardOnlyIndexesUseTheirCurves) {
    const Date periodDate(6, January, 2025);
    auto payCurve = ext::make_shared<CommodityCurve>(
        "forward-only-pay", commodityType_, USDCurrency(), BarrelUnitOfMeasure(),
        calendar_, std::vector<Date>{periodDate, periodDate + 1},
        std::vector<Real>{100.0, 100.0});
    auto receiveCurve = ext::make_shared<CommodityCurve>(
        "forward-only-receive", commodityType_, USDCurrency(), BarrelUnitOfMeasure(),
        calendar_, std::vector<Date>{periodDate, periodDate + 1},
        std::vector<Real>{90.0, 90.0});
    auto payIndex = ext::make_shared<CommodityIndex>(
        "forward-only-pay", commodityType_, USDCurrency(), BarrelUnitOfMeasure(),
        calendar_, 1.0, payCurve, ext::shared_ptr<ExchangeContracts>(), 0);
    auto receiveIndex = ext::make_shared<CommodityIndex>(
        "forward-only-receive", commodityType_, USDCurrency(), BarrelUnitOfMeasure(),
        calendar_, 1.0, receiveCurve, ext::shared_ptr<ExchangeContracts>(), 0);
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_SMALL(swap->NPV() + 10.0, 1.0e-10);
    BOOST_CHECK_EQUAL(swap->pricingErrors().size(), 2U);
    for (const PricingError& error : swap->pricingErrors())
        BOOST_CHECK_EQUAL(error.errorLevel, PricingError::Warning);
}

BOOST_AUTO_TEST_CASE(testStaleFixingsWithoutForwardCurveRaiseAnError) {
    const Date periodDate(6, January, 2025);
    auto payIndex = ext::make_shared<CommodityIndex>(
        "stale-pay-no-forward", commodityType_, USDCurrency(), BarrelUnitOfMeasure(),
        calendar_, 1.0, ext::shared_ptr<CommodityCurve>(),
        ext::shared_ptr<ExchangeContracts>(), 0);
    auto receiveIndex = makeIndex("stale-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {90.0, 90.0});
    payIndex->addFixing(evaluationDate_, 100.0);
    auto pricingPeriods = periods(periodDate, periodDate, Date(10, January, 2025),
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_THROW(swap->NPV(), Error);
    BOOST_CHECK(!swap->pricingErrors().empty());
    BOOST_CHECK_EQUAL(swap->pricingErrors().back().errorLevel, PricingError::Error);
}

BOOST_AUTO_TEST_CASE(testNearPaymentDatesSkipDiscounting) {
    const Date periodDate(3, January, 2025);
    const Date paymentDate(3, January, 2025);
    auto payIndex = makeIndex("near-payment-pay", USDCurrency(), BarrelUnitOfMeasure(),
                              {periodDate, periodDate + 1}, {100.0, 100.0});
    auto receiveIndex = makeIndex("near-payment-receive", USDCurrency(), BarrelUnitOfMeasure(),
                                  {periodDate, periodDate + 1}, {90.0, 90.0});
    auto pricingPeriods = periods(periodDate, periodDate, paymentDate,
                                  Quantity(commodityType_, BarrelUnitOfMeasure(), 1.0));
    auto swap = makeSwap(payIndex, receiveIndex, pricingPeriods, noBasis());

    BOOST_CHECK_SMALL(swap->NPV() + 10.0, 1.0e-10);
    BOOST_CHECK_EQUAL(swap->paymentCashFlows().at(paymentDate)->discountFactor(), 1.0);
    BOOST_CHECK(!swap->paymentCashFlows().at(paymentDate)->finalized());
}

BOOST_AUTO_TEST_SUITE_END()
BOOST_AUTO_TEST_SUITE_END()