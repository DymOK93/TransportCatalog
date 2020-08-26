#include "request.h"

/*Output with a given precision*/
#include <iomanip>

using namespace std;

namespace request {
	Handler::Handler(Type request_type_) noexcept
		: request_type{ request_type_ }
	{
	}


	Read::Read(Settings settings_, Type type_) noexcept
		: Handler(type_), settings{ settings_ }, id{ 0 }
	{
	}

	void Read::Parse(const Json::Node& request) {
		id = request.AsMap().at("id").AsNumber();
	}

	Read::Answer Read::create_answer() const {
		//cout << id << '\n';
		;
		return Answer{
				{
				/*field name*/"request_id",
				Json::Number(Json::NumberHolder{
				/*negative*/ false,
				/*whole*/ id,
				/*fractional*/ 0,
				/*fractional_length*/ 0
				})
		}
		};
	}

#ifdef MULTITHREADING
	void Read::add_to_storage(Answer answer) {
		settings.out.GetAccess().ref_to_value.push_back(Json::Node(move(answer)));
	}
#else
	void Read::add_to_storage(Answer answer) {
		settings.out.emplace_back(move(answer));
	}
#endif

	void Read::add_error_message(Answer* answer, string error_message) {
		answer->insert({
			"error_message",
			move(error_message)
			});
	}

	BusDatabase::BusDatabase(Settings settings_) noexcept
		: Read(settings_, Type::BUS_INFO)
	{
	}

	void BusDatabase::Parse(const Json::Node& request) {
		Read::Parse(request);
		name = request.AsMap().at("name").AsString();
	}

	void BusDatabase::Process() {
		auto answer{ Read::create_answer() };

		const auto bus_info{ settings.tr_catalog.GetBusInfo(name) };
		if (!bus_info) {
			add_error_message(addressof(answer));
		}
		else {
			answer.insert({ "route_length", Json::Number(bus_info->distance.real) });
			answer.insert({ "curvature", Json::Number(bus_info->distance.real / bus_info->distance.geographic) });
			answer.insert({ "stop_count", Json::Number(static_cast<uint64_t>(bus_info->stops)) });
			answer.insert({ "unique_stop_count", Json::Number(static_cast<uint64_t>(bus_info->unique_stops)) });
		}
		add_to_storage(move(answer));
	}

	StopInfo::StopInfo(Settings settings_) noexcept
		: Read(settings_, Type::STOP_INFO)
	{
	}

	void StopInfo::Parse(const Json::Node& request) {
		Read::Parse(request);
		name = request.AsMap().at("name").AsString();
	}

	void StopInfo::Process() {
		auto answer{ Read::create_answer() };

		const auto stop_info{ settings.tr_catalog.GetStopInfo(name) };
		if (!stop_info) {
			add_error_message(addressof(answer));
		}
		else {
			vector<Json::Node> buses;
			for (auto bus : stop_info->range) {
				buses.emplace_back(string(bus));
			}
			answer.insert({ "buses", move(buses) });
		}
		add_to_storage(move(answer));
	}

	RouteInfo::RouteInfo(Read::Settings settings_) noexcept
		: Read(settings_, Type::ROUTE_INFO)
	{
	}

	void RouteInfo::Parse(const Json::Node& request) {
		Read::Parse(request);
		const auto& route_map{ request.AsMap() };
		routing_stops.from = route_map.at("from").AsString();
		routing_stops.to = route_map.at("to").AsString();
	}

	void RouteInfo::Process() {
		auto answer{ Read::create_answer() };
		auto routing{ settings.tr_catalog.GetRouting(routing_stops) };
		if (!routing) {
			add_error_message(addressof(answer));
		}
		else {
			answer.insert({ "total_time", Json::Number(routing->total_time) });
			answer.insert({ "items", Json::Node(combine_routings_items(*routing)) });
		}
		add_to_storage(move(answer));
	}

	vector<Json::Node> RouteInfo::combine_routings_items(const routing::OnMap& route_un_map) {
		using routing::Point;
		vector<Json::Node> items_storage;
		for (const auto& point : route_un_map.items) {
			if (point.type == Point::Type::WAIT) {
				add_wait_info(addressof(items_storage), point);
			}
			else {
				add_trip_info(addressof(items_storage), point);
			}
		}
		return items_storage;
	}

