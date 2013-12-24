#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>

#include "tabixpp/tabix.hpp"

using namespace std;

string getEnvStr(string const &k) {
    char *v = getenv(k.c_str());
    return v == NULL ? string("") : string(v);
}

vector<string> split_on(const string& s, const char sep) {
    vector<string> v;
    size_t pos = 0;
    size_t ptr;
    while ((ptr = s.find_first_of(sep, pos)) != string::npos) {
        v.push_back(s.substr(pos, ptr-pos));
        pos = ptr + 1;
    }
    if (pos < s.length())
        v.push_back(s.substr(pos));
    return v;
}

class var {
public:
    string id;
    int pos;
    string genotypes;
};

double ld(const string& a, const string& b) {
    int n = a.length();
    int x11 = 0, x12 = 0, x21 = 0, x22 = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] == '0')
          if (b[i] == '0')
            ++x11;
          else
            ++x12;
        else
          if (b[i] == '0')
            ++x21;
        else
            ++x22;
    }
      
    double p1 = (1.0 *(x11 + x12)) / n;
    double p2 = (1.0 * (x21 + x22)) / n;
    double q1 = (1.0 * (x11 + x21)) / n;
    double q2 = (1.0 * (x12 + x22)) / n;
      
    double D = (1.0 * x11)/n - (p1*q1);
    if (D == 0.0) 
        return D;

    double r2 = D*D / (p1*p2*q1*q2);
    return r2;
}

int main() {

    string query = getEnvStr("QUERY_STRING");
    string fileName = getEnvStr("GENOTYPE_FILE");
    if (fileName == "") {
        cerr << "Missing required environment variable: GENOTYPE_FILE" << endl;
        return 1;
    }

    string chr("");
    int min = -1;
    int max = -1;
    string ref("");

    vector<string> v = split_on(query, '&');
    for (auto i = begin(v); i != end(v); ++i) {
        vector<string> kv = split_on(*i, '=');
        if (kv.size() != 2) {
            cerr << "Bad query string: " << query << endl;
            return 1;
        }

        string key = kv[0];
        string value = kv[1];

        if (key == "chr") {
            chr = value;
        } else if (key == "min") {
            min = stoi(value);
        } else if (key == "max") {
            max = stoi(value);
        } else if (key == "ref") {
            ref = value;
        } else {
            cerr << "Unknown query parameter " << key << endl;
        }
    }

    if (chr == "") {
        cerr << "Missing required parameter 'chr'" << endl;
        return 1;
    }
    if (min < 0) {
        cerr << "Missing required parameter 'min'" << endl;
        return 1;
    }
    if (max < 0) {
        cerr << "Missing required parameter 'max'" << endl;
        return 1;
    }
    if (max < min || (max-min) > 10000000) {
        cerr << "Bad range " << min << ".." << max << endl;
        return 1;
    }

    Tabix *tabix = new Tabix(fileName);
    stringstream regionstr;
    regionstr << chr << ":" << min << "-" << max;
    string region = regionstr.str();

    vector<var> vars;
    var refVar;

    if (tabix->setRegion(region)) {
        string line;
        
        while (tabix->getNextLine(line)) {
            vector<string> toks = split_on(line, '\t');
            var v;
            v.id = toks[2];
            v.pos = stoi(toks[1]);
            v.genotypes = toks[3];
            vars.push_back(v);
            if (v.id == ref) {
                refVar = v;
            }
        }
    }   


    cout.sync_with_stdio(false);
    cout << "Content-type: application/json" << endl << endl;

    cout << "[";
    for (int i  = 0; i < vars.size(); ++i) {
        var& v = vars[i];
        if (i > 0)
            cout << ',';
        cout << "{\"min\":" << v.pos
             << ",\"max\":" << v.pos
             << ",\"id\":\"" << v.id << "\"";
        if (refVar.genotypes.length() > 0) {
            cout << ",\"score\":" << ld(refVar.genotypes, v.genotypes);
        }
        cout << "}";
    }
    cout << "]" <<endl;
}
