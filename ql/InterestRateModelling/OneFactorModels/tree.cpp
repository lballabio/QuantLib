#include "ql/InterestRateModelling/OneFactorModels/tree.hpp"

using std::vector;

namespace QuantLib {
    namespace InterestRateModelling {
        double TrinomialTree::calcDiscount() {
            unsigned iMax = depth_ - 1;
            for (signed j=nodes_[iMax].front().j(); j<=nodes_[iMax].back().j(); j++)
                node(iMax,j).setValue(1.0);
            for (signed i=iMax; i>=0; i--) {
                for (signed j=nodes_[i].front().j(); j<=nodes_[i].back().j(); j++) {
                    double value = 0.0;
                    for (unsigned k=0; k<3; k++)
                        value += node(i,j).probability(k)
                            *node(i,j).descendant(k).value();
                    node(i,j).setValue(value*node(i,j).discount());
                }
            }
            return node(0,0).value();
        }

        void TrinomialTree::addLevel(const vector<signed>& k) {
            unsigned i = depth_;
            depth_++;
            nodes_.resize(depth_);
            signed jMin = k.front() - 1;
            signed jMax = k.back() + 1;
            unsigned width = jMax - jMin + 1;
            nodes_[i].resize(width);

            unsigned index = 0;

            for (signed j=jMin; j<=jMax; j++) {
                nodes_[i][index] = Node(3, i, j);
                index++;
            }
            QL_REQUIRE(k.size()==nodes_[i-1].size(), "Error!!!");
            for (unsigned l=0; l<k.size(); l++) {
                nodes_[i-1][l].setDescendant(node(i, k[l] - 1), 0);
                nodes_[i-1][l].setDescendant(node(i, k[l]    ), 1);
                nodes_[i-1][l].setDescendant(node(i, k[l] + 1), 2);
            }
        }

    }

}

