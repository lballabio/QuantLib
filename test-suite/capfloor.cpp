/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2003 RiskMap srl
 Copyright (C) 2004, 2005, 2006, 2007, 2008 StatPro Italia srl
 Copyright (C) 2019 Wojciech Ślusarski

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <http://quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#include "capfloor.hpp"
#include "utilities.hpp"
#include <ql/instruments/capfloor.hpp>
#include <ql/instruments/vanillaswap.hpp>
#include <ql/cashflows/cashflowvectors.hpp>
#include <ql/termstructures/yield/flatforward.hpp>
#include <ql/termstructures/yield/zerospreadedtermstructure.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/pricingengines/capfloor/blackcapfloorengine.hpp>
#include <ql/pricingengines/capfloor/bacheliercapfloorengine.hpp>
#include <ql/pricingengines/swap/discountingswapengine.hpp>
#include <ql/models/marketmodels/models/flatvol.hpp>
#include <ql/models/marketmodels/correlations/expcorrelations.hpp>
#include <ql/math/matrix.hpp>
#include <ql/time/daycounters/actualactual.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <ql/time/schedule.hpp>
#include <ql/utilities/dataformatters.hpp>
#include <ql/cashflows/cashflows.hpp>
#include <ql/cashflows/couponpricer.hpp>
#include <ql/quotes/simplequote.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

namespace capfloor_test {

    struct CommonVars {
        // common data
        Date settlement;
        std::vector<Real> nominals;
        BusinessDayConvention convention;
        Frequency frequency;
        ext::shared_ptr<IborIndex> index;
        Calendar calendar;
        Natural fixingDays;
        RelinkableHandle<YieldTermStructure> termStructure;

        // cleanup

        SavedSettings backup;

        // setup
        CommonVars()
        : nominals(1,100) {
            frequency = Semiannual;
            index = ext::shared_ptr<IborIndex>(new Euribor6M(termStructure));
            calendar = index->fixingCalendar();
            convention = ModifiedFollowing;
            Date today = calendar.adjust(Date::todaysDate());
            Settings::instance().evaluationDate() = today;
            Natural settlementDays = 2;
            fixingDays = 2;
            settlement = calendar.advance(today,settlementDays,Days);
            termStructure.linkTo(flatRate(settlement,0.05,
                                          ActualActual(ActualActual::ISDA)));
        }

        // utilities
        Leg makeLeg(const Date& startDate, Integer length) const {
            Date endDate = calendar.advance(startDate,length*Years,convention);
            Schedule schedule(startDate, endDate, Period(frequency), calendar,
                              convention, convention,
                              DateGeneration::Forward, false);
            return IborLeg(schedule, index)
                .withNotionals(nominals)
                .withPaymentDayCounter(index->dayCounter())
                .withPaymentAdjustment(convention)
                .withFixingDays(fixingDays);
        }

        ext::shared_ptr<PricingEngine> makeEngine(Volatility volatility) const {
            Handle<Quote> vol(ext::shared_ptr<Quote>(
                                                new SimpleQuote(volatility)));
            return ext::shared_ptr<PricingEngine>(
                                new BlackCapFloorEngine(termStructure, vol));
        }

        ext::shared_ptr<PricingEngine> makeBachelierEngine(Volatility volatility) const {
            Handle<Quote> vol(ext::shared_ptr<Quote>(
                                                new SimpleQuote(volatility)));
            return ext::shared_ptr<PricingEngine>(
                                new BachelierCapFloorEngine(termStructure, vol));
        }

        ext::shared_ptr<CapFloor> makeCapFloor(CapFloor::Type type,
                                               const Leg& leg,
                                               Rate strike,
                                               Volatility volatility,
                                               bool isLogNormal = true) const {
            ext::shared_ptr<CapFloor> result;
            switch (type) {
              case CapFloor::Cap:
                result = ext::shared_ptr<CapFloor>(
                                  new Cap(leg, std::vector<Rate>(1, strike)));
                break;
              case CapFloor::Floor:
                result = ext::shared_ptr<CapFloor>(
                                new Floor(leg, std::vector<Rate>(1, strike)));
                break;
              default:
                QL_FAIL("unknown cap/floor type");
            }
            if(isLogNormal){
                result->setPricingEngine(makeEngine(volatility));
            } else {
                result->setPricingEngine(makeBachelierEngine(volatility));
            }
            return result;
        }
    };

    bool checkAbsError(Real x1, Real x2, Real tolerance){
        return std::fabs(x1 - x2) < tolerance;
    }

    std::string typeToString(CapFloor::Type type) {
        switch (type) {
          case CapFloor::Cap:
            return "cap";
          case CapFloor::Floor:
            return "floor";
          case CapFloor::Collar:
            return "collar";
          default:
            QL_FAIL("unknown cap/floor type");
        }
    }

}


