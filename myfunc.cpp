#include <bits/stdc++.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
typedef std::pair<int, double> pidb;
const double alpha = 1.0;
const double beta = 0.75;
const double gama = 0.15;
std::vector<std::string> filePaths;               // 编号 -> 文档
std::vector<std::string> words;                   // 编号 -> 单词
std::map<std::string, int> fileNum;               // 文档 -> 编号
std::map<std::string, int> wordNum;               // 单词 -> 编号
std::set<char> separationChar;                    // 分隔符
std::vector<int> df;                              // 出现词项的文档数目
std::vector< std::vector<int> > tf;               // 词频矩阵
std::vector< std::vector<double> > tf_idf;        // tf-idf 权重     
std::vector< std::vector<double> > Cos_weight;    // 余弦归一化之后的权重 

bool isnumber(std::string s) {
    int len = (int)s.length();
    for (int i = 1; i < len; i++) {
        if (!isdigit(s[i])) {
            return false;
        } 
    }
    return true;
}

void getFiles(std::string path, std::vector<std::string>& files) {
    std::string path0 = path;
    DIR* pDir;
    struct dirent* ptr;
    struct stat s;
    lstat(path.c_str(), &s);
 
    if (!S_ISDIR(s.st_mode) || !(pDir = opendir(path.c_str()))) {
        std::cout << "not a valid directory: " << path << "\n";
        return;
    }

    std::string subFile;
    while ((ptr = readdir(pDir)) != 0) {
        subFile = ptr->d_name;
        if (subFile == "." || subFile == "..") continue;
        struct stat tmp;
        subFile = path0 + subFile + "/";
        lstat(subFile.c_str(), &tmp);
        if (S_ISDIR(tmp.st_mode)) {
            getFiles(subFile, files);
        }
        else {
            subFile.pop_back();
            files.push_back(subFile);
        }   
    }
    closedir(pDir);
}


std::string toLower(std::string s) {
    std::string res = s;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] >= 'A' && s[i] <= 'Z') {
            res[i] += 'a' - 'A';
        }
    }
    return res;
}


std::vector<std::string> split(const std::string &str, const std::set<char> &pattern) {
    // const char* convert to char*
    char * strc = new char[strlen(str.c_str())+1];
    strcpy(strc, str.c_str());
    size_t len = strlen(strc);
    std::vector<std::string> resultVec;

    char *tmpStr = strc;
    for (size_t i = 0; i < len; i++) {
        if (pattern.count(strc[i])) {
            strc[i] = 0;
            if (tmpStr[0] != 0) {
                resultVec.push_back(std::string(tmpStr));
            }
            tmpStr = strc + i + 1;
        }
    }
    if (tmpStr[0] != 0) {
        resultVec.push_back(std::string(tmpStr));
    }
    delete[] strc;
    return resultVec;
}

void normalize(std::vector<double> &res) {
    double sum = 0, sqrtsum;
    for (size_t i = 0; i < res.size(); i++) {
        sum += res[i] * res[i];
    }
    sqrtsum = sqrt(sum);
    for (size_t i = 0; i < res.size(); i++) {
        res[i] /= sqrtsum;
    }
}

std::vector<double> getWeight(std::string q) {
    std::vector<std::string> w = split(q, separationChar);
    std::vector<double> res;
    std::vector<int> cnt(tf.size());
    for (size_t i = 0; i < w.size(); i++) {
        std::string now = toLower(w[i]);
        if (!wordNum.count(now)) continue;
        int id = wordNum[now];
        cnt[id]++;
    }
    for (size_t i = 0; i < cnt.size(); i++) {
        if (!cnt[i]) res.push_back(0);
        else {
            double w_df = 1 + log10((double)cnt[i]);
            double idf_t = log10(1.0 * tf[0].size() / df[i]);
            res.push_back(w_df * idf_t);
        }
    }
    // 归一化
    normalize(res);
    return res;
}

std::vector<int> query(std::vector<double> qryWeight, size_t retCnt=10) {
    std::vector<pidb> dist(tf[0].size());
    std::vector<double> docWeight(qryWeight.size());
    for (size_t i = 0; i < dist.size(); i++) {
        // 计算归一化距离
        double vecDis = 0;
        for (size_t j = 0; j < docWeight.size(); j++) {
            vecDis += Cos_weight[j][i] * qryWeight[j];
        }
        dist[i] = std::make_pair(i, vecDis);
    }
    sort(dist.begin(), dist.end(), [&](pidb a, pidb b) {
        return a.second > b.second;
    });
    std::vector<int> res;
    for (size_t i = 0; i < std::min(retCnt, dist.size()); i++) {
        res.push_back(dist[i].first);
    }
    return res;
}
// 返回前 retCnt 个结果
std::vector<int> query(std::string q, size_t retCnt=10) {
    std::vector<double> qryWeight = getWeight(q);
    return query(qryWeight, retCnt);
}

std::string getResultStr(std::vector<int> &answer) {
    std::string sendstr;
    for (size_t i = 0; i < answer.size(); i++) {
        std::set<char> u{'/'};
        std::vector<std::string> tmp = split(filePaths[answer[i]], u);
        sendstr += std::to_string(i + 1) + ". "; 
        sendstr += "/" + tmp[tmp.size() - 3];
        sendstr += "/" + tmp[tmp.size() - 2];
        sendstr += "/" + tmp[tmp.size() - 1] + "\n";
    }
    return sendstr + "\n";
}

std::vector<int> feedback(std::string q, 
                          std::vector<int> relevant,
                          std::vector<int> irrelevant) {
    std::vector<double> qryWeight = getWeight(q);
    std::vector<std::string> w = split(q, separationChar);
    std::vector<double> res;
    std::vector<int> q0(tf.size());
    std::vector<double> qm(tf.size());
    std::vector<double> muDr(tf.size());
    std::vector<double> muDnr(tf.size());
    for (size_t i = 0; i < w.size(); i++) {
        std::string now = toLower(w[i]);
        if (!wordNum.count(now)) continue;
        int id = wordNum[now];
        q0[id]++;
    }
    for (size_t i = 0; i < tf.size(); i++) {
        for (size_t j = 0; j < relevant.size(); j++) {
            int docID = relevant[j];
            muDr[i] += tf[i][docID];
        }
        muDr[i] /= 1.0 * relevant.size();
        for (size_t j = 0; j < irrelevant.size(); j++) {
            int docID = irrelevant[j];
            muDnr[i] += tf[i][docID];
        }
        muDnr[i] /= 1.0 * irrelevant.size();
    }
    for (size_t i = 0; i < tf.size(); i++) {
        qm[i] = alpha * q0[i] + beta * muDr[i] - gama * muDnr[i];
        qm[i] = std::max(qm[i], 0.0);
    }
    normalize(qm);
    return query(qm);
}
