
#include <ql/quantlib.hpp>

using namespace QuantLib;

namespace Foo {

    int bar(int i) {
        QL_TRACE_ENTER_FUNCTION(Tracing::Info);
        QL_TRACE_VARIABLE(Tracing::Info, i);

        if (i == 42) {
            QL_TRACE_LOCATION(Tracing::Info);
            QL_TRACE(Tracing::Info, "Right answer, but no question");
        } else {
            QL_TRACE_LOCATION(Tracing::Info);
            QL_TRACE(Tracing::Info, "Wrong answer");
            i *= 2;
        }

        QL_TRACE_VARIABLE(Tracing::Info, i);
        QL_TRACE_EXIT_FUNCTION(Tracing::Info);
        return i;
    }

}

int main() {

    using namespace Foo;
    Tracing::instance().enable();

    QL_TRACE_ENTER_FUNCTION(Tracing::Info);

    int i = 21;
    int j = bar(i);
    int k = bar(j);

    QL_TRACE_EXIT_FUNCTION(Tracing::Info);
    return 0;
}