void CapFloorTest::testVega() {

    BOOST_TEST_MESSAGE("Testing cap/floor vega...");

    using namespace capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 4, 5, 6, 7, 10, 15, 20, 30 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };
    Rate strikes[] = { 0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09 };
    CapFloor::Type types[] = { CapFloor::Cap, CapFloor::Floor};

    Date startDate = vars.termStructure->referenceDate();
    static const Real shift = 1e-8;
    static const Real tolerance = 0.005;

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(vols); j++) {
            for (Size k=0; k<LENGTH(strikes); k++) {
                for (Size h=0; h<LENGTH(types); h++) {
                    Leg leg = vars.makeLeg(startDate, lengths[i]);
                    ext::shared_ptr<CapFloor> capFloor =
                        vars.makeCapFloor(types[h],leg,
                                          strikes[k],vols[j]);
                    ext::shared_ptr<CapFloor> shiftedCapFloor2 =
                        vars.makeCapFloor(types[h],leg,
                                          strikes[k],vols[j]+shift);
                     ext::shared_ptr<CapFloor> shiftedCapFloor1 =
                        vars.makeCapFloor(types[h],leg,
                                          strikes[k],vols[j]-shift);
                    Real value1 = shiftedCapFloor1->NPV();
                    Real value2 = shiftedCapFloor2->NPV();
                    Real numericalVega = (value2 - value1) / (2*shift);
                    if (numericalVega>1.0e-4) {
                        Real analyticalVega = capFloor->result<Real>("vega");
                        Real discrepancy =
                            std::fabs(numericalVega - analyticalVega);
                        discrepancy /= numericalVega;
                        if (discrepancy > tolerance)
                            BOOST_FAIL(
                                "failed to compute cap/floor vega:" <<
                                "\n   lengths:     " << lengths[j]*Years <<
                                "\n   strike:      " << io::rate(strikes[k]) <<
                                //"\n   types:       " << types[h] <<
                                std::fixed << std::setprecision(12) <<
                                "\n   calculated:  " << analyticalVega <<
                                "\n   expected:    " << numericalVega <<
                                "\n   discrepancy: " << io::rate(discrepancy) <<
                                "\n   tolerance:   " << io::rate(tolerance));
                     }
                }
            }
        }
    }
}

void CapFloorTest::testStrikeDependency() {

    BOOST_TEST_MESSAGE("Testing cap/floor dependency on strike...");

    using namespace capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };
    Rate strikes[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };

    Date startDate = vars.termStructure->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
        for (Size j=0; j<LENGTH(vols); j++) {
            // store the results for different strikes...
            std::vector<Real> cap_values, floor_values;
            for (Size k=0; k<LENGTH(strikes); k++) {
                Leg leg = vars.makeLeg(startDate,lengths[i]);
                ext::shared_ptr<Instrument> cap =
                    vars.makeCapFloor(CapFloor::Cap,leg,
                                      strikes[k],vols[j]);
                cap_values.push_back(cap->NPV());
                ext::shared_ptr<Instrument> floor =
                    vars.makeCapFloor(CapFloor::Floor,leg,
                                      strikes[k],vols[j]);
                floor_values.push_back(floor->NPV());
            }
            // and check that they go the right way
            std::vector<Real>::iterator it =
                std::adjacent_find(cap_values.begin(),cap_values.end(),
                                   std::less<Real>());
            if (it != cap_values.end()) {
                Size n = it - cap_values.begin();
                BOOST_FAIL(
                    "NPV is increasing with the strike in a cap: \n"
                    << std::setprecision(2)
                    << "    length:     " << lengths[i] << " years\n"
                    << "    volatility: " << io::volatility(vols[j]) << "\n"
                    << "    value:      " << cap_values[n]
                    << " at strike: " << io::rate(strikes[n]) << "\n"
                    << "    value:      " << cap_values[n+1]
                    << " at strike: " << io::rate(strikes[n+1]));
            }
            // same for floors
            it = std::adjacent_find(floor_values.begin(),floor_values.end(),
                                    std::greater<Real>());
            if (it != floor_values.end()) {
                Size n = it - floor_values.begin();
                BOOST_FAIL(
                    "NPV is decreasing with the strike in a floor: \n"
                    << std::setprecision(2)
                    << "    length:     " << lengths[i] << " years\n"
                    << "    volatility: " << io::volatility(vols[j]) << "\n"
                    << "    value:      " << floor_values[n]
                    << " at strike: " << io::rate(strikes[n]) << "\n"
                    << "    value:      " << floor_values[n+1]
                    << " at strike: " << io::rate(strikes[n+1]));
            }
        }
    }
}

