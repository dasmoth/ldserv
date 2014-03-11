#include <cstdlib>
#include <cmath>
#include <string>
#include <iostream>
#include <iomanip>
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

string mixColor(int r1, int g1, int b1, int r2, int g2, int b2, double fraction) {
    unsigned int rX = (unsigned int) ((fraction * r2) + ((1.0 - fraction) * r1));
    unsigned int gX = (unsigned int) ((fraction * g2) + ((1.0 - fraction) * g1));
    unsigned int bX = (unsigned int) ((fraction * b2) + ((1.0 - fraction) * b1));

    stringstream colstr;
    colstr << noshowbase << setfill('0') << hex << setw(2) << rX << setw(2) << gX << setw(2) << bX;
    return colstr.str();
}

vector<string> paletteRed {
    "808080",
    mixColor(255, 255, 255, 255, 0, 0, 0.25),
    mixColor(255, 255, 255, 255, 0, 0, 0.5),
    mixColor(255, 255, 255, 255, 0, 0, 0.75),
    mixColor(255, 255, 255, 255, 0, 0, 1.0),
};

vector<string> paletteBlue {
    "808080",
    mixColor(255, 255, 255, 0, 0, 255, 0.25),
    mixColor(255, 255, 255, 0, 0, 255, 0.5),
    mixColor(255, 255, 255, 0, 0, 255, 0.75),
    mixColor(255, 255, 255, 0, 0, 255, 1.0),
};

vector<string> paletteGreen {
    "808080",
    mixColor(255, 255, 255, 0, 255, 0, 0.25),
    mixColor(255, 255, 255, 0, 255, 0, 0.5),
    mixColor(255, 255, 255, 0, 255, 0, 0.75),
    mixColor(255, 255, 255, 0, 255, 0, 1.0),
};

vector<string> paletteOrange {
    "808080",
    mixColor(255, 255, 255, 240, 180, 0, 0.25),
    mixColor(255, 255, 255, 240, 180, 0, 0.5),
    mixColor(255, 255, 255, 240, 180, 0, 0.75),
    mixColor(255, 255, 255, 240, 180, 0, 1.0)
};

vector<string> palettePurple {
    "808080",
    mixColor(255, 255, 255, 180, 0, 230, 0.25),
    mixColor(255, 255, 255, 180, 0, 230, 0.5),
    mixColor(255, 255, 255, 180, 0, 230, 0.75),
    mixColor(255, 255, 255, 180, 0, 230, 1.0)
};

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
    vector<string> refs;
    bool color = false;

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
            refs.push_back(value);
        } else if (key == "color") {
            color = true;
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
    vector<var> refVars(refs.size());

    if (tabix->setRegion(region)) {
        string line;
        
        while (tabix->getNextLine(line)) {
            vector<string> toks = split_on(line, '\t');
            var v;
            v.id = toks[2];
            v.pos = stoi(toks[1]);
            v.genotypes = toks[3];
            vars.push_back(v);

            for (int x = 0; x < refs.size(); ++x) {
                if (refs[x] == v.id) {
                    refVars[x] = v;
                    break;
                }
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

        double maxLD = 0;
        int maxRef = -1;
        for (int r = 0; r < refVars.size(); ++r) {
            if (refVars[r].genotypes.length() > 0) {
                double tld = ld(refVars[r].genotypes, v.genotypes);
                cout << ",\"score" << (r+2) << "\":" << tld;
                if (tld > maxLD) {
                    maxLD = tld;
                    maxRef = r;
                }
            }
        }
        if (color && maxRef >= 0) {
            vector<string> palette = paletteGreen;

            if (maxRef == 0) {
                palette = paletteRed;
            } else if (maxRef == 1) {
                palette = paletteBlue;
            } else if (maxRef == 2) {
                palette = paletteOrange;
            } else if (maxRef == 3) {
                palette = palettePurple;
            } else {
                palette = paletteGreen;
            }

            int paletteSize = palette.size();
            int colIndex = (int) (maxLD * paletteSize);
            if (colIndex < 0) colIndex = 0;
            if (colIndex >= paletteSize) colIndex = paletteSize  - 1;
            string col = palette[colIndex];
            cout << ",\"color\": \"#" << col << "\"";
        }
        cout << "}";
    }
    cout << "]" <<endl;
}
