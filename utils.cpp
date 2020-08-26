#include "utils.h"

using Json::Node;
using Json::Document;
using request::HandlerHolder;
using request::IFactory;
using routing::Parameters;
using namespace std;

const Json::Node& GetBranch(const Json::Document& doc, const std::string& section) {
    return doc.GetRoot().AsMap().at(section);
}

Parameters ExtractRoadSettings(const Document& doc) {
    const auto& road_settings_map{ GetBranch(doc, "routing_settings").AsMap() };
    return {
        road_settings_map.at("bus_wait_time").AsNumber(),
        road_settings_map.at("bus_velocity").AsNumber()
    };
}

#ifdef RENDER
render::Settings ExtractRenderSettings(const Json::Document& doc) {
    const auto& render_settings_map{ GetBranch(doc, "render_settings").AsMap() };
    return render::Settings{
        .map = render::GetMapSettings(render_settings_map),
        .route = render::GetRouteSettings(render_settings_map),
        .stop_label = render::GetStopLabelSettings(render_settings_map),
        .bus_label = render::GetBusLabelSettings(render_settings_map),
        .substrate = render::GetSubstrateSettings(render_settings_map),
        .palette = render::GetPalette(render_settings_map),
        .layer_sequence = render::GetLayersSequence(render_settings_map)
    };
}
#endif

#ifdef MULTITHREADING
vector<HandlerHolder> MakeHandlers(IFactory* factory, const vector<Node>& raw_requests) {
    Synchronized<vector<HandlerHolder>> handlers;
    algo::execution::parallel_for(
        raw_requests.begin(),
        raw_requests.end(),
        [factory, &handlers](const Node& node) {
            auto handler{ factory->Create(node) };
            handlers.GetAccess().ref_to_value.push_back(move(handler));
        }
    );
    return move(handlers.GetAccess().ref_to_value);     //We create vector<HandlerHolder> here
}
#else
vector<HandlerHolder> MakeHandlers(IFactory* factory, const vector<Node>& raw_requests) {
    vector<HandlerHolder> handlers;
    for (const auto& node : raw_requests) {
        handlers.push_back(factory->Create(node));
    }
    return handlers;
}
#endif

#ifdef MULTITHREADING
void ProcessRequests(vector<request::HandlerHolder>& handlers) {
    algo::execution::parallel_for(
        handlers.begin(),
        handlers.end(),
        [](request::HandlerHolder& handler) {
            handler->Process();
        });
}
#else
void ProcessRequests(vector<request::HandlerHolder>& handlers) {
    for (auto& handler : handlers) {
        handler->Process();
    }
}
#endif

string SerializeResult(const request::Read::Storage& result) {
#ifdef MULTITHREADING
    return Json::Serialize(Json::Document(Json::Node(result.GetAccess().ref_to_value)));
#else
    return Json::Serialize(Json::Document(Json::Node(result)));
#endif
}