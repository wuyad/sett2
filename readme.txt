<<<编译依赖<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

本工程需要以下的库进行编译，所有编译都用静态库，将运行时的依赖减少到最小。
1. ACE 5.6
2. boost 1.34.1
3. otl
4. instantclient，oracle sdk with oci support
5. wuya

为了将不同环境下的编译改动降至最小，所有库以下面的目录形式存放：
[LIBS_ROOT]
├─ACE_wrappers
│  ├─ace
│  ├─include
│  │  └─makeinclude
│  └─lib
├─boost
│  ├─include
│  │  └─boost
│  │      ├─algorithm
│  │      ├─archive
│  │      ├─......
│  │      └─xpressive
│  └─lib
├─instantclient
│  ├─include
│  └─lib
├─otl
└─wuya
    └─include
        └─wuya

<<<VC2005编译<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
1. 分别配置依赖库的include目录与lib库目录。
2. 打开工程，src\sett2_vc8.sln
3. build工程
4. 执行

<<<Unix/Linux编译<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
1. 定义LIBS_ROOT环境变量
2. cd src
   make // 正常编译
   make clean // 清理
   make rebuild // 重新编译
   make CC=xxxx // 使用其他编译器进行编译，如aCC，xlC
   make release // 编译发行版
   make backup // 将所有文件备份至bak目录下
3. cd bin
   sett2

<<<运行时<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

运行时，需要oracle client或者instant oracle client支持。


wuya
