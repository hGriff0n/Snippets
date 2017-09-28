#include <fstream>
#include <sstream>
#include <iterator>

#include <algorithm>
#include <numeric>
#include <cmath>

#include <iostream>
#include <iomanip>

#include <string>
#include <vector>
#include <unordered_map>

// Helper typedefs
template<class T>
using Vec = std::vector<T>;
template<class K, class V>
using Map = std::unordered_map<K, V>;


// Simple struct representing a single example
struct Example {
    char classification;
    Vec<char> attributes;

    Example(const std::string& line) : classification{ line[0] } {
        size_t idx = 2;
        while (idx < line.size()) {
            attributes.push_back(line[idx]);
            idx += 2;
        }
    }
};

// Enumeration specifying the different error metrics
enum class ErrorMetric {
    ENTROPY,
    PROB_ERROR
};

// Static mapping of the decision numbers to their names
static std::vector<std::string> attr_names {
    "cap-shape",
    "cap-surface",
    "cap-color",
    "bruises",
    "odor",
    "gill-attachment",
    "gill-spacing",
    "gill-size",
    "gill-color",
    "stalk-shape",
    "stalk-root",
    "stalk-surface-above-ring",
    "stalk-surface-below-ring",
    "stalk-color-above-ring",
    "stalk-color-below-ring",
    "veil-type",
    "veil-color",
    "ring-number",
    "ring-type",
    "spore-print-color",
    "population",
    "habitat"
};

// Return the "maximal" classification of the chosen range
std::pair<const char, size_t> maxClass(const Vec<size_t>& idxs, const Vec<Example>& exs) {
    // Record the classification of all examples
    Map<char, size_t> count;
    for (auto i : idxs) count[exs[i].classification]++;

    // Find out which result is the "biggest"
    auto max = std::max_element(std::begin(count), std::end(count),
        [](std::pair<const char, size_t>& p1, std::pair<const char, size_t>& p2) { return p1.second < p2.second; });

    return *max;
}

// Count the number of "wrongly" classified examples assuming a majority decision
double countMinorities(const Vec<size_t>& idxs, const Vec<Example>& exs) {
    Map<char, size_t> count;
    for (auto i : idxs) count[exs[i].classification]++;

    // Find out which result is the "biggest"
    auto max = std::max_element(std::begin(count), std::end(count),
        [](std::pair<const char, size_t>& p1, std::pair<const char, size_t>& p2) { return p1.second < p2.second; });

    return *max;
}

// Count the number of "wrongly" classified examples assuming a majority decision
double countMinorities(const Vec<size_t>& idxs, const Vec<Example>& exs) {
    return idxs.size() - maxClass(idxs, exs).second;
}

// Compute the error number for the examples using the 'entropy' calculation metric
double entropy(const Vec<size_t>& idxs, const Vec<Example>& exs) {
    // Determine the percentage of "correctly" classified examples
    auto pos = maxClass(idxs, exs).second / double{ idxs.size() };

    // Early return to prevent problems due to "log2(0) = -Inf" and IEE754 standard
    if (pos == 1) return 0;
    return idxs.size() * ((-pos * std::log2(pos)) - ((1 - pos) * std::log2(1 - pos))) / exs.size();
}

// Compute the error number for the examples using the 'entropy' calculation metric
double entropy(const Vec<size_t>& idxs, const Vec<Example>& exs) {
    // Determine the percentage of "correctly" classified examples
    auto pos = maxClass(idxs, exs).second / double{ idxs.size() };

    // Early return to prevent problems due to "log2(0) = -Inf" and IEE754 standard
    if (pos == 1) return 0;
    return idxs.size() * ((-pos * std::log2(pos)) - ((1 - pos) * std::log2(1 - pos))) / exs.size();
}

// Compute the error number for the examples, switching on the value for the ErrorMetric 'selector'
double computeError(const Vec<Example>& exs, size_t l, size_t r, ErrorMetric selector) {
    if (r - l == 1) return 0;

    // Determine the maximum classification for the given range
    Map<char, size_t> count;
    for (size_t idx{ l }; idx != r; ++idx)
        count[exs[idx].classification]++;

    auto max = std::max_element(std::begin(count), std::end(count),
        [](std::pair<const char, size_t>& p1, std::pair<const char, size_t>& p2) { return p1.second < p2.second; });

    // If the error is being computed using the "probability of error" metric
    if (selector == ErrorMetric::PROB_ERROR) return (r - l - max->second) / double{ r - l };

    // Otherwise compute using the entropy metric
    auto pos = max->second / double{ r - l };
    if (pos == 1) return 0;
    return (-pos * std::log2(pos)) - ((1 - pos) * std::log2(1 - pos));
}

// Overload for usage in bestDecision
template<class Entry_T>
double computeError(const Vec<Example>& exs, Entry_T& entry, size_t siz, ErrorMetric selector) {
    // Accumulate the error over every value for the current decision
    return std::accumulate(std::begin(entry.second), std::end(entry.second),
        0., [&exs, siz, idx=entry.first, selector](double acc, std::pair<const char, Vec<size_t>>& p) {
            // Perform different accumulations based on the chosen error metric
            if (selector == ErrorMetric::PROB_ERROR)
                return acc + countMinorities(p.second, exs);
            else
                return acc + entropy(p.second, exs);
        }) / (selector == ErrorMetric::PROB_ERROR ? siz : 1);
        // Divide to account for "probability of error" shortcut
}