void CapFloorTest::testConsistency() {

    BOOST_TEST_MESSAGE("Testing consistency between cap, floor and collar...");

    using namespace capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate cap_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Rate floor_rates[] = { 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = vars.termStructure->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(cap_rates); j++) {
        for (Size k=0; k<LENGTH(floor_rates); k++) {
          for (Size l=0; l<LENGTH(vols); l++) {

              Leg leg = vars.makeLeg(startDate,lengths[i]);
              ext::shared_ptr<CapFloor> cap =
                  vars.makeCapFloor(CapFloor::Cap,leg,
                                    cap_rates[j],vols[l]);
              ext::shared_ptr<CapFloor> floor =
                  vars.makeCapFloor(CapFloor::Floor,leg,
                                    floor_rates[k],vols[l]);
              Collar collar(leg,std::vector<Rate>(1,cap_rates[j]),
                            std::vector<Rate>(1,floor_rates[k]));
              collar.setPricingEngine(vars.makeEngine(vols[l]));

              if (std::fabs((cap->NPV()-floor->NPV())-collar.NPV()) > 1e-10) {
                  BOOST_FAIL(
                    "inconsistency between cap, floor and collar:\n"
                    << "    length:       " << lengths[i] << " years\n"
                    << "    volatility:   " << io::volatility(vols[l]) << "\n"
                    << "    cap value:    " << cap->NPV()
                    << " at strike: " << io::rate(cap_rates[j]) << "\n"
                    << "    floor value:  " << floor->NPV()
                    << " at strike: " << io::rate(floor_rates[k]) << "\n"
                    << "    collar value: " << collar.NPV());


              // test re-composition by optionlets, N.B. two per year
              Real capletsNPV = 0.0;
              std::vector<ext::shared_ptr<CapFloor> > caplets;
              for (Integer m=0; m<lengths[i]*2; m++) {
                caplets.push_back(cap->optionlet(m));
                caplets[m]->setPricingEngine(vars.makeEngine(vols[l]));
                capletsNPV += caplets[m]->NPV();
              }

              if (std::fabs(cap->NPV() - capletsNPV) > 1e-10) {
                BOOST_FAIL(
                  "sum of caplet NPVs does not equal cap NPV:\n"
                    << "    length:       " << lengths[i] << " years\n"
                    << "    volatility:   " << io::volatility(vols[l]) << "\n"
                    << "    cap value:    " << cap->NPV()
                    << " at strike: " << io::rate(cap_rates[j]) << "\n"
                    << "    sum of caplets value:  " << capletsNPV
                    << " at strike (first): " << io::rate(caplets[0]->capRates()[0]) << "\n"
                );
              }

              Real floorletsNPV = 0.0;
              std::vector<ext::shared_ptr<CapFloor> > floorlets;
              for (Integer m=0; m<lengths[i]*2; m++) {
                floorlets.push_back(floor->optionlet(m));
                floorlets[m]->setPricingEngine(vars.makeEngine(vols[l]));
                floorletsNPV += floorlets[m]->NPV();
              }

              if (std::fabs(floor->NPV() - floorletsNPV) > 1e-10) {
                BOOST_FAIL(
                  "sum of floorlet NPVs does not equal floor NPV:\n"
                    << "    length:       " << lengths[i] << " years\n"
                    << "    volatility:   " << io::volatility(vols[l]) << "\n"
                    << "    cap value:    " << floor->NPV()
                    << " at strike: " << io::rate(floor_rates[j]) << "\n"
                    << "    sum of floorlets value:  " << floorletsNPV
                    << " at strike (first): " << io::rate(floorlets[0]->floorRates()[0]) << "\n"
                );
              }

              Real collarletsNPV = 0.0;
              std::vector<ext::shared_ptr<CapFloor> > collarlets;
              for (Integer m=0; m<lengths[i]*2; m++) {
                collarlets.push_back(collar.optionlet(m));
                collarlets[m]->setPricingEngine(vars.makeEngine(vols[l]));
                collarletsNPV += collarlets[m]->NPV();
              }

              if (std::fabs(collar.NPV() - collarletsNPV) > 1e-10) {
                BOOST_FAIL(
                  "sum of collarlet NPVs does not equal floor NPV:\n"
                    << "    length:       " << lengths[i] << " years\n"
                    << "    volatility:   " << io::volatility(vols[l]) << "\n"
                    << "    cap value:    " << collar.NPV()
                    << " at strike floor: " << io::rate(floor_rates[j])
                    << " at strike cap: " << io::rate(cap_rates[j]) << "\n"
                    << "    sum of collarlets value:  " << collarletsNPV
                    << " at strike floor (first): " << io::rate(collarlets[0]->floorRates()[0])
                    << " at strike cap (first): " << io::rate(collarlets[0]->capRates()[0]) << "\n"
                );
              }




              }
          }
        }
      }
    }
}

void CapFloorTest::testParity() {

    BOOST_TEST_MESSAGE("Testing cap/floor parity...");

    using namespace capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate strikes[] = { 0., 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = vars.termStructure->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
      for (Size j=0; j<LENGTH(strikes); j++) {
        for (Size k=0; k<LENGTH(vols); k++) {

            Leg leg = vars.makeLeg(startDate,lengths[i]);
            ext::shared_ptr<Instrument> cap =
                vars.makeCapFloor(CapFloor::Cap,leg,
                                  strikes[j],vols[k]);
            ext::shared_ptr<Instrument> floor =
                vars.makeCapFloor(CapFloor::Floor,leg,
                                  strikes[j],vols[k]);
            Date maturity = vars.calendar.advance(startDate,lengths[i],Years,
                                              vars.convention);
            Schedule schedule(startDate,maturity,
                              Period(vars.frequency),vars.calendar,
                              vars.convention,vars.convention,
                              DateGeneration::Forward,false);
            VanillaSwap swap(VanillaSwap::Payer, vars.nominals[0],
                             schedule, strikes[j], vars.index->dayCounter(),
                             schedule, vars.index, 0.0,
                             vars.index->dayCounter());
            swap.setPricingEngine(ext::shared_ptr<PricingEngine>(
                              new DiscountingSwapEngine(vars.termStructure)));
            // FLOATING_POINT_EXCEPTION
            if (std::fabs((cap->NPV()-floor->NPV()) - swap.NPV()) > 1.0e-10) {
                BOOST_FAIL(
                    "put/call parity violated:\n"
                    << "    length:      " << lengths[i] << " years\n"
                    << "    volatility:  " << io::volatility(vols[k]) << "\n"
                    << "    strike:      " << io::rate(strikes[j]) << "\n"
                    << "    cap value:   " << cap->NPV() << "\n"
                    << "    floor value: " << floor->NPV() << "\n"
                    << "    swap value:  " << swap.NPV());
            }
        }
      }
    }
}

