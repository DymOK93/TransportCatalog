#include "utils.h"

#include <iostream>
#include <functional>
#ifdef WINDOWS_DEBUG
#include <Windows.h>
#endif
using namespace std;

int main()
{
#ifdef WINDOWS_DEBUG
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif
    []() {
        cin.tie(nullptr);
        ios_base::sync_with_stdio(false);
    }();
    TransportCatalog tr_catalog;

    string raw_json_doc{ Json::Read(cin) };

    Json::Document doc{ Json::Load(
       raw_json_doc
    ) };

    const auto& [base, stat] {SplitByCategories(doc)};

    request::Read::Storage result;

    unique_ptr<request::IFactory> base_factory{ make_unique<request::ModifyRequestFactory>(request::Modify::Settings{ tr_catalog }) },
        stat_factory{ make_unique<request::ReadRequestFactory>(request::Read::Settings{tr_catalog, result}) };

    auto base_update{ MakeHandlers(base_factory.get(), base) },
        base_stat{ MakeHandlers(stat_factory.get(), stat) };

    ProcessRequests(base_update);

    tr_catalog.SetRoutingSettings(
        ExtractRoadSettings(doc)
    );
#ifdef RENDER
    tr_catalog.SetRenderSettings(
        ExtractRenderSettings(doc)
    );
#endif
    tr_catalog.Synchronize();

    ProcessRequests(base_stat);
    cout << SerializeResult(result);

    return 0;
}
