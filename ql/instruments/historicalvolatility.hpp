
#ifndef quantlib_historical_volatility_hpp
#define quantlib_historical_volatility_hpp

#include <ql/quantlib.hpp>
#include <vector>
using namespace std;

namespace QuantLib {

    class HistoricalVolatility {
      public:
        static Volatility calculateHistoricalVolatility(vector<double>& stockPrices,
          double lengthOfTimeIntervalInYears) {
          QL_ENSURE(3 <= stockPrices.size(), "the stockPrice vector must have a size at least equal to 3");
          double result = 0;

          double currentStock;
          double previousStock;

          double logreturn = 0;
          vector<double> logreturns;
          double meanlogreturn = 0;
          int size = stockPrices.size();
          for (int i = 0; i < size; i++) {
              currentStock = stockPrices[i];
              if (0 < i) {
                  logreturn = log(currentStock / previousStock);

                  logreturns.push_back(logreturn);
                  meanlogreturn = meanlogreturn + logreturn;
              }
              previousStock = currentStock;
          }
          meanlogreturn = meanlogreturn / (double)(size - 1);

          double standarddeviation = 0;
          double element = 0;
          double element2 = 0;
          for (int j = 0; j < size - 1; j++) {
              element = meanlogreturn - logreturns[j];
              element2 = element * element;
              standarddeviation = standarddeviation + element2;
          }
          standarddeviation = standarddeviation / (double)(size - 2);
          standarddeviation = sqrt(standarddeviation);
          result = standarddeviation / sqrt(lengthOfTimeIntervalInYears);
          return result;
      }

    };


};
#endif