void CapFloorTest::testATMRate() {

    BOOST_TEST_MESSAGE("Testing cap/floor ATM rate...");

    using namespace capfloor_test;

    CommonVars vars;

    Integer lengths[] = { 1, 2, 3, 5, 7, 10, 15, 20 };
    Rate strikes[] = { 0., 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.15, 0.20 };

    Date startDate = vars.termStructure->referenceDate();

    for (Size i=0; i<LENGTH(lengths); i++) {
        Leg leg = vars.makeLeg(startDate,lengths[i]);
        Date maturity = vars.calendar.advance(startDate,lengths[i],Years,
                                  vars.convention);
        Schedule schedule(startDate,maturity,
                          Period(vars.frequency),vars.calendar,
                          vars.convention,vars.convention,
                          DateGeneration::Forward,false);

        for (Size j=0; j<LENGTH(strikes); j++) {
            for (Size k=0; k<LENGTH(vols); k++) {
                ext::shared_ptr<CapFloor> cap =
                    vars.makeCapFloor(CapFloor::Cap, leg, strikes[j],vols[k]);
                ext::shared_ptr<CapFloor> floor =
                    vars.makeCapFloor(CapFloor::Floor, leg, strikes[j],vols[k]);
                Rate capATMRate = cap->atmRate(**vars.termStructure);
                Rate floorATMRate = floor->atmRate(**vars.termStructure);
                if (!checkAbsError(floorATMRate, capATMRate, 1.0e-10))
                    BOOST_FAIL(
                      "Cap ATM Rate and floor ATM Rate should be equal :\n"
                      << "   length:        " << lengths[i] << " years\n"
                      << "   volatility:    " << io::volatility(vols[k]) << "\n"
                      << "   strike:        " << io::rate(strikes[j]) << "\n"
                      << "   cap ATM rate:  " << capATMRate << "\n"
                      << "   floor ATM rate:" << floorATMRate << "\n"
                      << "   relative Error:"
                      << relativeError(capATMRate, floorATMRate,
                                       capATMRate)*100 << "%" );
                VanillaSwap swap(VanillaSwap::Payer, vars.nominals[0],
                                 schedule, floorATMRate,
                                 vars.index->dayCounter(),
                                 schedule, vars.index, 0.0,
                                 vars.index->dayCounter());
                swap.setPricingEngine(ext::shared_ptr<PricingEngine>(
                              new DiscountingSwapEngine(vars.termStructure)));
                Real swapNPV = swap.NPV();
                if (!checkAbsError(swapNPV, 0, 1.0e-10))
                    BOOST_FAIL(
                      "the NPV of a Swap struck at ATM rate "
                      "should be equal to 0:\n"
                      << "   length:        " << lengths[i] << " years\n"
                      << "   volatility:    " << io::volatility(vols[k]) << "\n"
                      << "   ATM rate:      " << io::rate(floorATMRate) << "\n"
                      << "   swap NPV:      " << swapNPV);
        }
      }
    }
}


