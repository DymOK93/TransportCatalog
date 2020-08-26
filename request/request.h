#pragma once
#include "transport_catalog.h"
#include "json.h"

#ifdef MULTITHREADING
/*Synchronized access to the results store*/
#include "synchronized.h"
#endif

/*Standart headers*/
#include <memory>
#include <string>

namespace request {
	enum class Type {
		ADD_STOP,
		ADD_BUS,
		ROUTING_SETTINGS,
		STOP_INFO,
		BUS_INFO,
		ROUTE_INFO,
		MAP
	};

	class Handler {
	public:
#ifdef MULTITHREADING
		using Storage = Synchronized<std::vector<Json::Node>>;
#else
		using Storage = std::vector<Json::Node>;
#endif
		const Type request_type;
	public:
		Handler(Type request_type_) noexcept;
		virtual ~Handler() = default;
		virtual void Parse(const Json::Node& request) = 0;
		virtual void Process() = 0;
	};

	using HandlerHolder = std::unique_ptr<Handler>;

	/*For read (stat) requests*/
	class Read : public Handler {
	public:

		/*Settings for handler and factory*/
		struct Settings {
			const TransportCatalog& tr_catalog;
			Storage& out;
		};
	public:
		Read(Read::Settings settings, Type type_) noexcept;
		virtual void Parse(const Json::Node& request) = 0;
	protected:
		Settings settings;	//Handler settings (catalog and output)
		uint64_t id;	//request_id
		std::string_view name;	//bus or stop name
	protected:
		/*Answer type is std::decay_t<Json::Node::AsMap()>*/
		using Answer = Json::map_t;

		/*Creates Json::map_t with request_id*/
		Answer create_answer() const;

		/*Add answer to storage*/
		void add_to_storage(Answer answer);

		static void add_error_message(Answer* answer, std::string error_message = "not found");
	};

	class BusDatabase : public Read {
	public:
		BusDatabase(Read::Settings settings_) noexcept;
		virtual void Process() override;
		virtual void Parse(const Json::Node& request) override;
	};

	class StopInfo : public Read {
	public:
		StopInfo(Read::Settings settings_) noexcept;
		virtual void Process() override;
		virtual void Parse(const Json::Node& request) override;
	};

	class RouteInfo : public Read {
	public:
		RouteInfo(Read::Settings settings_) noexcept;
		virtual void Parse(const Json::Node& request) override;
		virtual void Process() override;
	protected:
		routing::Bounds routing_stops;
	private:
		static std::vector<Json::Node> combine_routings_items(const routing::OnMap& route_un_map);
		static void add_wait_info(std::vector<Json::Node>* storage, const routing::Point& wait);
		static void add_trip_info(std::vector<Json::Node>* storage, const routing::Point& trip);
	};

#ifdef RENDER
	class Map : public Read {
	public:
		Map(Read::Settings settings_) noexcept;
		virtual void Parse(const Json::Node& request) override;
		virtual void Process() override;
	};
#endif

	/*For modify (update) requests*/
	class Modify : public Handler {
	public:
		struct Settings {
			TransportCatalog& tr_catalog;
		};
	public:
		Modify(Modify::Settings settings_, Type type_) noexcept;
	protected:
		Modify::Settings settings;
	};

	class AddStop : public Modify {
	public:
		AddStop(Modify::Settings settings_) noexcept;
		virtual void Parse(const Json::Node& request) override;
		virtual void Process() override;
	protected:
		geographic::Stop stop;
	};

	class AddBus : public Modify {
	public:
		AddBus(Modify::Settings settings_) noexcept;
		virtual void Parse(const Json::Node& request) override;
		virtual void Process() override;
	protected:
		geographic::Bus bus;
	};

	struct IFactory {
		virtual HandlerHolder Create(const Json::Node& request) const = 0;
		virtual ~IFactory() = default;
	};

	using FactoryHolder = std::unique_ptr<IFactory>;

	class ReadRequestFactory : public IFactory {
	public:
		ReadRequestFactory(Read::Settings settings_) noexcept;
		virtual HandlerHolder Create(const Json::Node& request) const override;
	protected:
		Read::Settings settings;
	};

	class ModifyRequestFactory : public IFactory {
	public:
		ModifyRequestFactory(Modify::Settings settings_) noexcept;
		virtual HandlerHolder Create(const Json::Node& request)  const override;
	protected:
		Modify::Settings settings;
	};
}

