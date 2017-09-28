#include <iostream>
#include <regex>
#include <string>
#include <set>

#include <assert.h>

bool isWordChar(char& c) {
    return std::isalnum(c) || c == '_';
}

bool all_words_unique(std::string s) {
    std::set<std::string> uniq;
    
    auto front = std::begin(s);
    const auto back = std::end(s);
    while (front != back) {
        auto word_end = std::find_if(front, back, [](auto& c) { return !isWordChar(c); });
        
        std::string word{ front, word_end };
        if (uniq.count(word)) return false;
        uniq.insert(word);
        
        front = std::find_if(word_end, back, [](auto& c) { return isWordChar(c); });
    }
    
    return true;
}

using Num = long long;

bool isPalindrome(const std::vector<Num>& nums) {
    auto last = nums.size() - 1;
    for (size_t i = 0; i < last - i; ++i) {
        if (nums[i] != nums[last - i]) return false;
    }
    return true;
}

// TODO: Handle '999999999999999999999999999999' better
bool counts_up_and_down(std::string s)
{
    auto front = std::begin(s);
    const auto back = std::end(s);
    
    std::vector<Num> nums;
    
    while (front != back) {
        auto num_start = std::find_if(front, back, [](auto& c) { return !std::isspace(c); });
        if (num_start == back) break;
        
        front = std::find_if(num_start, back, [](auto& c) { return !std::isdigit(c); });
        if (front == num_start || std::distance(num_start, front) > 19) return false;
        
        nums.push_back(std::stoll(std::string{ num_start, front }));
    }
    
    return nums.size() % 2 && isPalindrome(nums) && nums.front() == 1;
}


// Print '1..n..1' without repeating a word
int main() {
    std::cout << "1\n";
}