void CapFloorTest::testImpliedVolatility() {

    BOOST_TEST_MESSAGE("Testing implied term volatility for cap and floor...");

    using namespace capfloor_test;

    CommonVars vars;

    Size maxEvaluations = 100;
    Real tolerance = 1.0e-8;

    CapFloor::Type types[] = { CapFloor::Cap, CapFloor::Floor };
    Rate strikes[] = { 0.02, 0.03, 0.04 };
    Integer lengths[] = { 1, 5, 10 };

    // test data
    Rate rRates[] = { 0.02, 0.03, 0.04, 0.05, 0.06, 0.07 };
    Volatility vols[] = { 0.01, 0.05, 0.10, 0.20, 0.30, 0.70, 0.90 };

    for (Size k=0; k<LENGTH(lengths); k++) {
        Leg leg = vars.makeLeg(vars.settlement, lengths[k]);

        for (Size i=0; i<LENGTH(types); i++) {
            for (Size j=0; j<LENGTH(strikes); j++) {

                ext::shared_ptr<CapFloor> capfloor =
                    vars.makeCapFloor(types[i], leg, strikes[j], 0.0);

                for (Size n=0; n<LENGTH(rRates); n++) {
                    for (Size m=0; m<LENGTH(vols); m++) {

                        Rate r = rRates[n];
                        Volatility v = vols[m];
                        vars.termStructure.linkTo(flatRate(vars.settlement,r,
                                                       Actual360()));
                        capfloor->setPricingEngine(vars.makeEngine(v));

                        Real value = capfloor->NPV();
                        Volatility implVol = 0.0;
                        try {
                            implVol =
                                capfloor->impliedVolatility(value,
                                                            vars.termStructure,
                                                            0.10,
                                                            tolerance,
                                                            maxEvaluations,
                                                            10.0e-7, 4.0,
                                                            ShiftedLognormal, 0.0);
                        } catch (std::exception& e) {
                            // couldn't bracket?
                            capfloor->setPricingEngine(vars.makeEngine(0.0));
                            Real value2 = capfloor->NPV();
                            if (std::fabs(value-value2) < tolerance) {
                                // ok, just skip:
                                continue;
                            }
                            // otherwise, report error
                            BOOST_ERROR("implied vol failure: " <<
                                        typeToString(types[i]) <<
                                        "\n  strike:     " << io::rate(strikes[j]) <<
                                        "\n  risk-free:  " << io::rate(r) <<
                                        "\n  length:     " << lengths[k] << "Y" <<
                                        "\n  volatility: " << io::volatility(v) <<
                                        "\n  price:      " << value <<
                                        "\n" << e.what());
                        }
                        if (std::fabs(implVol-v) > tolerance) {
                            // the difference might not matter
                            capfloor->setPricingEngine(
                                                    vars.makeEngine(implVol));
                            Real value2 = capfloor->NPV();
                            if (std::fabs(value-value2) > tolerance) {
                            BOOST_FAIL("implied vol failure: " <<
                                       typeToString(types[i]) <<
                                       "\n  strike:        " << io::rate(strikes[j]) <<
                                       "\n  risk-free:     " << io::rate(r) <<
                                       "\n  length:        " << lengths[k] << "Y" <<
                                       "\n  volatility:    " << io::volatility(v) <<
                                       "\n  price:         " << value <<
                                       "\n  implied vol:   " << io::volatility(implVol) <<
                                       "\n  implied price: " << value2);
                            }
                        }
                    }
                }
            }
        }
    }
}

void CapFloorTest::testCachedValue() {

    BOOST_TEST_MESSAGE("Testing Black cap/floor price against cached values...");

    using namespace capfloor_test;

    CommonVars vars;

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().evaluationDate() = cachedToday;
    vars.termStructure.linkTo(flatRate(cachedSettlement, 0.05, Actual360()));
    Date startDate = vars.termStructure->referenceDate();
    Leg leg = vars.makeLeg(startDate,20);
    ext::shared_ptr<Instrument> cap = vars.makeCapFloor(CapFloor::Cap,leg,
                                                          0.07,0.20);
    ext::shared_ptr<Instrument> floor = vars.makeCapFloor(CapFloor::Floor,leg,
                                                            0.03,0.20);

    Real cachedCapNPV, cachedFloorNPV ;
    if (!IborCoupon::usingAtParCoupons()) {
        // index fixing price
        cachedCapNPV   = 6.87630307745,
        cachedFloorNPV = 2.65796764715;
    } else {
        // par coupon price
        cachedCapNPV   = 6.87570026732;
        cachedFloorNPV = 2.65812927959;
    }

    // test Black cap price against cached value
    if (std::fabs(cap->NPV()-cachedCapNPV) > 1.0e-11)
        BOOST_ERROR(
            "failed to reproduce cached cap value:\n"
            << std::setprecision(12)
            << "    calculated: " << cap->NPV() << "\n"
            << "    expected:   " << cachedCapNPV);
    // test Black floor price against cached value
    if (std::fabs(floor->NPV()-cachedFloorNPV) > 1.0e-11)
        BOOST_ERROR(
            "failed to reproduce cached floor value:\n"
            << std::setprecision(12)
            << "    calculated: " << floor->NPV() << "\n"
            << "    expected:   " << cachedFloorNPV);
}

