#!/usr/bin/python

"""
 Copyright (C) 2000-2001 QuantLib Group

 This file is part of QuantLib.
 QuantLib is a C++ open source library for financial quantitative
 analysts and developers --- http://quantlib.sourceforge.net/

 QuantLib is free software and you are allowed to use, copy, modify, merge,
 publish, distribute, and/or sell copies of it under the conditions stated
 in the QuantLib License.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.

 You should have received a copy of the license along with this file;
 if not, contact ferdinando@ametrano.net
 The license is also available at http://quantlib.sourceforge.net/LICENSE.TXT

 The members of the QuantLib Group are listed in the Authors.txt file, also
 available at http://quantlib.sourceforge.net/Authors.txt
"""

"""
    $Id$
    $Source$
    $Log$
    Revision 1.8  2001/05/10 15:32:18  nando
    now when it runs as main it doesn't stop waiting for user keystroke anymore

    Revision 1.7  2001/04/27 10:44:15  lballabio
    Support for unittest in Python 2.1

    Revision 1.6  2001/04/20 10:52:37  nando
    smoothing the autobuild process

    Revision 1.5  2001/04/18 09:03:22  nando
    added/removed final
    raw_input('press any key to continue')

    Revision 1.4  2001/04/17 09:11:27  nando
    minor smoothing

    Revision 1.3  2001/04/09 11:28:17  nando
    updated copyright notice header and improved CVS tags

"""

import sys
import unittest

def test():
    suite = unittest.TestSuite()

    from american_option import AmericanOptionTest
    suite.addTest(AmericanOptionTest())

    from barrier_option import BarrierOptionTest
    suite.addTest(BarrierOptionTest())

    from binary_option import BinaryOptionTest
    suite.addTest(BinaryOptionTest())

    from cliquet_option import CliquetOptionTest
    suite.addTest(CliquetOptionTest())

    from date import DateTest
    suite.addTest(DateTest())

    from distributions import DistributionTest
    suite.addTest(DistributionTest())

    from european_option import EuropeanOptionTest
    suite.addTest(EuropeanOptionTest())

    from european_with_dividends import DividendEuropeanOptionTest
    suite.addTest(DividendEuropeanOptionTest())

    from everest_option import EverestOptionTest
    suite.addTest(EverestOptionTest())

    from finite_difference_european import FDEuropeanOptionTest
    suite.addTest(FDEuropeanOptionTest())

    from get_covariance import CovarianceTest
    suite.addTest(CovarianceTest())

    from himalaya_option import HimalayaOptionTest
    suite.addTest(HimalayaOptionTest())

    from implied_volatility import ImpliedVolatilityTest
    suite.addTest(ImpliedVolatilityTest())

    from montecarlo_pricers import MonteCarloPricerTest
    suite.addTest(MonteCarloPricerTest())

    from pagoda_option import PagodaOptionTest
    suite.addTest(PagodaOptionTest())

    from plain_basket_option import PlainBasketOptionTest
    suite.addTest(PlainBasketOptionTest())

    from random_generators import RNGTest
    suite.addTest(RNGTest())

    from risk_statistics import RiskStatisticsTest
    suite.addTest(RiskStatisticsTest())

    from statistics import StatisticsTest
    suite.addTest(StatisticsTest())

    from term_structures import TermStructureTest
    suite.addTest(TermStructureTest())

    if sys.hexversion >= 0x020100f0:
        result = unittest.TextTestRunner(verbosity=2).run(suite)
    else:
        result = unittest.TextTestRunner().run(suite)

    if not result.wasSuccessful:
        sys.exit(1)

if __name__ == '__main__':
    test()
