#pragma once
/*Sequential of parallel execution*/
#include "execution.h"

/*JSON serialization and deserialization*/
#include "json.h"

/*Transport catalog engine*/
#include "transport_catalog.h"

/*Requests handling*/
#include "request.h"

#ifdef RENDER
/*2D vector graphical primitives and settings*/
#include "render.h"
#include "render_settings_extractor.h"
#endif

/*Standart headers*/
#include <vector>
#include <tuple>

std::vector<request::HandlerHolder> MakeHandlers(
    request::IFactory* factory, 
    const std::vector<Json::Node>& raw_requests
);

void ProcessRequests(std::vector<request::HandlerHolder>& handlers);
routing::Parameters ExtractRoadSettings(const Json::Document& doc);

#ifdef RENDER
render::Settings ExtractRenderSettings(const Json::Document& doc);
#endif

const Json::Node& GetBranch(const Json::Document& doc, const std::string& section);

inline decltype(auto) SplitByCategories(const Json::Document& doc) {       //Without inline we get an ODR error
    return std::tie(
        GetBranch(doc, "base_requests").AsArray(),
        GetBranch(doc, "stat_requests").AsArray()
    );
}

std::string SerializeResult(const request::Read::Storage& result);

