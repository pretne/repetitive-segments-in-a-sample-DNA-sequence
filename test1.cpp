#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <tuple>

using namespace std;

// DNA 碱基互补配对
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

// 在 query 中查找连续重复的匹配序列，并记录信息
void analyze_repeats(const string &reference, const string &query) {
    vector<tuple<int, int, int, bool>> results;
    int ref_len = reference.size(), query_len = query.size();
    
    vector<int> marked_next(query_len, -1); // 记录每个位置的下一个匹配位置
    for(int len = 2; len <= ref_len; ++len) {
        for(int i = 0; i <= ref_len - len; ++i) {
            string sub_ref = reference.substr(i, len); // 参考序列片段
            string rev_ref = reverse_complement(sub_ref); // 参考序列片段的逆转互补序列
            
            int j = i + len;
            while(j < query_len && marked_next[j] != -1 && marked_next[j] < query_len) { // 跳过已标记的重复序列
                j = marked_next[j];
            }
            if(j + len > query_len) {
                continue; // 超出 query 长度
            }
            string sub = query.substr(j, len); // 查询序列片段
            if(sub != sub_ref && sub != rev_ref) { // 片段不匹配
                continue;
            }
            int repeat_count = 1;

            while(j + len * 2 <= query_len && query.substr(j + len, len) == sub && (ref_len < j + len * 2 || reference.substr(j + len, len) != sub)) {
                repeat_count++;
                j += len;
            }

            if(repeat_count > 1) {
                marked_next[i + len] = j + len;

                if(sub == sub_ref) {
                    results.emplace_back(i + len, len, repeat_count, false);
                } else {
                    results.emplace_back(i + len, len, repeat_count, true);
                }
                i = j - 1;
            }
        }
    }

    //按位置升序排序，如果位置相同，按长度升序排序
    sort(results.begin(), results.end(), [](const auto& a, const auto& b) {
        if (get<0>(a) != get<0>(b)) {
            return get<0>(a) < get<0>(b); // 按 reference 位置升序
        }
        return get<1>(a) < get<1>(b); // 位置相同时，按长度升序
    });

    for(const auto & res : results) {
        cout << "Position: " << get<0>(res)
             << ", Sequence Length: " << get<1>(res)
             << ", Repeat Count: " << get<2>(res)
             << ", Is Reversed: " << (get<3>(res) ? "Yes" : "No") << endl;
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
    
    analyze_repeats(reference, query);
    
    return 0;
}
