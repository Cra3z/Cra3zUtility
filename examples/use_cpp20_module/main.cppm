import std;
import cra3zutil;

auto main() ->int {
	cra3zutil::move_only_function<void()> fun{[] {
		std::cout << "hello\n";
	}};
	cra3zutil::scope_exit scope_exit{[]() noexcept {
		std::cout << "bye\n";
	}};
	fun();
	using lst1 = cra3zutil::type_list<int, char, float, int, int>;
	static_assert(lst1::find<float> == 2 && !lst1::containes<double>);
	using lst2 = lst1::unique;
	static_assert(std::same_as<lst2, cra3zutil::type_list<int, char, float>>);
	using lst3 = lst2::append<double>;
	static_assert(std::same_as<lst3, cra3zutil::type_list<int, char, float, double>>);
	using lst4 = lst3::replace<int, short>;
	static_assert(std::same_as<lst4, cra3zutil::type_list<short, char, float, double>>);
}