// Pick the best decision from the examples
size_t bestDecision(const Vec<Example>& exs, Vec<size_t> taken, size_t l, size_t r, ErrorMetric selector, std::string b$    auto num_decs = exs.back().attributes.size();
    auto num_decs = exs.back().attributes.size();

    auto t_begin = std::begin(taken);
    auto t_end = std::end(taken);

    // // Collect the attributes for all decisions for all examples
    Map<size_t, Map<char, Vec<size_t>>> sort_set;
    for (size_t ex{ l }; ex != r; ++ex) {
        for (size_t i{}; i != num_decs; ++i) {
            if (std::find(t_begin, t_end, i) != t_end) continue;

            sort_set[i][exs[ex].attributes[i]].push_back(ex);
        }
    }

    // Find the decision that performs the best (the one with "minimal error")
    size_t bestDec = -1;
    double min_err = std::numeric_limits<double>::max();
    for (auto& entry : sort_set) {
        auto err = computeError(exs, entry, r-l, selector);

        std::cout << buf << " error: " << std::fixed << std::setw(6) << std::setprecision(4)
                  << err << " for " << entry.first << '(' << attr_names[entry.first] << ")\n";

        if (err < min_err) {
            min_err = err;
            bestDec = entry.first;
        }
    }

    std::cout << buf << "Decision for indices [" << l << ',' << r << "): "
              << attr_names[bestDec] << '(' << bestDec << "), error=" << min_err << '\n';

    return bestDec;
}

// Partition the example vector on the chosen decision
Vec<size_t> partition(Vec<Example>& exs, size_t dec, size_t l, size_t r) {
    // Collect the values for the current decision (determines number of needed partitions)
    Map<char, size_t> attr_vals;
    for (size_t ex{ l }; ex != r; ++ex)
        attr_vals[exs[ex].attributes[dec]] = true;

    const auto begin = std::begin(exs);
    const auto end = begin + r;
    auto iter = begin + l;

    Vec<size_t> ret;
    for (const auto& pair : attr_vals) {
        // Move all examples with this value to the front of the current vector range
        // Marks the rest of the vector as the range for the next iteration
        iter = std::partition(iter, end,
            [dec, val=pair.first](const Example& e) { return e.attributes[dec] == val; });

        // Store the index after the end of the partition
        ret.emplace_back(iter - begin);
    }

    return ret;
}

// Tree class to organize the decision tree
class DecTree {
    Vec<DecTree> nodes;
    const size_t l, r;
    size_t decision = -1;

    public:
        // Helper constructor for initial construction
        DecTree(Vec<Example>& arr, ErrorMetric selector)
            : DecTree{ arr, {}, 0, arr.size(), computeError(arr, 0, arr.size(), selector), selector, "" } {}

        // Constructor for the tree structure
        DecTree(Vec<Example>& arr, Vec<size_t> taken, size_t l, size_t r, double err, ErrorMetric selector, std::string$                : l{ l }, r{ r } {

            // Stop if no more decisions can/need to be made
            if (err == 0 || taken.size() == arr.back().attributes.size()) return;
            std::cout << buf << "Calculating decision for indices ["
                      << l << ',' << r << "), current error = " << err << '\n';
            buf += ' ';

            // Find the best decision to take
            decision = bestDecision(arr, taken, l, r, selector, buf);
            taken.emplace_back(decision);

            // Take it and construct sub-nodes from the possible values
            buf += ' ';
            auto ends = partition(arr, decision, l, r);
            for (auto r : ends) {
                nodes.emplace_back(arr, taken, l, r, computeError(arr, l, r, selector), selector, buf);
                l = r;
            }
        }

        // Prints the created decision tree
        template <class Ostream>
        Ostream& print(Ostream& s, const Vec<Example>& arr, std::string buf="") {
            if (!l && r == arr.size()) s << "\nInitial:";
            if (decision == -1) return s << "Decided " << arr[l].classification << '\n';

            s << attr_names[decision] << '\n';
            buf += ' ';
            for (auto& dec : nodes) {
                s << buf << arr[dec.l].attributes[decision] << ':';
                dec.print(s, arr, buf);
            }
            return s;
        }

};


// Read in all examples from the file
Vec<Example> readFile(int argc, const char* argv[]) {
    std::ifstream o;

    // Find the file in the arguments array and open it
    for (int i{1}; i != argc; ++i)
        if (argv[i] != std::string{ "-e" }) {
            o.open(argv[i]);
            break;
        }

    // Read in the examples from the file
    Vec<Example> ret;
    if (o) {
        std::string line;
        while (std::getline(o, line))
            ret.emplace_back(line);
    }

    return ret;
}

// Get the error metric to use (find a '-e' in the arguments array)
ErrorMetric getErrorMetric(int argc, const char* argv[]) {
    for (int i{}; i != argc; ++i)
        if (argv[i] == std::string{"-e"}) return ErrorMetric::ENTROPY;
    return ErrorMetric::PROB_ERROR;
}

// Run the decision tree program
int main(int argc, const char* argv[]) {
    auto examples = readFile(argc, argv);
    if (examples.size() == 0) return (std::cout << "No data read from the given file\n"), 0;
    DecTree{ examples, getErrorMetric(argc, argv) }.print(std::cout, examples);
}
