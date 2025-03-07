#include <iostream>
#include <fstream>
#include <unordered_set>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// 生成DNA互补碱基
char complement(char base) {
    switch (base) {
        case 'A': return 'T';
        case 'T': return 'A';
        case 'C': return 'G';
        case 'G': return 'C';
        default: return base;
    }
}

// 生成序列的逆转互补序列
string reverse_complement(const string& seq) {
    string rev_comp;
    for (int i = seq.size() - 1; i >= 0; --i) {
        rev_comp += complement(seq[i]);
    }
    return rev_comp;
}

// 计算query中的连续重复子串，并在reference中查找
void find_continuous_repeated_subsequences(const string& reference, const string& query) {
    vector<tuple<int, int, int, string, int>> results; // 存储结果（位置, 长度, 频次, 是否逆转）
    unordered_set<string> processed_substrings; // 存储已处理的子串
    int q_len = query.size();
    int match_len = 0; // 匹配长度

    while (match_len < reference.size() && match_len < query.size() && reference[match_len] == query[match_len]) {
        match_len++;
    }
    
    // 计算query中的连续重复子串
    for (int len = q_len / 2; len >= 2; --len) { // 子串最短为2
        for (int i = 0; i + len * 2 <= q_len; ++i) {
            string sub = query.substr(i, len);
            if (processed_substrings.find(sub) != processed_substrings.end()) {
                continue; // 跳过已处理的子串
            }
            int repeat_count = 1;
            int j = i + len;
            
            // 计算连续重复次数
            while (j + len <= q_len && query.substr(j, len) == sub) {
                // if(j == 600 && len == 70) {
                //     cout << "sub: " << sub << endl << repeat_count << endl;
                // }
                repeat_count++;
                j += len;
            }

            if (repeat_count > 1) { // 至少出现两次才算重复
                // 在reference中查找
                size_t ref_pos = reference.find(sub);
                string rev_comp = reverse_complement(sub);
                size_t rev_pos = reference.find(rev_comp);
                
                if (ref_pos != string::npos) {
                    if(ref_pos == i) {
                        repeat_count--; // 减去匹配位置之前的重复次数
                        i += len; // 跳过重复子串
                    }
                    results.emplace_back(ref_pos, len, repeat_count, "no", i);
                    i = j - 1;
                } else if (rev_pos != string::npos) {
                    results.emplace_back(rev_pos, len, repeat_count, "yes", i);
                    i = j - 1;
                }
                
                // 标记已处理的子串
                processed_substrings.insert(sub);
                processed_substrings.insert(rev_comp);
            }
        }
    }
    
    // 按位置升序排序，如果位置相同，按长度升序排序
    sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        if (get<0>(a) + get<1>(a) != get<0>(b) + get<1>(b)) {
            return get<0>(a) + get<1>(a) < get<0>(b) + get<1>(b); // 按 reference 位置升序
        }
        return get<1>(a) < get<1>(b); // 位置相同时，按长度升序
    });

    // 输出排序后的结果
    for (const auto& res : results) {
        if (get<0>(res) + get<1>(res) < 400) {
            continue; // 跳过匹配位置之前的结果
        }
        cout << get<0>(res) + get<1>(res) << " " << get<1>(res) << " " << get<2>(res) << " " << get<3>(res) << " " << get<4>(res) << endl;
        for (int i = get<0>(res); i < get<0>(res) + get<1>(res); ++i) {
            cout << reference[i];
        }
        cout << endl;
        for (int i = get<4>(res); i < get<4>(res) + get<1>(res) * get<2>(res); ++i) {
            cout << query[i];
        }
        cout << endl;
    }
}

int main() {
    string reference, query;
    ifstream inputFile("test_1.txt");
    
    if (!inputFile) {
        cerr << "Error: Unable to open file test_1.txt" << endl;
        return 1;
    }
    
    getline(inputFile, reference);
    getline(inputFile, query);
    inputFile.close();
    
    find_continuous_repeated_subsequences(reference, query);
    return 0;
}
