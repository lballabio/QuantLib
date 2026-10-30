#include <ql/currencies/exchangeratemanager.hpp>
#include <ql/currencies/europe.hpp>
#include <ql/currencies/america.hpp>
#include <ql/currencies/asia.hpp>
#include <ql/currencies/oceania.hpp>
#include <ql/settings.hpp>
#include <fuzzer/FuzzedDataProvider.h>

using namespace QuantLib;

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
    FuzzedDataProvider fdp(data, size);
    SavedSettings saved_settings;

    try {
        ExchangeRateManager& erm = ExchangeRateManager::instance();

        if (fdp.ConsumeBool()) {
            erm.clear();
        }

        const Currency currencies[] = {
            EURCurrency(), USDCurrency(), GBPCurrency(), JPYCurrency(),
            CHFCurrency(), CADCurrency(), AUDCurrency(), HKDCurrency()
        };

        size_t numRates = fdp.ConsumeIntegralInRange<size_t>(0, 10);
        for (size_t i = 0; i < numRates; ++i) {
            Currency c1 = fdp.PickValueInArray(currencies);
            Currency c2 = fdp.PickValueInArray(currencies);
            if (c1 == c2) continue;

            Rate rate = fdp.ConsumeFloatingPointInRange<Rate>(0.01, 2.0);
            ExchangeRate er(c1, c2, rate);

            Date d1(fdp.ConsumeIntegralInRange<int>(1, 31),
                    static_cast<Month>(fdp.ConsumeIntegralInRange<int>(1, 12)),
                    fdp.ConsumeIntegralInRange<int>(1901, 2199));
            Date d2 = d1 + fdp.ConsumeIntegralInRange<int>(1, 365);
            
            erm.add(er, d1, d2);
        }

        Currency s = fdp.PickValueInArray(currencies);
        Currency t = fdp.PickValueInArray(currencies);
        Date d(fdp.ConsumeIntegralInRange<int>(1, 31),
               static_cast<Month>(fdp.ConsumeIntegralInRange<int>(1, 12)),
               fdp.ConsumeIntegralInRange<int>(1901, 2199));
        ExchangeRate::Type type = fdp.PickValueInArray({ExchangeRate::Direct, ExchangeRate::Derived});
        
        try {
            (void)erm.lookup(s, t, d, type);
        } catch (...) {}

    } catch (...) {}
    return 0;
}
