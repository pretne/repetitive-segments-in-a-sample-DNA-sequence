# 寻找样本 DNA 序列中的重复片段

[TOC]

## 1. 算法思路

本项目旨在解决存在结构变异的 DNA 序列比对问题。给定一个参考序列（reference）和一个查询序列（query），目标是将 query 切分为若干非重叠片段，使得每个片段都能与 reference 的某一段（或其反向互补段）高效匹配，并输出所有片段的对应关系。

### 1.1 后缀自动机匹配

- 首先，分别为 reference 及其反向互补序列构建后缀自动机（Suffix Automaton），以支持高效的子串匹配查询。
- 对于 query 的每个起始位置，分别在 reference 和其反向互补序列的自动机中查找能匹配的最长前缀，记录最大匹配长度及是否为反向互补。

### 1.2 贪心切割与重复检测

- 遍历 query，每次优先选择当前位置能匹配的最长片段，并判断该片段是否为反向互补。
- 检查该片段在 query 中是否连续重复出现，统计重复次数。
- 记录每个片段在 reference 中的起始位置、长度、重复次数及是否为反向互补，保证所有片段在 query 中不重叠。

### 1.3 输出切割方案

- 最终输出所有切割片段的对应关系，格式为 (q_st, q_en, r_st, r_en)，便于后续对齐得分计算和可视化分析。

## 2. 主要函数伪代码

```cpp
Function inv(s: string) -> string:
    res ← reverse(s)
    for each character c in res:
        if c == 'A': c ← 'T'
        else if c == 'T': c ← 'A'
        else if c == 'C': c ← 'G'
        else if c == 'G': c ← 'C'
    return res


Function build_suffix_automaton(s: string) -> SuffixAutomaton:
    sam ← new SuffixAutomaton
    for each character c in s:
        sam.extend(c)
    return sam


Method SuffixAutomaton.extend(c: char):
    curr ← new state with length = last.len + 1
    while p != -1 and no transition from state p with char c:
        set transition p→c = curr
        p ← p.link
    if p == -1:
        curr.link ← 0
    else:
        q ← transition from p with char c
        if p.len + 1 == q.len:
            curr.link ← q
        else:
            clone ← copy of q with updated length = p.len + 1
            while p != -1 and transition p→c == q:
                p→c ← clone
                p ← p.link
            q.link ← clone
            curr.link ← clone
    last ← curr


Function find_max_length(sam: SuffixAutomaton, query: string, start: int) -> int:
    curr ← 0
    max_len ← 0
    for i from start to length of query:
        c ← query[i]
        if sam.states[curr] has transition c:
            curr ← sam.states[curr].next[c]
            max_len ← max_len + 1
        else:
            break
    return max_len


Function main():
    reference ← 读取 "reference.txt"
    query ← 读取 "query.txt"
    inv_reference ← inv(reference)

    sam_ref ← build_suffix_automaton(reference)
    sam_inv_ref ← build_suffix_automaton(inv_reference)

    max_lengths ← array of (length, inverted) for each position in query
    for i from 0 to length of query:
        len_ref ← find_max_length(sam_ref, query, i)
        len_inv ← find_max_length(sam_inv_ref, query, i)
        inverted ← (len_inv > len_ref) or (len_inv == len_ref and len_inv > 0)
        max_lengths[i] ← (max(len_ref, len_inv), inverted)

    result ← empty list
    i ← 0
    while i < length of query:
        (dup_len, inverted) ← max_lengths[i]
        if dup_len == 0:
            i ← i + 1
            continue
        substr ← query[i : i + dup_len]
        count ← 1
        j ← i + dup_len
        while j + dup_len ≤ length of query:
            if max_lengths[j].first < dup_len or max_lengths[j].second ≠ inverted:
                break
            if query[j : j + dup_len] ≠ substr:
                break
            count ← count + 1
            j ← j + dup_len
        ref_substr ← inverted ? inv(substr) : substr
        ref_start ← index of ref_substr in reference, or -1 if not found
        append (i, ref_start, dup_len, count, inverted) to result
        i ← j

    将结果按格式写入 "result.txt":
        for each (q_start, r_start, len, _, inverted) in result:
            r_end ← r_start + len if r_start ≠ -1 else -1
            输出 (q_start, q_start + len, r_start, r_end)

```

## 3. 时空复杂度分析

| 步骤/函数                  | 时间复杂度         | 空间复杂度         |
|---------------------------|--------------------|--------------------|
| 构建后缀自动机             | \(O(N)\)           | \(O(N)\)           |
| 查询最长匹配               | \(O(M)\)           | \(O(1)\)           |
| 贪心切割与重复检测         | \(O(M)\)           | \(O(M)\)           |
| 输出切割片段               | \(O(K)\)           | \(O(1)\)           |
| **总复杂度**               | \(O(N + M)\)       | \(O(N + M)\)       |

其中，N 为 reference 长度，M 为 query 长度，K 为切割片段数。

## 4. 运行结果截图

![alt text](image-1.png)

![alt text](image-2.png)
