/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2010 Manas Bhatt

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

#include "toplevelfixture.hpp"
#include "utilities.hpp"
#include <ql/experimental/commodities/unitofmeasureconversionmanager.hpp>
#include <ql/experimental/commodities/petroleumunitsofmeasure.hpp>

using namespace QuantLib;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantLibTest, TopLevelFixture)

BOOST_AUTO_TEST_SUITE(CommodityUnitOfMeasureExperimentalTest)

BOOST_AUTO_TEST_CASE(testDirect) {

    BOOST_TEST_MESSAGE("Testing direct commodity unit of measure conversions...");

    UnitOfMeasureConversionManager& UOMManager =
        UnitOfMeasureConversionManager::instance();

    //MB to BBL
    Quantity actual =
        UnitOfMeasureConversion(NullCommodityType(), MBUnitOfMeasure(),
                                BarrelUnitOfMeasure(), 1000)
        .convert(Quantity(NullCommodityType(), MBUnitOfMeasure(), 1000));
    Quantity calc =
        UOMManager.lookup(NullCommodityType(), BarrelUnitOfMeasure(),
                          MBUnitOfMeasure(), UnitOfMeasureConversion::Direct)
        .convert(Quantity(NullCommodityType(), MBUnitOfMeasure(), 1000));

     if (!close(calc,actual)) {
        BOOST_FAIL("Wrong result for MB to BBL Conversion: \n"
                   << "    actual:     " << actual << "\n"
                   << "    calculated: " << calc);
    }

     //BBL to Gallon 
     actual =
         UnitOfMeasureConversion(NullCommodityType(), BarrelUnitOfMeasure(),
                                 GallonUnitOfMeasure(), 42)
         .convert(Quantity(NullCommodityType(), GallonUnitOfMeasure(), 1000));
     calc =
         UOMManager.lookup(NullCommodityType(), BarrelUnitOfMeasure(),
                           GallonUnitOfMeasure(),
                           UnitOfMeasureConversion::Direct)
         .convert(Quantity(NullCommodityType(), GallonUnitOfMeasure(), 1000));

     if (!close(calc,actual)) {
        BOOST_FAIL("Wrong result for BBL to Gallon Conversion: \n"
                   << "    actual:     " << actual << "\n"
                   << "    calculated: " << calc);
     }

     //BBL to Litre 
     actual =
         UnitOfMeasureConversion(NullCommodityType(), BarrelUnitOfMeasure(),
                                 LitreUnitOfMeasure(), 158.987)
         .convert(Quantity(NullCommodityType(), LitreUnitOfMeasure(), 1000));
     calc =
         UOMManager.lookup(NullCommodityType(),BarrelUnitOfMeasure(),
                           LitreUnitOfMeasure(),
                           UnitOfMeasureConversion::Direct)
         .convert(Quantity(NullCommodityType(), LitreUnitOfMeasure(), 1000));

     if (!close(calc,actual)) {
        BOOST_FAIL("Wrong result for BBL to Litre Conversion: \n"
                   << "    actual:     " << actual << "\n"
                   << "    calculated: " << calc);
     }

     //BBL to KL 
     actual =
         UnitOfMeasureConversion(NullCommodityType(), KilolitreUnitOfMeasure(),
                                 BarrelUnitOfMeasure(), 6.28981)
         .convert(Quantity(NullCommodityType(),KilolitreUnitOfMeasure(),1000));
     calc =
         UOMManager.lookup(NullCommodityType(),BarrelUnitOfMeasure(),
                           KilolitreUnitOfMeasure(),
                           UnitOfMeasureConversion::Direct)
         .convert(Quantity(NullCommodityType(),KilolitreUnitOfMeasure(),1000));

     if (!close(calc,actual)) {
        BOOST_FAIL("Wrong result for BBL to KiloLitre Conversion: \n"
                   << "    actual:     " << actual << "\n"
                   << "    calculated: " << calc);
     }

     //MB to Gallon 
     actual =
         UnitOfMeasureConversion(NullCommodityType(), GallonUnitOfMeasure(),
                                 MBUnitOfMeasure(), 42000)
         .convert(Quantity(NullCommodityType(),MBUnitOfMeasure(),1000));
     calc =
         UOMManager.lookup(NullCommodityType(),GallonUnitOfMeasure(),
                           MBUnitOfMeasure(), UnitOfMeasureConversion::Direct)
         .convert(Quantity(NullCommodityType(),MBUnitOfMeasure(),1000));

     if (!close(calc,actual)) {
        BOOST_FAIL("Wrong result for MB to Gallon Conversion: \n"
                   << "    actual:     " << actual << "\n"
                   << "    calculated: " << calc);
     }

     //Gallon to Litre 
     actual =
         UnitOfMeasureConversion(NullCommodityType(), LitreUnitOfMeasure(),
                                 GallonUnitOfMeasure(), 3.78541)
         .convert(Quantity(NullCommodityType(),LitreUnitOfMeasure(),1000));
     calc =
         UOMManager.lookup(NullCommodityType(),GallonUnitOfMeasure(),
                           LitreUnitOfMeasure(),
                           UnitOfMeasureConversion::Direct)
         .convert(Quantity(NullCommodityType(),LitreUnitOfMeasure(),1000));

     if (!close(calc,actual)) {
        BOOST_FAIL("Wrong result for Gallon to Litre Conversion: \n"
                   << "    actual:     " << actual << "\n"
                   << "    calculated: " << calc);
     }
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
