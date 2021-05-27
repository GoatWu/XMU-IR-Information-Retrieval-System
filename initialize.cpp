#include "myfunc.cpp"

const int MAXFILENAME = 10000;
char buffer[MAXFILENAME];
extern std::vector<std::string> filePaths;            // 编号 -> 文档
extern std::vector<std::string> words;                // 编号 -> 单词
extern std::map<std::string, int> fileNum;            // 文档 -> 编号
extern std::map<std::string, int> wordNum;            // 单词 -> 编号
extern std::set<char> separationChar;                 // 分隔符
extern std::vector<int> df;                           // 出现词项的文档数目
extern std::vector< std::vector<int> > tf;            // 词频矩阵
extern std::vector< std::vector<double> > tf_idf;     // tf-idf 权重
extern std::vector< std::vector<double> > Cos_weight; // 余弦归一化之后的权重

// 获取全部文件的绝对路径
int initFilePaths() {
    getcwd(buffer, sizeof(buffer));
    std::string path;
    path.assign(buffer);
    path += "/datafile/";

    getFiles(path, filePaths);

    for (size_t i = 0; i < filePaths.size(); i++) {
        fileNum[filePaths[i]] = i;
        // std::cout << filePaths[i] << "\n";
    }
    return (int) filePaths.size();
}

void initSeperationChar() {
    std::set<char> tmp{
        ' ', 13, 10, '\t', ',', '.', ';',
        ':', '?', '!', '\"', '\''
    };
    separationChar = tmp;
}

int initWordsNum() {
    int wordCnt = 0;
    for (size_t i = 0; i < filePaths.size(); i++) {
        std::set<std::string> used; // 单次在当前文档中是否出现过
        std::ifstream t(filePaths[i]);  
        std::stringstream buffer;  
        buffer << t.rdbuf();  
        std::string contents(buffer.str());
        std::vector<std::string> w = split(contents, separationChar);
        for (size_t j = 0; j < w.size(); j++) {
            std::string now = toLower(w[j]);
            if (!wordNum.count(now)) {
                wordNum[now] = wordCnt++;
                df.push_back(1);
                used.insert(now);
                words.push_back(now);
            }
            else if (!used.count(now)) {
                df[wordNum[now]]++;
                used.insert(now);
            }
        }
        t.close();
    }
    return wordCnt;
}

void initTf() {
    tf.resize(words.size());
    for (size_t i = 0; i < tf.size(); i++) {
        tf[i].resize(filePaths.size());
    }
    std::ifstream inFile;
    for (size_t i = 0; i < filePaths.size(); i++) {
        std::ifstream t(filePaths[i]);  
        std::stringstream buffer;  
        buffer << t.rdbuf();  
        std::string contents(buffer.str());
        std::vector<std::string> w = split(contents, separationChar);
        for (size_t j = 0; j < w.size(); j++) {
            std::string now = toLower(w[j]);
            tf[wordNum[now]][i]++;
        }
        t.close();
    }
}

void initWeight() {
    int n = filePaths.size();
    int m = words.size();
    tf_idf.resize(m);
    Cos_weight.resize(m);
    for (size_t i = 0; i < m; i++) {
        tf_idf[i].resize(n);
        Cos_weight[i].resize(n);
    }
    for (size_t i = 0; i < m; i++) {
        for (size_t j = 0; j < n; j++) {
            if (tf[i][j] == 0) tf_idf[i][j] = 0;
            else {
                double w_df = 1 + log10((double)tf[i][j]);
                double idf_t = log10(1.0 * n / df[i]);
                tf_idf[i][j] = w_df * idf_t;
            }
        }
    }
    for (size_t j = 0; j < n; j++) {
        double sum = 0, sqrtsum;
        for (size_t i = 0; i < m; i++) {
            sum += tf_idf[i][j] * tf_idf[i][j];
        }
        sqrtsum = sqrt(sum);
        for (size_t i = 0; i < m; i++) {
            Cos_weight[i][j] = tf_idf[i][j] / sqrtsum;
        }
    }
}
