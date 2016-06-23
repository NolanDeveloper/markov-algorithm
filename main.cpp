#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <exception>
#include <algorithm>
#include <cassert>

using namespace std;

static const string EPS = "_";
static const string ARROW = "->";
static const char * WHITESPACES = " \t";

#define ASSERT_IN_BOUNDS(x, bottom, top) \
    assert((bottom <= (x)) && ((x) < top));

class substitution {
    string _pattern;
    string _result;
    bool _is_final;

public:
    substitution(const string & line);
    bool apply(string & word) const;
    string str() const;
    bool is_final() const;
};

void bad_format(const string & line) {
    ostringstream oss;
    oss << "Can't parse substitution: line \"" << line << "\" has bad format.";
    throw logic_error(oss.str());
}

string trim_substr(const string & line, size_t first, size_t last) {
    ASSERT_IN_BOUNDS(first, 0, line.length());
    ASSERT_IN_BOUNDS(last, 0, line.length() + 1);
    assert(first < last);

    first = line.find_first_not_of(WHITESPACES, first, last - first);
    if (string::npos == first) return "";
    last = line.find_last_not_of(WHITESPACES, last - 1, last - first);
    return line.substr(first, last - first + 1);
}

substitution::substitution(const string & line) : _is_final(false) {
    auto p = line.find(ARROW);
    if (string::npos == p) bad_format(line);
    _pattern = trim_substr(line, 0, p);
    if (_pattern.empty()) bad_format(line);
    p += ARROW.length();
    if (line[p] == '.') {
        _is_final = true;
        ++p;
    }
    _result = trim_substr(line, p, line.length());
    if (_result.empty()) bad_format(line);
}

bool substitution::apply(string & word) const {
    if (EPS == _pattern) {
        word = _result + word;
        return true;
    }
    auto pos = word.find(_pattern);
    if (string::npos == pos) return false;
    if (EPS == _result) {
        word.erase(pos, _pattern.size());
    } else {
        word.replace(pos, _pattern.size(), _result); 
    }
    return true;
}

string substitution::str() const { 
    auto arrow = _is_final ? " ->. " : " -> ";
    return _pattern + arrow + _result; 
}

bool substitution::is_final() const { return _is_final; };

class algorithm {
    vector<substitution> _substitutions;

public:
    algorithm(istream & is);
    const substitution * apply(string & word) const;
    string str() const;
};

algorithm::algorithm(istream & is) {
    string line;
    while (getline(is, line)) {
        if (string::npos == line.find_first_of(WHITESPACES)) 
            continue;
        _substitutions.emplace_back(line);
    }
}

// Retruns false if application was final or there wasn't application and true otherwise.
const substitution * algorithm::apply(string & word) const {
    for (auto & s : _substitutions)
        if (s.apply(word))
            return &s;
    return nullptr;
}

string algorithm::str() const {
    ostringstream oss;
    oss << "╭" << endl;
    size_t n = _substitutions.size();
    for (size_t i = 0; i < n; ++i) {
        auto parenthesis = (i == (n / 2)) ? "┤ " : "│ ";
        oss << parenthesis << _substitutions[i].str() << endl; 
    }
    oss << "╰" << endl;
    return oss.str();
}

int main(int argc, char ** argv) {
    if (argc < 3) {
        cout << "usage: " << argv[0] << " filename.mark trololo" << endl; 
        return 0;
    }
    string file_name = argv[1];
    string word = argv[2];
    ifstream file(argv[1]);
    if (!file.good()) {
        cout << "Can't open file: " << argv[1] << endl;
        return -1;
    }
    algorithm algo(file);
    cout << algo.str() << endl;
    size_t i = 1;
    cout << setw(4) << i << ": " << word << endl;
    const substitution * s;
    while ((s = algo.apply(word))) {
        ++i;
        cout << setw(4) << i << ": " << word << " [" << s->str() << "]" << endl;
        if (s->is_final()) break;
    }
    return 0;
}
