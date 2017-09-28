#include "catch.hpp"

#include <algorithm>
#include <iostream>
#include <memory>
#include <numeric>
#include <set>
#include <string>
#include <vector>

// This class helps track instances for testing purposes
struct DebugNodeTracker {
    DebugNodeTracker();
    ~DebugNodeTracker();
    auto getSlotCount() const -> size_t;
};


struct Node;
using NodePtr = std::shared_ptr<Node>;

std::string safeSubStr(std::string const& s, size_t idx) {
    return idx < s.size() ? s.substr(idx) : "";
}

struct Node : DebugNodeTracker {
    std::vector<NodePtr> m_children;
    std::vector<char> keys;
    std::string m_value, common;
    
    auto findTrie(char c) const {
        int i = 0;
        for (auto k : keys) {
            if (c == k) return i;
            ++i;
        }
        return -1;
    }

    auto maxSubStrLen(std::string str) {
        int i = 0;
        auto maxlen = std::max(str.size(), common.size());
        while (i < maxlen && str[i] == common[i]) ++i;
        return i;
    }

    auto getAt(char c) const -> NodePtr {
        auto index = findTrie(c);
        return (index != -1) ? m_children[index] : nullptr;
    }

    auto getNext(std::string str) -> std::tuple<Node*, std::string> {
        auto len = maxSubStrLen(str);
        if (len == str.size()) return { this, "" };

        auto index = findTrie(str[len]);
        if (index == -1) return { nullptr, "" };
        return { m_children[index].get(), safeSubStr(str, len + 1) };
    }

    void setAt(char c, NodePtr const& newChild) {
        if (auto index = findTrie(c); index == -1) {
            keys.push_back(c);
            m_children.push_back(newChild);
        } else {
            m_children[index] = newChild;
        }
    }

    // I don't think it's splitting right
    auto splitWith(std::string str) {
        // Determine how the splitting will happen
        auto len = maxSubStrLen(str);

        // If the split occurs earlier than we have room for
            // Split off the subtries to the next level
        if (len < common.size()) {
            auto left = std::make_shared<Node>();
            m_children.swap(left->m_children);
            keys.swap(left->keys);

            keys.push_back(common[len]);
            m_children.push_back(left);
            
            left->common = safeSubStr(common, len + 1);
            common = common.substr(0, len);
            
            if (m_value != "") {
                m_value.swap(left->m_value);
            }
        }

        // Handle the new string being smaller than the old
        if (len == str.size()) {
            m_value = str;
            return;
        }

        // Otherwise add a new branch for the new string
            // NOTE: Population is guaranteed to be '1'
        auto right = std::make_shared<Node>();
        right->common = right->m_value = safeSubStr(str, len + 1);
        m_children.push_back(right);
        keys.push_back(str[len]);

        return right;
    }
};

class Trie {
    auto find(std::string str) {
        auto node = m_root.get();
        
        // The `getNext` function doesn't 
        while (true) {
            auto [next, left] = node->getNext(str);
            
            if (!next) break;
            node = next;
            
            if (left == "") break;
            str = left;
        }
        
        return std::tuple{ node, str };
    }

    NodePtr m_root = std::make_shared<Node>();

    public:
        enum class InsertionResult{ WasInserted, AlreadyExists };

        auto insert(std::string const& str) {
            // while (true);
            auto [node, sub] = find(str);

            if (node->m_value == str) return InsertionResult::AlreadyExists;
            if (node->m_value == "") {
                node->common = node->m_value = str;

            } else {
                node->splitWith(sub)->m_value = str;
            }

            return InsertionResult::WasInserted;
        }

        auto exists(std::string const& str) {
            auto [node, _] = find(str);
            return node->m_value == str;
        }
};

// You can use this function to write any tests you may want.
void tests() {

}

// Tracking for tests (static_casts are just so we can keep noisy test code out of the Node class)
std::set<Node*> g_allNodes;
DebugNodeTracker::DebugNodeTracker() { g_allNodes.insert( static_cast<Node*>( this ) ); }
DebugNodeTracker::~DebugNodeTracker() { g_allNodes.erase( static_cast<Node*>( this ) ); }

auto DebugNodeTracker::getSlotCount() const -> size_t { return static_cast<Node const*>( this )->m_children.size(); }

auto totalChildNodeSlots() {
    return std::accumulate( g_allNodes.begin(), g_allNodes.end(), 0, []( auto count, auto node ) { return count + node->getSlotCount(); } );
}

// Serialisation for result type enum, in case of test failure
auto operator <<( std::ostream& os, Trie::InsertionResult resultType ) -> std::ostream& {
    switch( resultType ) {
        case Trie::InsertionResult::WasInserted: return os << "WasInserted";
        case Trie::InsertionResult::AlreadyExists: return os << "AlreadyExists";
        default: return os << "{** Unrecognised enum value: " << (int)resultType << "**}";
    }
}

// Helper to build an example trie
auto buildTrie() {
    Trie trie;

    CHECK( trie.insert( "hello" ) == Trie::InsertionResult::WasInserted );
    CHECK( trie.insert( "hello" ) == Trie::InsertionResult::AlreadyExists );
    CHECK( trie.insert( "hell" ) == Trie::InsertionResult::WasInserted );
    CHECK( trie.insert( "hello world" ) == Trie::InsertionResult::WasInserted );

    return trie;
}

// This test case should pass out of the box
TEST_CASE( "Items can be inserted and looked up" ) {
    g_allNodes.clear();
    {
        auto trie = buildTrie();

        CHECK( trie.exists( "hello" ) );
        CHECK( trie.exists( "hell" ) );
        CHECK( trie.exists( "hello world" ) );
        CHECK( trie.exists( "hell no" ) == false );
        CHECK( trie.exists( "completely different" ) == false );
    }
    INFO( "All nodes should have been destroyed at this point" );
    CHECK( g_allNodes.size() == 0 );
}

// The tests in this test case will be failing to start with - the task is to make
// them pass. This will be a progressive endeavour
TEST_CASE( "The minimum child array memory is used" ) {

    g_allNodes.clear();

    auto trie = buildTrie();

    auto childNodeSlots = totalChildNodeSlots();
    SECTION( "good" )
        CHECK( childNodeSlots <= 11 );
    SECTION( "better" )
        CHECK( childNodeSlots <= 6 );
    SECTION( "best" )
        CHECK( childNodeSlots <= 3 );
}

TEST_CASE( "Your test cases" ) {
    tests();
}