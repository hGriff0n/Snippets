#include <algorithm>
#include <iostream>
#include <iterator>

constexpr const auto &lorem_ipsum = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Curabitur eu lorem sed odio varius vestibulum et eu ante. Quisque rutrum, sem vitae accumsan finibus, enim elit mattis urna, gravida rhoncus erat sem quis lectus. Donec ultrices pretium arcu, rhoncus facilisis eros lobortis sit amet. Quisque vitae lorem at ante ultricies pulvinar. Sed suscipit faucibus tempus. Donec ut sem felis. Ut porttitor libero justo, ultrices egestas purus cursus cursus. Fusce et sapien felis. Phasellus ut ornare arcu. Vestibulum eget finibus dui. Sed quam sem, efficitur vitae risus egestas, vehicula vestibulum est. Nulla rutrum tempus mollis. Nunc a elementum felis";

// Use your code below from the previous part as a starting point.
template<size_t N>
constexpr auto lorem_len(const char(&)[N]) { return N; }

class lorem_view { // constexpr string
    private:
        const std::size_t sz_;
    public:
        constexpr lorem_view(size_t sz) : sz_{ sz } {}
        constexpr std::size_t size() const { return sz_; } // size()
};

std::ostream& operator<<(std::ostream& s, const lorem_view& str) {
    constexpr auto len = lorem_len(lorem_ipsum);
    for (size_t i = 0; i <= str.size(); ++i) s << lorem_ipsum[i % len];
    return s;
}

constexpr lorem_view operator"" _lorem(unsigned long long N) {
    return lorem_view{ static_cast<size_t>(N) };
}

int main() {
   constexpr auto text = 35_lorem;
   static_assert(text.size() == 35);
   
   std::cout << text << std::endl;
}