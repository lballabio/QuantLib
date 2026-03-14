/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2007 Cristina Duminuco
 Copyright (C) 2006 Ferdinando Ametrano
 Copyright (C) 2006 François du Vignaud
 Copyright (C) 2006 Giorgio Facchinetti

 This file is part of QuantLib, a free-software/open-source library
 for financial quantitative analysts and developers - http://quantlib.org/

 QuantLib is free software: you can redistribute it and/or modify it
 under the terms of the QuantLib license.  You should have received a
 copy of the license along with this program; if not, please email
 <quantlib-dev@lists.sf.net>. The license is also available online at
 <https://www.quantlib.org/license.shtml>.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE.  See the license for more details.
*/

#ifndef quantlib_test_swaption_volatility_structures_utilities_hpp
#define quantlib_test_swaption_volatility_structures_utilities_hpp

#include <ql/time/period.hpp>
#include <ql/math/matrix.hpp>
#include <ql/quotes/simplequote.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual365fixed.hpp>
#include <ql/termstructures/volatility/swaption/swaptionvolmatrix.hpp>
#include <ql/termstructures/yieldtermstructure.hpp>
#include <iostream>
#include <vector>

namespace QuantLib {

    struct SwaptionTenors {
        std::vector<Period> options;
        std::vector<Period> swaps;
    };
    struct SwaptionMarketConventions {
        Calendar calendar;
        BusinessDayConvention optionBdc;
        DayCounter dayCounter;
        void setConventions() {
            calendar = TARGET();
            optionBdc = ModifiedFollowing;
            dayCounter = Actual365Fixed();
        }
    };
    struct AtmVolatility {
        SwaptionTenors tenors;
        Matrix vols;
        std::vector<std::vector<Handle<Quote> > > volsHandle;
        void setMarketData() {
            tenors.options.resize(6);
            tenors.options[0] = Period(1, Months);
            tenors.options[1] = Period(6, Months);
            tenors.options[2] = Period(1, Years);
            tenors.options[3] = Period(5, Years);
            tenors.options[4] = Period(10, Years);
            tenors.options[5] = Period(30, Years);
            tenors.swaps.resize(4);
            tenors.swaps[0] = Period(1, Years);
            tenors.swaps[1] = Period(5, Years);
            tenors.swaps[2] = Period(10, Years);
            tenors.swaps[3] = Period(30, Years);
            vols = Matrix(tenors.options.size(), tenors.swaps.size());
            vols[0][0]=0.1300; vols[0][1]=0.1560; vols[0][2]=0.1390; vols[0][3]=0.1220;
            vols[1][0]=0.1440; vols[1][1]=0.1580; vols[1][2]=0.1460; vols[1][3]=0.1260;
            vols[2][0]=0.1600; vols[2][1]=0.1590; vols[2][2]=0.1470; vols[2][3]=0.1290;
            vols[3][0]=0.1640; vols[3][1]=0.1470; vols[3][2]=0.1370; vols[3][3]=0.1220;
            vols[4][0]=0.1400; vols[4][1]=0.1300; vols[4][2]=0.1250; vols[4][3]=0.1100;
            vols[5][0]=0.1130; vols[5][1]=0.1090; vols[5][2]=0.1070; vols[5][3]=0.0930;
            volsHandle.resize(tenors.options.size());
            for (Size i=0; i<tenors.options.size(); i++){
                volsHandle[i].resize(tenors.swaps.size());
                for (Size j=0; j<tenors.swaps.size(); j++)
                    // every handle must be reassigned, as the ones created by
                    // default are all linked together.
                    volsHandle[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(new
                        SimpleQuote(vols[i][j])));
            }
        };
    };
    struct VolatilityCube {
        SwaptionTenors tenors;
        Matrix volSpreads;
        std::vector<std::vector<Handle<Quote> > > volSpreadsHandle;
        std::vector<Spread> strikeSpreads;
        void setMarketData() {
            tenors.options.resize(3);
            tenors.options[0] = Period(1, Years);
            tenors.options[1] = Period(10, Years);
            tenors.options[2] = Period(30, Years);
            tenors.swaps.resize(3);
            tenors.swaps[0] = Period(2, Years);
            tenors.swaps[1] = Period(10, Years);
            tenors.swaps[2] = Period(30, Years);
            strikeSpreads.resize(5);
            strikeSpreads[0] = -0.020;
            strikeSpreads[1] = -0.005;
            strikeSpreads[2] = +0.000;
            strikeSpreads[3] = +0.005;
            strikeSpreads[4] = +0.020;
            volSpreads = Matrix(tenors.options.size()*tenors.swaps.size(), strikeSpreads.size());
            volSpreads[0][0] = 0.0599; volSpreads[0][1] = 0.0049;
            volSpreads[0][2] = 0.0000;
            volSpreads[0][3] =-0.0001; volSpreads[0][4] = 0.0127;
            volSpreads[1][0] = 0.0729; volSpreads[1][1] = 0.0086;
            volSpreads[1][2] = 0.0000;
            volSpreads[1][3] =-0.0024; volSpreads[1][4] = 0.0098;
            volSpreads[2][0] = 0.0738; volSpreads[2][1] = 0.0102;
            volSpreads[2][2] = 0.0000;
            volSpreads[2][3] =-0.0039; volSpreads[2][4] = 0.0065;
            volSpreads[3][0] = 0.0465; volSpreads[3][1] = 0.0063;
            volSpreads[3][2] = 0.0000;
            volSpreads[3][3] =-0.0032; volSpreads[3][4] =-0.0010;
            volSpreads[4][0] = 0.0558; volSpreads[4][1] = 0.0084;
            volSpreads[4][2] = 0.0000;
            volSpreads[4][3] =-0.0050; volSpreads[4][4] =-0.0057;
            volSpreads[5][0] = 0.0576; volSpreads[5][1] = 0.0083;
            volSpreads[5][2] = 0.0000;
            volSpreads[5][3] =-0.0043; volSpreads[5][4] = -0.0014;
            volSpreads[6][0] = 0.0437; volSpreads[6][1] = 0.0059;
            volSpreads[6][2] = 0.0000;
            volSpreads[6][3] =-0.0030; volSpreads[6][4] =-0.0006;
            volSpreads[7][0] = 0.0533; volSpreads[7][1] = 0.0078;
            volSpreads[7][2] = 0.0000;
            volSpreads[7][3] =-0.0045; volSpreads[7][4] =-0.0046;
            volSpreads[8][0] = 0.0545; volSpreads[8][1] = 0.0079;
            volSpreads[8][2] = 0.0000;
            volSpreads[8][3] =-0.0042; volSpreads[8][4] =-0.0020;
            volSpreadsHandle = std::vector<std::vector<Handle<Quote> > >(tenors.options.size()*tenors.swaps.size());
            for (Size i=0; i<tenors.options.size()*tenors.swaps.size(); i++){
                volSpreadsHandle[i] = std::vector<Handle<Quote> >(strikeSpreads.size());
                for (Size j=0; j<strikeSpreads.size(); j++) {
                    // every handle must be reassigned, as the ones created by
                    // default are all linked together.
                    volSpreadsHandle[i][j] = Handle<Quote>(ext::shared_ptr<Quote>(new
                        SimpleQuote(volSpreads[i][j])));
                }
            }
        };
    };


 /*   static void setupCubeUtilities() {
        conventions_.calendar = TARGET();
        conventions_.optionBdc = Following;
        conventions_.dayCounter = Actual365Fixed();
        atm_.setMarketData();
        cube_.setMarketData();
        atmVolMatrix_ = RelinkableHandle<SwaptionVolatilityStructure>(
            ext::shared_ptr<SwaptionVolatilityStructure>(new
                SwaptionVolatilityMatrix(conventions_.calendar,
                                         atm_.tenors.options,
                                         atm_.tenors.swaps,
                                         atm_.volsHandle,
                                         conventions_.dayCounter,
                                         conventions_.optionBdc)));
    }*/

}

#endif
