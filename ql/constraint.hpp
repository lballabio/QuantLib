
#ifndef quantlib_constraint_h
#define quantlib_constraint_h

#include "ql/errors.hpp"
#include "ql/types.hpp"
#include <vector>

namespace QuantLib {

    class Constraint {
      public:
        Constraint(unsigned int size)
        : minParams_(size, QL_MIN_DOUBLE), maxParams_(size, QL_MAX_DOUBLE) {}
        virtual ~Constraint() {}

        void setLowerBound(unsigned int i, double boundary) {
            minParams_[i] = boundary;
        }

        void setUpperBound(unsigned int i, double boundary) {
            maxParams_[i] = boundary;
        }

        virtual bool operator()(const std::vector<double>& params) const {
            Size size(params.size());
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
