#ifndef quantlib_constraint_h
#define quantlib_constraint_h

#include <vector>
#include "ql/errors.hpp"

namespace QuantLib {

    class Constraint {
      public:
        Constraint(std::vector<double> minParams, std::vector<double> maxParams) :
            minParams_(minParams), maxParams_(maxParams) {
            QL_REQUIRE(minParams_.size() == maxParams_.size(), 
              "min. and max. bounds are not of the same size");
        }
        virtual ~Constraint() {}

        virtual bool operator()(const std::vector<double>& params) const {
            size_t size(params.size());
            QL_REQUIRE(size == minParams_.size(),
              "parameter vector is not of appropriate size");
            for (unsigned i=0; i<size; i++) {
                if ((minParams_[i]>=params[i]) || (maxParams_[i]<=params[i]))
                    return false;
            }
            return true;
        }
        double minParam(unsigned i) const { return minParams_[i]; }
        double maxParam(unsigned i) const { return maxParams_[i]; }

      private:
        std::vector<double> minParams_;
        std::vector<double> maxParams_;

    };
}

#endif
