#include <string>
#include <iostream>
#include <vector>

using namespace std;

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

int main(int argc, char **argv) {
    string line;

    cin.sync_with_stdio(false);
    cout.sync_with_stdio(false);
        
    while (getline(cin, line)) {
        if (line.length() == 0 || line[0] == '#')
            continue;

        vector<string> toks = split_on(line, '\t');
        string chr = toks[0];
        int pos = stoi(toks[1]);
        string id = toks[2];
        string genotypes;
        for (int i = 9; i < toks.size(); ++i) {
            genotypes.push_back(toks[i][2]);
        }

        cout << chr << '\t' << pos << '\t' << id << '\t' << genotypes <<endl;
    }   
}
