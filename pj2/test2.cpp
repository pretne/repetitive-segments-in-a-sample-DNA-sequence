#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

using namespace std;

// 反向互补函数
string inv(const string& s) {
    string res = s;
    reverse(res.begin(), res.end());
    for (char& c : res) {
        switch (c) {
            case 'A': c = 'T'; break;
            case 'T': c = 'A'; break;
            case 'C': c = 'G'; break;
            case 'G': c = 'C'; break;
        }
    }
    return res;
}

// 后缀自动机状态
struct State {
    int len = 0;
    int link = -1;
    map<char, int> next;
};

// 后缀自动机构建
class SuffixAutomaton {
public:
    vector<State> states;
    int size, last;

    SuffixAutomaton() {
        states.emplace_back();
        size = 1;
        last = 0;
    }

    void extend(char c) {
        int curr = size++;
        states.emplace_back();
        states[curr].len = states[last].len + 1;
        int p = last;
        while (p != -1 && !states[p].next.count(c)) {
            states[p].next[c] = curr;
            p = states[p].link;
        }
        if (p == -1) {
            states[curr].link = 0;
        } else {
            int q = states[p].next[c];
            if (states[p].len + 1 == states[q].len) {
                states[curr].link = q;
            } else {
                int clone = size++;
                states.emplace_back();
                states[clone].len = states[p].len + 1;
                states[clone].next = states[q].next;
                states[clone].link = states[q].link;
                while (p != -1 && states[p].next[c] == q) {
                    states[p].next[c] = clone;
                    p = states[p].link;
                }
                states[q].link = clone;
                states[curr].link = clone;
            }
        }
        last = curr;
    }
};

SuffixAutomaton build_suffix_automaton(const string& s) {
    SuffixAutomaton sam;
    for (char c : s) {
        sam.extend(c);
    }
    return sam;
}

// 匹配从 query[start] 开始的最长前缀
int find_max_length(const SuffixAutomaton& sam, const string& query, int start) {
    int curr = 0, max_len = 0;
    for (int i = start; i < query.size(); ++i) {
        char c = query[i];
        if (sam.states[curr].next.count(c)) {
            curr = sam.states[curr].next.at(c);
            ++max_len;
        } else {
            break;
        }
    }
    return max_len;
}

int main() {
    ifstream ref_file("input/reference1.txt");
    ifstream qry_file("input/query1.txt");

    if (!ref_file || !qry_file) {
        cerr << "Cannot open input files!" << endl;
        return 1;
    }

    string reference, query;
    getline(ref_file, reference);
    getline(qry_file, query);
    ref_file.close();
    qry_file.close();

    string inv_ref = inv(reference);

    auto sam_ref = build_suffix_automaton(reference);
    auto sam_inv_ref = build_suffix_automaton(inv_ref);

    // 预处理每个位置的最长匹配长度及是否是反向互补
    vector<pair<int, bool>> max_lengths(query.size());
    for (int i = 0; i < query.size(); ++i) {
        int len_ref = find_max_length(sam_ref, query, i);
        int len_inv = find_max_length(sam_inv_ref, query, i);
        bool inverted = (len_inv > len_ref) || (len_inv == len_ref && len_inv > 0);
        max_lengths[i] = {max(len_ref, len_inv), inverted};
    }

    // 查找重复段
    vector<tuple<int, int, int, int, bool>> result;
    int i = 0;
    while (i < query.size()) {
        int dup_len = max_lengths[i].first;
        if (dup_len == 0) {
            ++i;
            continue;
        }
        bool inverted = max_lengths[i].second;
        string substr = query.substr(i, dup_len);
        int count = 1;
        int j = i + dup_len;
        while (j <= query.size() - dup_len) {
            if (max_lengths[j].first < dup_len || max_lengths[j].second != inverted)
                break;
            if (query.substr(j, dup_len) != substr)
                break;
            ++count;
            j += dup_len;
        }
        string ref_substr = inverted ? inv(substr) : substr;
        int ref_start = reference.find(ref_substr);
        result.emplace_back(i, ref_start, dup_len, count, inverted);
        i = j;
    }

    // 仅输出切割信息
    ofstream fout("output/result1.txt");
    if (!fout) {
        cerr << "Cannot open output file!" << endl;
        return 1;
    }

    fout << "[\n";
    for (size_t idx = 0; idx < result.size(); ++idx) {
        auto [q_start, r_start, len, _, inverted] = result[idx];
        int r_end = (r_start != -1) ? (r_start + len) : -1;
        fout << "(" << q_start << "," << (q_start + len) << "," << r_start << "," << r_end << ")";
        if (idx + 1 != result.size()) fout << ",\n";
    }
    fout << "\n]\n";
    fout.close();

    return 0;
}