	void RouteInfo::add_wait_info(vector<Json::Node>* storage, const routing::Point& wait) {
		storage->push_back(
			Answer{
			{"type", Json::Node("Wait"s)},
			{"stop_name", Json::Node(string(wait.name))},
			{"time", Json::Node(Json::Number(wait.time)) }
			}
		);
	}

	void RouteInfo::add_trip_info(vector<Json::Node>* storage, const routing::Point& trip) {
		storage->push_back(
			Answer{
			{"type", Json::Node("Bus"s)},
			{"bus", Json::Node(string(trip.name))},
			{"span_count", Json::Node(Json::Number(*trip.span_count))},
			{"time", Json::Node(Json::Number(trip.time)) }
			}
		);
	}

#ifdef RENDER
	Map::Map(Read::Settings settings_) noexcept
		: Read(settings_, Type::MAP) 
	{
	}
	void Map::Parse(const Json::Node& request) {
		Read::Parse(request);
	}
	void Map::Process() {
		auto answer{ Read::create_answer() };
		
		const auto& svg_doc{ settings.tr_catalog.GetMap() };
		answer.insert({
			"map",
			Json::Node(svg_doc.Render())
			});
		add_to_storage(move(answer));
	}
#endif

	Modify::Modify(Modify::Settings settings_, Type type_) noexcept
		: Handler(type_), settings{ settings_ }
	{
	}

	AddStop::AddStop(Settings settings_) noexcept
		: Modify(settings_, Type::ADD_STOP)
	{
	}

	void AddStop::Parse(const Json::Node& request) {
		const auto& stop_info{ request.AsMap() };
		stop.name = stop_info.at("name").AsString();
		stop.coordinates.latitude = stop_info.at("latitude").AsNumber();
		stop.coordinates.longitude = stop_info.at("longitude").AsNumber();
		
		const auto& road_distances{ stop_info.at("road_distances").AsMap() };
		for (const auto& [other_stop, distance] : road_distances) {
			stop.distances.insert({ other_stop, distance.AsNumber() });
		}
	}
	
	void AddStop::Process() {
		settings.tr_catalog.AddStop(stop);
	}

	
	AddBus::AddBus(Settings settings_) noexcept
		: Modify(settings_, Type::ADD_BUS)
	{
	}

	void AddBus::Parse(const Json::Node& request) {
		const auto& bus_info{ request.AsMap() };

		bus.name = bus_info.at("name").AsString();
		bus.is_roundtrip = bus_info.at("is_roundtrip").AsBool();

		const auto& stops{ bus_info.at("stops").AsArray() };
		for (const auto& stop : stops) {
			bus.stops.push_back(stop.AsString());
		}
	}

	void AddBus::Process() {
		settings.tr_catalog.AddBus(bus);
	}

	ReadRequestFactory::ReadRequestFactory(Read::Settings settings_) noexcept
		: settings{ settings_ }
	{
	}

	HandlerHolder ReadRequestFactory::Create(const Json::Node& request) const {
		const auto& type{ request.AsMap().at("type").AsString() };
		HandlerHolder handler;
		if (type == "Stop") {
			handler = make_unique<StopInfo>(settings);
		}
		else if (type == "Bus") {
			handler = make_unique<BusDatabase>(settings);
		}
		else if (type == "Route"){
			handler = make_unique<RouteInfo>(settings);
		}
#ifdef RENDER
		else if(type == "Map") {
			handler = make_unique<Map>(settings);
		}
#endif
		else {
			throw std::invalid_argument("Invalid request type");
		}
		/*Updating handler internal data*/
		handler->Parse(request);

		return handler;
	}

	ModifyRequestFactory::ModifyRequestFactory(Modify::Settings settings_) noexcept
		: settings{ settings_ }
	{
	}

	HandlerHolder ModifyRequestFactory::Create(const Json::Node& request) const {
		const auto& type{ request.AsMap().at("type").AsString() };
		HandlerHolder handler;
		if (type == "Stop") {
			handler = make_unique<AddStop>(settings);
		}
		else {
			handler = make_unique<AddBus>(settings);
		}	

		/*Updating handler internal data*/
		handler->Parse(request);

		return handler;
	}
}
			


