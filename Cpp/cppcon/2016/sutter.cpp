#include <algorithm>
#include <iostream>
#include <type_traits>

class CIString {
    std::string internal;
    
    public:
        template<class S, class=std::enable_if_t<std::is_constructible_v<std::string, S>>>
        CIString(S in) : internal{ in } {}
        
        friend bool operator<(const CIString&, const CIString&);
};

// Could use `operator<=>` here to simplify it further
bool operator<(const CIString& lhs, const CIString& rhs) {
    for (auto i=0; i != lhs.internal.size(); ++i) {
        auto lchar = std::tolower(lhs.internal[i]),
             rchar = std::tolower(rhs.internal[i]);
             
        if (lchar < rchar) return true;
        if (lchar > rchar) return false;
    }
    
    return lhs.internal.size() < rhs.internal.size();
}
bool operator==(const CIString& lhs, const CIString& rhs) {
    return !(lhs < rhs) && !(rhs < lhs);
}
bool operator!=(const CIString& lhs, const CIString& rhs) {
    return !(lhs == rhs);
}
bool operator<=(const CIString& lhs, const CIString& rhs) {
    return !(rhs < lhs);
}
bool operator>(const CIString& lhs, const CIString& rhs) {
    return rhs < lhs;
}
bool operator>=(const CIString& lhs, const CIString& rhs) {
    return rhs <= lhs;
}