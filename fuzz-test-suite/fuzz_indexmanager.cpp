#include <ql/indexes/indexmanager.hpp>
#include <ql/indexes/ibor/euribor.hpp>
#include <ql/settings.hpp>
#include <fuzzer/FuzzedDataProvider.h>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    SavedSettings saved_settings;

    try {
        IndexManager& im = IndexManager::instance();

        if (fdp.ConsumeBool()) {
            im.clearHistories();
        }

        ext::shared_ptr<IborIndex> euribor = ext::make_shared<Euribor6M>();
        
        size_t numFixings = fdp.ConsumeIntegralInRange<size_t>(0, 10);
        for (size_t i = 0; i < numFixings; ++i) {
            Date d(fdp.ConsumeIntegralInRange<int>(1, 31),
                   static_cast<Month>(fdp.ConsumeIntegralInRange<int>(1, 12)),
                   fdp.ConsumeIntegralInRange<int>(1901, 2199));
            if (euribor->isValidFixingDate(d)) {
                Real fixing = fdp.ConsumeFloatingPointInRange<Real>(-1.0, 1.0);
                try {
                    euribor->addFixing(d, fixing);
                } catch (...) {}
            }
        }

        Date d2(fdp.ConsumeIntegralInRange<int>(1, 31),
                static_cast<Month>(fdp.ConsumeIntegralInRange<int>(1, 12)),
                fdp.ConsumeIntegralInRange<int>(1901, 2199));
        (void)euribor->hasHistoricalFixing(d2);

        (void)im.histories();
        
        euribor->clearFixings();

    } catch (...) {}
    return 0;
}
