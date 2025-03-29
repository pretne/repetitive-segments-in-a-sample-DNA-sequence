#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <fstream>
#include <algorithm>
#include <tuple>
using namespace std;

// 生成互补碱基
char get_complement(char base) {
    switch (base) {
        case 'A': return 'T';
        case 'T': return 'A';
        case 'C': return 'G';
        case 'G': return 'C';
        default: return base;
    }
}

// 生成逆转互补序列
string generate_reverse_complement(const string& seq) {
    string rev_complement;
    for (int i = seq.size() - 1; i >= 0; --i) {
        rev_complement += get_complement(seq[i]);
    }
    return rev_complement;
}

// 构建参考序列的哈希表
unordered_map<string, pair<int, bool>> create_hash_table(const string& ref_seq) {
    unordered_map<string, pair<int, bool>> hash_map; 
    int len = ref_seq.size();
    
    // 正向子字符串处理
    for (int i = 0; i < len; ++i) {
        string substring;
        for (int j = i; j < len; ++j) {
            substring += ref_seq[j];
            hash_map[substring] = {i, false}; 
        }
    }
    
    // 反向互补子字符串处理
    string rev_comp_seq = generate_reverse_complement(ref_seq);
    for (int i = 0; i < len; ++i) {
        string substring;
        for (int j = i; j < len; ++j) {
            substring += rev_comp_seq[j];
            int sub_len = j - i + 1;
            int start_pos = len - 1 - j;
            if (!hash_map.count(substring)) { 
                hash_map[substring] = {start_pos, true}; 
            }
        }
    }
    
    return hash_map; 
}

// 动态规划求解最优子字符串分段
vector<tuple<int, int, bool>> get_optimal_segments(const string& query, const unordered_map<string, pair<int, bool>>& hash_map) { 
    int query_len = query.size();
    vector<int> dp(query_len + 1, query_len + 1);
    dp[query_len] = 0;
    vector<pair<int, pair<int, bool>>> trace(query_len + 1, {-1, {-1, false}});

    unordered_map<string, int> seg_count;

    for (int i = query_len - 1; i >= 0; --i) {
        string substring;
        int min_segments = -1;
        pair<int, pair<int, bool>> best_trace = {-1, {-1, false}}; 
        for (int j = i; j < query_len; ++j) {
            substring += query[j];
            if (hash_map.find(substring) != hash_map.end()) { 
                int next_pos = j + 1;
                int segments = 1 + dp[next_pos];
                auto [start_pos, is_rev] = hash_map.at(substring);
                int sub_len = next_pos - i;
                string seg_key = to_string(start_pos) + "_" + to_string(start_pos + sub_len - 1);

                if (segments < min_segments || min_segments == -1) {
                    min_segments = segments;
                    best_trace = {next_pos, {start_pos, is_rev}};
                } else if (segments == min_segments) {
                    string best_key = to_string(best_trace.second.first) + "_" 
                                   + to_string(best_trace.second.first + (next_pos - i) - 1);
                    if (seg_count[seg_key] > seg_count[best_key]) {
                        best_trace = {next_pos, {start_pos, is_rev}};
                    }
                }
            }
        }
        if (min_segments != -1) {
            dp[i] = min_segments;
            trace[i] = best_trace;
            string seg_key = to_string(best_trace.second.first) + "_" 
                               + to_string(best_trace.second.first + (best_trace.first - i) - 1);
            seg_count[seg_key]++;
        }
    }

    // 根据动态规划路径重建区间
    vector<tuple<int, int, bool>> segments;
    int pos = 0;
    while (pos < query_len) {
        auto [next_pos, info] = trace[pos];
        int start_pos = info.first;
        bool is_rev = info.second;
        int sub_len = next_pos - pos;
        segments.emplace_back(start_pos + 1, start_pos + sub_len, is_rev);
        pos = next_pos;
    }
    return segments;
}

// 计算重复的区间
vector<tuple<int, int, int, bool>> find_repeated_segments(const vector<tuple<int, int, bool>>& segments) {
    unordered_map<string, int> count_map;
    for (const auto& seg : segments) {
        string key = to_string(get<0>(seg)) + "_" + to_string(get<1>(seg)) + "_" 
                    + (get<2>(seg) ? "1" : "0");
        count_map[key]++;
    }

    vector<tuple<int, int, int, bool>> result;
    unordered_map<string, tuple<int, int, int, bool>> stats;

    for (const auto& small : segments) {
        bool is_contained = false;
        for (const auto& large : segments) {
            if (get<0>(large) <= get<0>(small) && get<1>(small) <= get<1>(large) && 
                !(get<0>(large) == get<0>(small) && get<1>(large) == get<1>(small) && get<2>(large) == get<2>(small))) {
                is_contained = true;
                break;
            }
        }

        if (is_contained) {
            int len = get<1>(small) - get<0>(small) + 1;
            string key = to_string(get<0>(small)) + "_" + to_string(get<1>(small)) + "_" 
                        + (get<2>(small) ? "1" : "0");
            int count = count_map[key];
            string stat_key = to_string(get<1>(small)) + "_" + to_string(len) + "_" 
                            + (get<2>(small) ? "1" : "0");
            if (!stats.count(stat_key)) {
                stats[stat_key] = {get<1>(small), len, count, get<2>(small)};
            }
        }
    }

    for (const auto& res : stats) {
        result.push_back(res.second);
    }

    // 排序结果
    sort(result.begin(), result.end(), [](const tuple<int, int, int, bool>& a, const tuple<int, int, int, bool>& b) {
        if (get<0>(a) != get<0>(b)) return get<0>(a) < get<0>(b);
        if (get<1>(a) != get<1>(b)) return get<1>(a) < get<1>(b);
        return get<3>(a) < get<3>(b);
    });

    return result;
}

// 输出重复区间的统计结果
void output_results(const vector<tuple<int, int, int, bool>>& result) {
    for (const auto& res : result) {
        cout << "Position in Reference: " << get<0>(res)
             << ", Repeat Length: " << get<1>(res)
             << ", Repeat Count: " << get<2>(res)
             << ", Reverse: " << (get<3>(res) ? "Yes" : "No") << endl;
    }
}

int main() {
    string ref_seq, query_seq;
    ifstream file("test_1.txt");
    
    if (!file) {
        cerr << "Error: Unable to open file test_1.txt" << endl;
        return 1;
    }
    
    getline(file, ref_seq);
    getline(file, query_seq);
    file.close();
    
    auto hash_map = create_hash_table(ref_seq);
    vector<tuple<int, int, bool>> segments = get_optimal_segments(query_seq, hash_map); 
    vector<tuple<int, int, int, bool>> repeated_segments = find_repeated_segments(segments);
    output_results(repeated_segments);
    
    return 0;
}
