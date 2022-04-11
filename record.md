## cmake

### 内存泄露检测
编译时添加-ltcmalloc:
```
target_link_libraries(target -ltcmalloc)
```

获取svg:
```
env HEAPCHECK=normal ./bin-unittest/libevkv_test

pprof ./bin-unittest/libevkv_test "/tmp/libevkv_test.25357._main_-end.heap" --inuse_objects --lines --heapcheck  --edgefraction=1e-10 --nodefraction=1e-10 --svg >> libevkv_mem_leak.svg
```

### 指针检测
如内存重复释放检测

需注释掉-ltcmalloc
```
env MALLOC_CHECK_=3 ./bin-unittest/libevkv_test
```

### 单测覆盖率
```
SET(CMAKE_C_FLAGS_COVERAGE "--coverage") 等于-fprofile-arcs -ftest-coverage
SET(CMAKE_EXE_LINKER_FLAGS "--coverage") 等于-lgcov
```

## libev

### 默认ev\_loop
```
struct ev_loop *loop = EV_DEFAULT;
struct ev_loop *loop = ev_default_loop(0);
```

### ev\_run
若loop未注册任何watcher，则ev\_run(loop, 0)会直接返回ret=0, errno=0

### multi loop
参考：using libev with multiple threads http://lists.schmorp.de/pipermail/libev/2013q1/002094.html
注意：当工作线程run ev loop时，不能在其它线程向其注册watcher, 这样不是线程安全的

## protobuf-c
```
protoc --c_out=. s3_packet_header.proto
```
同一结构体的protobuf编码长度不固定，不适合做网络packet协议，适合做RPC协议

## gdb
输出core文件需执行./autobuild.sh unittest编译
查看core file size: ulimit -c
修改core file size: ulimit -c unlimited

## 第三方库
third/ctest drdr.xp
third/logc  https://github.com/rxi/log.c

### third/logc调整
日志格式把path调整为了file
添加了线程id

## include
    1      User Commands
    2      System Calls
    3      C Library Functions
    4      Devices and Special Files
    5      File Formats and Conventions
    6      Games et. al.
    7      Miscellanea
    8      System Administration tools and Daemons
NULL         #include <stdint.h>
malloc()     #include <stdlib.h>
bzero()      #include <strings.h>
assert()     #include <assert.h>

## c语法
本地线程静态变量，如: static __thread int64_t tid = -1;
void __attribute__((constructor(priority))) func(); 在main之前调用，可设置priority，priority越小越先执行
void __attribute__((destructor(priority))) func(); 在main()函数退出或者调用了exit()之后调用，可设置priority，priority越小越后执行
