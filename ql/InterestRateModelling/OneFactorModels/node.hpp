#ifndef quantlib_interest_rate_modelling_node_h
#define quantlib_interest_rate_modelling_node_h

#include "ql/errors.hpp"
#include "ql/qldefines.hpp"
#include <vector>

namespace QuantLib {
    namespace InterestRateModelling {
        class Node {
          public:
            Node(unsigned nbDescendants = 1, unsigned i = 0, signed j = 0) 
            : nbDescendants_(nbDescendants), nbAscendants_(0),
              descendants_(nbDescendants), ascendants_(10), 
              ascendantBranches_(10), probabilities_(nbDescendants), 
              statePrice_(0.0), i_(i), j_(j) {}

            double value() const { return value_; }
            void setValue(double value) { value_ = value; }
            double discount() const { return discount_; }
            void setDiscount(double value) { discount_ = value; }
 
            unsigned i() const { return i_; }
            signed j() const { return j_; }

            double probability(unsigned int branch) const {
                return probabilities_[branch];
            }
            void setProbability(double prob, unsigned int branch) {
                probabilities_[branch] = prob;
            }
            void setProbabilities(const std::vector<double>& probabilities) {
                probabilities_ = probabilities;
            }
            Node& ascendant(unsigned int index) {
                return *ascendants_[index];
            }
            Node& descendant(unsigned int branch) { 
                return *descendants_[branch]; 
            }
            unsigned int ascendantBranch(unsigned int index) const {
                return ascendantBranches_[index];
            }
            unsigned int nbAscendants() const {
                return nbAscendants_;
            }
            void setDescendant(Node& node, unsigned int branch) {
                descendants_[branch] = &node; 
                node.addAscendant(*this, branch);
            }

            void setStatePrice(double price) { statePrice_ = price; }
            double statePrice() const { return statePrice_; }

          private:
            void addAscendant(Node& node, unsigned int branch) {
                ascendants_[nbAscendants_] = &node;
                ascendantBranches_[nbAscendants_] = branch;
                nbAscendants_++;
            }

            unsigned int nbDescendants_;
            unsigned int nbAscendants_;
            std::vector<Node*> descendants_;
            std::vector<Node*> ascendants_;
            std::vector<unsigned int> ascendantBranches_;
            std::vector<double> probabilities_;
            double statePrice_;
            unsigned i_;
            signed j_;
            double value_;
            double discount_;

        };
    }
}

#endif
