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

### ev\_break
ev\_break需要在ev watcher callback routine里面调用才起作用

### multi loop
参考：using libev with multiple threads http://lists.schmorp.de/pipermail/libev/2013q1/002094.html
注意：当工作线程run ev loop时，不能在其它线程向其注册watcher, 这样不是线程安全的

## protobuf-c
```
protoc --c_out=. s3_packet.proto
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
sleep()      #include <unistd.h>

## c语法
本地线程静态变量，如: static __thread int64_t tid = -1;
void __attribute__((constructor(priority))) func(); 在main之前调用，可设置priority，priority越小越先执行
void __attribute__((destructor(priority))) func(); 在main()函数退出或者调用了exit()之后调用，可设置priority，priority越小越后执行

## socket
全关闭：close(fd)
半关闭：shutdown(fd, WR)，不可写，可读

close(fd)后本端读写fd报错，errno = 9(EBADF), Bad File Descriptor
close(fd)后对端写数据，errno = 104(ECONNRESET), connection reset by peer

读写失败时，若errno是如下两种，重试即可:
    EINTR: retry at once
    EAGAIN: retry next time

## 信号
输出内存分配统计：kill -41 `ps -ef | grep bin/libevkv | grep -v grep | awk '{print $2}'`

https://man7.org/linux/man-pages/man7/signal.7.html
       Signal        x86/ARM     Alpha/   MIPS   PARISC   Notes
                   most others   SPARC
       ─────────────────────────────────────────────────────────────────
       SIGHUP           1           1       1       1
       SIGINT           2           2       2       2
       SIGQUIT          3           3       3       3
       SIGILL           4           4       4       4
       SIGTRAP          5           5       5       5
       SIGABRT          6           6       6       6
       SIGIOT           6           6       6       6
       SIGBUS           7          10      10      10
       SIGEMT           -           7       7      -
       SIGFPE           8           8       8       8
       SIGKILL          9           9       9       9
       SIGUSR1         10          30      16      16
       SIGSEGV         11          11      11      11
       SIGUSR2         12          31      17      17
       SIGPIPE         13          13      13      13
       SIGALRM         14          14      14      14
       SIGTERM         15          15      15      15
       SIGSTKFLT       16          -       -        7
       SIGCHLD         17          20      18      18
       SIGCLD           -          -       18      -
       SIGCONT         18          19      25      26
       SIGSTOP         19          17      23      24
       SIGTSTP         20          18      24      25
       SIGTTIN         21          21      26      27
       SIGTTOU         22          22      27      28
       SIGURG          23          16      21      29
       SIGXCPU         24          24      30      12
       SIGXFSZ         25          25      31      30
       SIGVTALRM       26          26      28      20
       SIGPROF         27          27      29      21
       SIGWINCH        28          28      20      23
       SIGIO           29          23      22      22
       SIGPOLL                                            Same as SIGIO
       SIGPWR          30         29/-     19      19
       SIGINFO          -         29/-     -       -
       SIGLOST          -         -/29     -       -
       SIGSYS          31          12      12      31
       SIGUNUSED       31          -       -       31

## errno
https://chromium.googlesource.com/chromiumos/docs/+/master/constants/errnos.md
number	hex	symbol	description
1	0x01	EPERM	Operation not permitted
2	0x02	ENOENT	No such file or directory
3	0x03	ESRCH	No such process
4	0x04	EINTR	Interrupted system call
5	0x05	EIO	Input/output error
6	0x06	ENXIO	No such device or address
7	0x07	E2BIG	Argument list too long
8	0x08	ENOEXEC	Exec format error
9	0x09	EBADF	Bad file descriptor
10	0x0a	ECHILD	No child processes
11	0x0b	EAGAIN	Resource temporarily unavailable
11	0x0b	EWOULDBLOCK	(Same value as EAGAIN) Resource temporarily unavailable
12	0x0c	ENOMEM	Cannot allocate memory
13	0x0d	EACCES	Permission denied
14	0x0e	EFAULT	Bad address
15	0x0f	ENOTBLK	Block device required
16	0x10	EBUSY	Device or resource busy
