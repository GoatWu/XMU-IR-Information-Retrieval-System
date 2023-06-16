# XMU-IR-Information-Retrieval-System

<big><strong>Author：GoatWu</strong></big>

2020-2021春季XMU信息检索大作业：自适应文本检索系统的实现。

此次作业完成了任务：“每一次检索后对返回的文档执行相关反馈的判断，重新生成查询”。项目全部使用C++语言，并且独自从零开始编写。代码总量为18KB、530行。由于文本检索需要服务器与客户端，此项目使用socket编程进行通信。

完整文档见pdf文件。

## 1. 文件架构介绍

1. `initialize.cpp`
    用于初始化服务器，即构造向量空间模型。这里包括：
    
      - 获取全部文档的绝对路径，并将文档与一个数字编号一一映射；
      - 读取全部文档，并将所有单词与一个数字编号一一映射；
      - 构造词频矩阵$\text{tf}_{t,d}$；
      - 构造文档频率向量`df`；
      - 构造`tf-idf`权重矩阵，并且进行余弦归一化；
2. `myfunc.cpp`
    用于提供各种函数支持，并且定义全局变量（如：词频矩阵$tf_{t,d}$、文档频率向量`df`等）。各函数的功能将在下文详细介绍。
    
2. `server.cpp`
   
    此文件是服务器代码。首先的工作是初始化服务器，这里用到了`initialize.cpp`中的各个函数；然后是建立socket服务，绑定服务器管理员指定的端口后监听此端口。当有客户端进程来connect的时候，主进程会fork一个子进程与其通信，以满足多用户同时查询；每次用户查询结束之后，服务器会给客户端提供3个选项：
    
    - 提供相关反馈信息，以取得更为精确的查询结果；
    - 不提供相关反馈信息，继续新的查询；
    - 退出查询。
    
3. `client.cpp`
	此文件是客户端代码。客户端负责向服务器发送查询、接受信息，直到客户端用户输入`bye()`或者按下`control+C`强制退出。

## 2. 实验

### 2.1 运行

编译时，由于代码使用了 lambda 函数（匿名函数）等 C++11 特性，需加入编译选项 `-std=c++11`：

```bash
g++ server.cpp -std=c++11 server
g++ client.cpp -std=c++11 client
```

运行时，打开两个终端窗口，代表服务端与客户端：

![2](https://goatwu.oss-cn-beijing.aliyuncs.com/img/2.png)

![3](https://goatwu.oss-cn-beijing.aliyuncs.com/img/3.png)

### 2.2 查询与服务

1. 客户端输入查询`game`

   如图所示，客户端输入查询 `game`（比赛）后，服务器返回了一系列相关文档，其中7 项是关于 NBA、2 项是关于 football、1 项是关于 nokia 的，直观上来看都和 game 有所关联。尤其是 NBA，因为 NBA 球赛的英文是"basketball game"。

   ![5](https://goatwu.oss-cn-beijing.aliyuncs.com/img/5.png)

2. 客户端进行相关反馈查询
   假设用户想要查找关于 NBA 的信息，于是输入`1`进行相关反馈查询。紧接着客户端输入相关的标号`1,3,5,7,8,9,10`。如图可以看见，服务器新返回的相关反馈查询结果全部 10 条记录都是关于 NBA 的：

   ![6](https://goatwu.oss-cn-beijing.aliyuncs.com/img/6.png)

3. 客户端输入查询 `i like to play soccer`
   如图所示，客户端输入查询 `i like to play soccer` 后，服务器返回的结果中，前 5 项有 4 项是关于足球的。可见此查询系统也可以查询句子，而不仅仅是单词。客户端输入`2` 之后，可以继续查询。

   ![7](https://goatwu.oss-cn-beijing.aliyuncs.com/img/7.png)

4. 客户端输入 `bye()` 退出查询
   如图所示，客户端输入 `bye()`退出查询。而后客户端可以再次连接到服务器，也即服务器不受影响。

   ![8](https://goatwu.oss-cn-beijing.aliyuncs.com/img/8.png)

