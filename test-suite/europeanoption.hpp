
/*
 Copyright (C) 2003 RiskMap srl

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it under the
 terms of the QuantLib license.  You should have received a copy of the
 license along with this program; if not, please email ferdinando@ametrano.net
 The license is also available online at http://quantlib.org/html/license.html

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/
// $Id$

#ifndef quantlib_test_european_option_hpp
#define quantlib_test_european_option_hpp

#include <ql/quantlib.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>

class EuropeanOptionTest : public CppUnit::TestFixture {
  public:
    void testGreeks();
    void testImpliedVol();
    void testBinomialEngines();
    static CppUnit::Test* suite();
  private:
    enum EngineType { Analytic, JR, CRR, EQP, Trigeorgis, Tian};
    double relativeError(double x1, double x2, double reference);
    QL::Handle<QL::Instrument> makeEuropeanOption(
        QL::Option::Type type,
        const QL::Handle<QL::MarketElement>& underlying,
        double strike,
        const QL::Handle<QL::TermStructure>& divCurve,
        const QL::Handle<QL::TermStructure>& rfCurve,
        const QL::Date& exDate,
        const QL::Handle<QL::BlackVolTermStructure>& volatility,
        EngineType engineType = Analytic);
    QL::Handle<QL::TermStructure> makeFlatCurve(
        const QL::Handle<QL::MarketElement>& forward);
    QL::Handle<QL::BlackVolTermStructure> makeFlatVolatility(
        const QL::Handle<QL::MarketElement>& volatility);
    std::string typeToString(QL::Option::Type);
    std::string engineTypeToString(EngineType);
};


#endif