void CapFloorTest::testCachedValueFromOptionLets() {

    BOOST_TEST_MESSAGE("Testing Black cap/floor price as a sum of optionlets prices against cached values...");

    using namespace capfloor_test;

    CommonVars vars;

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().evaluationDate() = cachedToday;
    ext::shared_ptr<YieldTermStructure> baseCurve = flatRate(cachedSettlement, 
                                                             0.05, Actual360());                                              
    vars.termStructure.linkTo(baseCurve);
    Date startDate = vars.termStructure->referenceDate();
    Leg leg = vars.makeLeg(startDate,20);  

    ext::shared_ptr<Instrument> cap = vars.makeCapFloor(CapFloor::Cap,leg,
                                                          0.07,0.20);
    ext::shared_ptr<Instrument> floor = vars.makeCapFloor(CapFloor::Floor,leg,
                                                            0.03,0.20);
    Real calculatedCapletsNPV = 0.0,
         calculatedFloorletsNPV = 0.0;

    Real cachedCapNPV, cachedFloorNPV;
    if (IborCoupon::usingAtParCoupons()) {
        cachedCapNPV = 6.87570026732;
        cachedFloorNPV = 2.65812927959;
    } else {
        cachedCapNPV = 6.87630307745;
        cachedFloorNPV = 2.65796764715;
    }

    // test Black floor price against cached value
    std::vector<Real> capletPrices;
    std::vector<Real> floorletPrices;
    
    capletPrices = cap->result<std::vector<Real> >("optionletsPrice");
    floorletPrices = floor->result<std::vector<Real> >("optionletsPrice");
    
    if (capletPrices.size() != 40)
        BOOST_ERROR(
            "failed to produce prices for all caplets:\n"
            << "    calculated: " << capletPrices.size() << " caplet prices\n"
            << "    expected:   " << 40);

    for (Size n=0; n<capletPrices.size(); n++){
        calculatedCapletsNPV += capletPrices[n];
    }

    for (Size n=0; n<floorletPrices.size(); n++){
        calculatedFloorletsNPV += floorletPrices[n];
    }

    if (std::fabs(calculatedCapletsNPV-cachedCapNPV) > 1.0e-11)
        BOOST_ERROR(
            "failed to reproduce cached cap value from its caplets' values:\n"
            << std::setprecision(12)
            << "    calculated: " << calculatedCapletsNPV << "\n"
            << "    expected:   " << cachedCapNPV);
    // test Black floor price against cached value
    if (std::fabs(calculatedFloorletsNPV-cachedFloorNPV) > 1.0e-11)
        BOOST_ERROR(
            "failed to reproduce cached floor value from its floorlets' values:\n"
            << std::setprecision(12)
            << "    calculated: " << calculatedFloorletsNPV << "\n"
            << "    expected:   " << cachedFloorNPV);
}

