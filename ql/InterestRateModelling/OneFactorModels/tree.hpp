
#ifndef quantlib_interest_rate_modelling_tree_h
#define quantlib_interest_rate_modelling_tree_h

#include "ql/InterestRateModelling/OneFactorModels/node.hpp"

namespace QuantLib {
    namespace InterestRateModelling {
        class Tree {
          public:
            Tree(unsigned n) : depth_(1), nbDescendants_(n), 
                nodes_(1, std::vector<Node>(1, Node(n,0,0))) {
            }
            virtual ~Tree() {}

            virtual Node& node(unsigned i, signed j) = 0;

          protected:
            unsigned depth_;
            unsigned nbDescendants_;
            std::vector<std::vector<Node> > nodes_;
        };


        class BinomialTree : public Tree {
          public:
            BinomialTree() : Tree(2) {}
            Node& node(unsigned i, signed j) {
                return nodes_[i][(i+j)/2];
            }
            
        };

        class TrinomialTree : public Tree {
          public:
            TrinomialTree() : Tree(3) {}
            double calcDiscount();
            void addLevel(const std::vector<signed>& k);
            Node& node(unsigned i, signed j) {
                signed jMin = nodes_[i][0].j();
                return nodes_[i][j-jMin];
            }
        };

    }

}

#endif
