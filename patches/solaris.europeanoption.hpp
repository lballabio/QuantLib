*** QuantLib-old/ql/Pricers/europeanoption.hpp	Sun May 19 15:24:06 2002
--- QuantLib/ql/Pricers/europeanoption.hpp	Tue Oct 22 11:23:17 2002
***************
*** 57,63 ****
              void setDividendYield(Rate newDividendYield);
              double beta() const;
            private:
!             static const Math::CumulativeNormalDistribution f_;
              double alpha() const;
              double standardDeviation() const;
              double D1() const;
--- 57,63 ----
              void setDividendYield(Rate newDividendYield);
              double beta() const;
            private:
!             Math::CumulativeNormalDistribution f_;
              double alpha() const;
              double standardDeviation() const;
              double D1() const;