void CapFloorTest::testOptionLetsDelta() {

    BOOST_TEST_MESSAGE("Testing Black caplet/floorlet delta coefficients against finite difference values...");

    using namespace capfloor_test;

    CommonVars vars;

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().evaluationDate() = cachedToday;
    ext::shared_ptr<YieldTermStructure> baseCurve = flatRate(cachedSettlement, 
                                                             0.05, Actual360());
    RelinkableHandle<YieldTermStructure> baseCurveHandle(baseCurve);

    // Define spreaded curve with eps as spread used for FD sensitivities
    Real eps = 1.0e-6;
    ext::shared_ptr<SimpleQuote> spread(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> spreadCurve(new ZeroSpreadedTermStructure(
                                                            baseCurveHandle,
                                                            Handle<Quote>(spread),
                                                            Continuous,
                                                            Annual,
                                                            Actual360()));                                               
    vars.termStructure.linkTo(spreadCurve);
    Date startDate = vars.termStructure->referenceDate();
    Leg leg = vars.makeLeg(startDate,20);  

    ext::shared_ptr<CapFloor> cap = vars.makeCapFloor(CapFloor::Cap,leg,
                                                          0.05,0.20);
    ext::shared_ptr<CapFloor> floor = vars.makeCapFloor(CapFloor::Floor,leg,
                                                            0.05,0.20);

    
    //so far tests pass, now try to get additional results and it will fail
    Size capletsNum = cap->capRates().size();
    std::vector<Real> capletUpPrices, 
                      capletDownPrices,
                      capletAnalyticDelta,
                      capletDiscountFactorsUp,
                      capletDiscountFactorsDown,
                      capletForwardsUp,
                      capletForwardsDown,
                      capletFDDelta(capletsNum, 0.0); 
    Size floorletNum = floor->floorRates().size();
    std::vector<Real> floorletUpPrices, 
                      floorletDownPrices,
                      floorletAnalyticDelta,
                      floorletDiscountFactorsUp,
                      floorletDiscountFactorsDown,
                      floorletForwardsUp,
                      floorletForwardsDown,
                      floorletFDDelta(floorletNum, 0.0);
    
    capletAnalyticDelta = cap->result<std::vector<Real> >("optionletsDelta");
    floorletAnalyticDelta = floor->result<std::vector<Real> >("optionletsDelta");
    
    spread->setValue(eps);
    capletUpPrices = cap->result<std::vector<Real> >("optionletsPrice");
    floorletUpPrices = floor->result<std::vector<Real> >("optionletsPrice");
    capletDiscountFactorsUp = cap->result<std::vector<Real> >("optionletsDiscountFactor");
    floorletDiscountFactorsUp = floor->result<std::vector<Real> >("optionletsDiscountFactor");
    capletForwardsUp = cap->result<std::vector<Real> >("optionletsAtmForward");
    floorletForwardsUp = floor->result<std::vector<Real> >("optionletsAtmForward");
    
    spread->setValue(-eps);
    capletDownPrices = cap->result<std::vector<Real> >("optionletsPrice");
    floorletDownPrices = floor->result<std::vector<Real> >("optionletsPrice");
    capletDiscountFactorsDown = cap->result<std::vector<Real> >("optionletsDiscountFactor");
    floorletDiscountFactorsDown = floor->result<std::vector<Real> >("optionletsDiscountFactor");
    capletForwardsDown = cap->result<std::vector<Real> >("optionletsAtmForward");
    floorletForwardsDown = floor->result<std::vector<Real> >("optionletsAtmForward");

    Real accrualFactor;
    Leg capLeg = cap->floatingLeg();
    Leg floorLeg = floor->floatingLeg();
    
    for (Size n=1; n < capletUpPrices.size(); n++){
        // calculating only caplet's FD sensitivity w.r.t. forward rate
        // without the effect of sensitivity related to changed discount factor
        ext::shared_ptr<FloatingRateCoupon> c = ext::dynamic_pointer_cast<FloatingRateCoupon>(capLeg[n]);
        accrualFactor = c->nominal() * c->accrualPeriod() * c->gearing();
        capletFDDelta[n] = (capletUpPrices[n] / capletDiscountFactorsUp[n]
                           - capletDownPrices[n] / capletDiscountFactorsDown[n]) 
                           / (capletForwardsUp[n] - capletForwardsDown[n])
                           / accrualFactor;
    }

    for (Size n=0; n<floorletUpPrices.size(); n++){
        // calculating only caplet's FD sensitivity w.r.t. forward rate
        // without the effect of sensitivity related to changed discount factor
        ext::shared_ptr<FloatingRateCoupon> c = ext::dynamic_pointer_cast<FloatingRateCoupon>(floorLeg[n]);
        accrualFactor = c->nominal() * c->accrualPeriod() * c->gearing();
        floorletFDDelta[n] = (floorletUpPrices[n] / floorletDiscountFactorsUp[n] 
                             - floorletDownPrices[n] / floorletDiscountFactorsDown[n]) 
                             / (floorletForwardsUp[n] - floorletForwardsDown[n])
                             / accrualFactor;        
    }

    for (Size n=0; n<capletAnalyticDelta.size(); n++){
        if (std::fabs(capletAnalyticDelta[n]-capletFDDelta[n]) > 1.0e-6)
            BOOST_ERROR(
                "failed to compare analytical and finite difference caplet delta:\n"
                << "caplet number:\t" << n << "\n"
                << std::setprecision(12)
                << "    finite difference: " << capletFDDelta[n]<< "\n"
                << "    analytical value:   " << capletAnalyticDelta[n] << "\n"
                << "    resulting ratio: " << capletFDDelta[n] / capletAnalyticDelta[n]);    
    }

    for (Size n=0; n<floorletAnalyticDelta.size(); n++){
        if (std::fabs(floorletAnalyticDelta[n]-floorletFDDelta[n]) > 1.0e-6)
            BOOST_ERROR(
                "failed to compare analytical and finite difference floorlet delta:\n"
                << "floorlet number:\t" << n << "\n"
                << std::setprecision(12)
                << "    finite difference: " << floorletFDDelta[n]<< "\n"
                << "    analytical value:   " << floorletAnalyticDelta[n] << "\n"
                << "    resulting ratio: " << floorletFDDelta[n] / floorletAnalyticDelta[n]);    
    }

}

void CapFloorTest::testBachelierOptionLetsDelta() {

    BOOST_TEST_MESSAGE("Testing Bachelier caplet/floorlet delta coefficients against finite difference values...");

    using namespace capfloor_test;

    CommonVars vars;

    Date cachedToday(14,March,2002),
         cachedSettlement(18,March,2002);
    Settings::instance().evaluationDate() = cachedToday;
    ext::shared_ptr<YieldTermStructure> baseCurve = flatRate(cachedSettlement, 
                                                             0.05, Actual360());
    RelinkableHandle<YieldTermStructure> baseCurveHandle(baseCurve);

    // Define spreaded curve with eps as spread used for FD sensitivities
    Real eps = 1.0e-6;
    ext::shared_ptr<SimpleQuote> spread(new SimpleQuote(0.0));
    ext::shared_ptr<YieldTermStructure> spreadCurve(new ZeroSpreadedTermStructure(
                                                            baseCurveHandle,
                                                            Handle<Quote>(spread),
                                                            Continuous,
                                                            Annual,
                                                            Actual360()));                                               
    vars.termStructure.linkTo(spreadCurve);
    Date startDate = vars.termStructure->referenceDate();
    Leg leg = vars.makeLeg(startDate,20);  

    // Use normal model (BachelierCapFloorEngine)
    bool isLogNormal = false;

    ext::shared_ptr<CapFloor> cap = vars.makeCapFloor(CapFloor::Cap,leg,
                                                      0.05, 0.01, isLogNormal);
    ext::shared_ptr<CapFloor> floor = vars.makeCapFloor(CapFloor::Floor,leg,
                                                        0.05, 0.01, isLogNormal);

    
    //so far tests pass, now try to get additional results and it will fail
    Size capletsNum = cap->capRates().size();
    std::vector<Real> capletUpPrices, 
                      capletDownPrices,
                      capletAnalyticDelta,
                      capletDiscountFactorsUp,
                      capletDiscountFactorsDown,
                      capletForwardsUp,
                      capletForwardsDown,
                      capletFDDelta(capletsNum, 0.0); 
    Size floorletNum = floor->floorRates().size();
    std::vector<Real> floorletUpPrices, 
                      floorletDownPrices,
                      floorletAnalyticDelta,
                      floorletDiscountFactorsUp,
                      floorletDiscountFactorsDown,
                      floorletForwardsUp,
                      floorletForwardsDown,
                      floorletFDDelta(floorletNum, 0.0);
    
    capletAnalyticDelta = cap->result<std::vector<Real> >("optionletsDelta");
    floorletAnalyticDelta = floor->result<std::vector<Real> >("optionletsDelta");
    
    spread->setValue(eps);
    capletUpPrices = cap->result<std::vector<Real> >("optionletsPrice");
    floorletUpPrices = floor->result<std::vector<Real> >("optionletsPrice");
    capletDiscountFactorsUp = cap->result<std::vector<Real> >("optionletsDiscountFactor");
    floorletDiscountFactorsUp = floor->result<std::vector<Real> >("optionletsDiscountFactor");
    capletForwardsUp = cap->result<std::vector<Real> >("optionletsAtmForward");
    floorletForwardsUp = floor->result<std::vector<Real> >("optionletsAtmForward");
    
    spread->setValue(-eps);
    capletDownPrices = cap->result<std::vector<Real> >("optionletsPrice");
    floorletDownPrices = floor->result<std::vector<Real> >("optionletsPrice");
    capletDiscountFactorsDown = cap->result<std::vector<Real> >("optionletsDiscountFactor");
    floorletDiscountFactorsDown = floor->result<std::vector<Real> >("optionletsDiscountFactor");
    capletForwardsDown = cap->result<std::vector<Real> >("optionletsAtmForward");
    floorletForwardsDown = floor->result<std::vector<Real> >("optionletsAtmForward");

    Real accrualFactor;
    Leg capLeg = cap->floatingLeg();
    Leg floorLeg = floor->floatingLeg();
    
    for (Size n=1; n < capletUpPrices.size(); n++){
        // calculating only caplet's FD sensitivity w.r.t. forward rate
        // without the effect of sensitivity related to changed discount factor
        ext::shared_ptr<FloatingRateCoupon> c = ext::dynamic_pointer_cast<FloatingRateCoupon>(capLeg[n]);
        accrualFactor = c->nominal() * c->accrualPeriod() * c->gearing();
        capletFDDelta[n] = (capletUpPrices[n] / capletDiscountFactorsUp[n]
                           - capletDownPrices[n] / capletDiscountFactorsDown[n]) 
                           / (capletForwardsUp[n] - capletForwardsDown[n])
                           / accrualFactor;
    }

    for (Size n=0; n<floorletUpPrices.size(); n++){
        // calculating only caplet's FD sensitivity w.r.t. forward rate
        // without the effect of sensitivity related to changed discount factor
        ext::shared_ptr<FloatingRateCoupon> c = ext::dynamic_pointer_cast<FloatingRateCoupon>(floorLeg[n]);
        accrualFactor = c->nominal() * c->accrualPeriod() * c->gearing();
        floorletFDDelta[n] = (floorletUpPrices[n] / floorletDiscountFactorsUp[n] 
                             - floorletDownPrices[n] / floorletDiscountFactorsDown[n]) 
                             / (floorletForwardsUp[n] - floorletForwardsDown[n])
                             / accrualFactor;        
    }

    for (Size n=0; n<capletAnalyticDelta.size(); n++){
        if (std::fabs(capletAnalyticDelta[n]-capletFDDelta[n]) > 1.0e-6)
            BOOST_ERROR(
                "failed to compare analytical and finite difference caplet delta:\n"
                << "caplet number:\t" << n << "\n"
                << std::setprecision(12)
                << "    finite difference: " << capletFDDelta[n]<< "\n"
                << "    analytical value:   " << capletAnalyticDelta[n] << "\n"
                << "    resulting ratio: " << capletFDDelta[n] / capletAnalyticDelta[n]);    
    }

    for (Size n=0; n<floorletAnalyticDelta.size(); n++){
        if (std::fabs(floorletAnalyticDelta[n]-floorletFDDelta[n]) > 1.0e-6)
            BOOST_ERROR(
                "failed to compare analytical and finite difference floorlet delta:\n"
                << "floorlet number:\t" << n << "\n"
                << std::setprecision(12)
                << "    finite difference: " << floorletFDDelta[n]<< "\n"
                << "    analytical value:   " << floorletAnalyticDelta[n] << "\n"
                << "    resulting ratio: " << floorletFDDelta[n] / floorletAnalyticDelta[n]);    
    }

}

test_suite* CapFloorTest::suite() {
    test_suite* suite = BOOST_TEST_SUITE("Cap and floor tests");
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testStrikeDependency));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testConsistency));
    // FLOATING_POINT_EXCEPTION
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testParity));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testVega));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testATMRate));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testImpliedVolatility));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testCachedValue));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testCachedValueFromOptionLets));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testOptionLetsDelta));
    suite->add(QUANTLIB_TEST_CASE(&CapFloorTest::testBachelierOptionLetsDelta));
    return suite;
}

