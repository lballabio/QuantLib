*** QuantLib-old/ql/Pricers/europeanoption.cpp	Tue Oct 22 11:27:05 2002
--- QuantLib/ql/Pricers/europeanoption.cpp	Tue Oct 22 11:23:34 2002
***************
*** 30,37 ****
  
      namespace Pricers {
  
-         const Math::CumulativeNormalDistribution EuropeanOption::f_;
- 
          EuropeanOption::EuropeanOption(Option::Type type, double underlying,
              double strike, Spread dividendYield, Rate riskFreeRate,
              Time residualTime, double volatility)
--- 30,35 ----
