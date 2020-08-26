#include <memory>
#include <mutex>
#include <utility>
#include <functional>

template <class Ty>
class Synchronized {
public:
	Synchronized(Ty init = Ty())
		: value{std::move(init) } {}
	~Synchronized() = default;

	Synchronized(const Synchronized&) = delete;
	Synchronized& operator=(const Synchronized&) = delete;

	Synchronized(Synchronized&& other) 
		: value{ std::move(other.value) } {}

	Synchronized& operator=(Synchronized&& other) {
		if (std::addressof(other) != this) {
			this->GetAccess().ref = std::move(other.value);
		}
		return *this;
	}
private:
	template <class RefTy>
	struct Proxy {
		std::lock_guard<std::mutex> guard;
		RefTy& ref_to_value;
	};
public:
	Proxy<Ty> GetAccess() {
		return { std::lock_guard(mtx), value };
	}

	Proxy<const Ty> GetAccess() const {
		return { std::lock_guard(mtx), value };
	}
private:
	Ty value;
	mutable std::mutex mtx;
};