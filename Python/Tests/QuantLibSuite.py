
import sys
import unittest
from american_option import AmericanOptionTest
from barrier_option import BarrierOptionTest
from binary_option import BinaryOptionTest
from cliquet_option import CliquetOptionTest
from date import DateTest
from distributions import DistributionTest
from european_option import EuropeanOptionTest
from european_with_dividends import DividendEuropeanOptionTest
from everest_option import EverestOptionTest
from finite_difference_european import FDEuropeanOptionTest
from get_covariance import CovarianceTest
from himalaya_option import HimalayaOptionTest
from implied_volatility import ImpliedVolatilityTest
from montecarlo_pricers import MonteCarloPricerTest
from pagoda_option import PagodaOptionTest
from plain_basket_option import PlainBasketOptionTest
from random_generators import RNGTest
from risk_statistics import RiskStatisticsTest
from statistics import StatisticsTest
from term_structures import TermStructureTest

suite = unittest.TestSuite()
suite.addTest(AmericanOptionTest())
suite.addTest(BarrierOptionTest())
suite.addTest(BinaryOptionTest())
suite.addTest(CliquetOptionTest())
suite.addTest(CovarianceTest())
suite.addTest(DateTest())
suite.addTest(DistributionTest())
suite.addTest(EuropeanOptionTest())
suite.addTest(DividendEuropeanOptionTest())
suite.addTest(EverestOptionTest())
suite.addTest(FDEuropeanOptionTest())
suite.addTest(HimalayaOptionTest())
suite.addTest(ImpliedVolatilityTest())
suite.addTest(MonteCarloPricerTest())
suite.addTest(PagodaOptionTest())
suite.addTest(PlainBasketOptionTest())
suite.addTest(RNGTest())
suite.addTest(RiskStatisticsTest())
suite.addTest(StatisticsTest())
suite.addTest(TermStructureTest())

result = unittest.TextTestRunner().run(suite)
if not result.wasSuccessful:
    sys.exit(1)




