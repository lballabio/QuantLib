
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

#ifndef quantlib_test_swaption_hpp
#define quantlib_test_swaption_hpp

#include <ql/quantlib.hpp>
#include <cppunit/TestCase.h>
#include <cppunit/TestFixture.h>

class SwaptionTest : public CppUnit::TestFixture {
  public:
    SwaptionTest();
    void setUp();
    void testStrikeDependency();
    void testSpreadDependency();
    void testSpreadTreatment();
    void testCachedValue();
    static CppUnit::Test* suite();
  private:
    QL::Handle<QLINS::SimpleSwap> makeSwap(const QL::Date& start, 
                                           int length, QL::Rate fixedRate,
                                           QL::Spread floatingSpread,
                                           bool payFixed);
    QL::Handle<QLINS::Swaption> makeSwaption(
                                       const QL::Handle<QLINS::SimpleSwap>&,
                                       const QL::Date& exercise,
                                       double volatility);
    QL::Date today_, settlement_;
    double nominal_;
    QL::Calendar calendar_;
    QL::RollingConvention rollingConvention_;
    int fixedFrequency_, floatingFrequency_;
    QL::DayCounter fixedDayCount_;
    bool fixedIsAdjusted_;
    QL::Handle<QLIDX::Xibor> index_;
    int settlementDays_, fixingDays_;
    QL::RelinkableHandle<QL::TermStructure> termStructure_;
};


#endif
