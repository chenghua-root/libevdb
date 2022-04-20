## 概述

本项目由C语言编写，使用cmake编译。

基于libev构建了一个网络服务框架，提供类似RPC的功能。基于事件的网络IO，事件触发后读取请求报文，解码后进行任务处理，完成后返回结果。


## 编译

若client和server运行在不同的节点上，需要修改IP配置，client: s3\_main\_client.c, server: s3\_conf\_define.h

安装依赖如libev, protobuf-c等

然后执行./autobuild.sh [release/unittest/coverage]

```
./autobuild.sh          增量编译，包含debug信息，用于开发或调试
./autobuild.sh release  全量编译，不包含debug信息，用于创建运行包
./autobuild.sh unittest 全量编译，包含debug信息，用于单测
./autobuild.sh coverage 全量编译，包含coverage信息，用于查看单测覆盖率
```


## 执行

启动server: ./bin/libevkv

启动client: ./bin/kvclient


## 架构

### 线程

1个主线程，1个监听线程，多个IO线程，多个worker线程。

主线程:

    初始化系统后sleep，等待系统退出。

监听线程:

    包含一个libev loop(epool实例)，注册了listen fd，用于accept新连接。

IO线程:

    每个IO线程包含一个libev loop，注册了一个管道(pipe[0])，用于接收监听线程分发的新连接，接收到分发的新连接后，把新连接注册到本线程的libev loop中。

    每个IO线程还注册了一个异步事件(thread watcher)，用于任务完成后被唤醒执行send操作（返回结果）。

worker线程:

    等待任务分配，收到任务后执行。执行完成后挂载到对应的IO线程，并唤醒IO线程(通过thread watcher)

### IO事件

IO线程中同一个fd的读写事件分开注册，均为水平触发模型。读事件一直开启，写事件关闭，写事件会根据需要打开。


### IO流程

连接到达：

    监听线程accept新的连接，新连接的fd根据负载均衡(如遍历)分发给IO线程，并注册到IO线程的libev loop中。

请求到达：

    当注册的连接中有读事件触发时(请求报文)时，读取报文，解码封装request结构，并把request交给worker线程。

处理请求：

    worker线程接收到request后根据请求code把任务提交给对应的任务处理函数。

    处理完成后封装返回packet，存放在request->out_packet，并把request挂到对应的IO线程，并唤醒IO线程。

返回结果：

    IO线程把request递给所属的连接(struct connnection, 简称conn)，conn根据out packet生成send报文即buf(packet header buf, body buf, body crc buf)，并挂载到发送buf链表中, 然后执行发送。

    若send报文未全部发送完成，则打开conn的写事件，等待写事件触发后继续发送，发送完成后关闭写事件。


## 数据模型&资源管理

### 报文格式

packet
```
    |---header---|---body---|--body crc--|
```

header采用内存直接映射的方式进行网络读写； // 不支持跨系统

body采用protobuf-c进行序列化;

crc采用内存直接映射的方式;


### 数据类型
message:

    一个conn包含多个message，一个message包含多个request，每个request包含一个请求packet和一个用于返回此请求结果的packet.

    接收到的数据存放在message中，从message中提取packet(header + body + body crc)封装request挂到此message中;

请求packet(in packet):

    请求packet的body数据存储在message中，packet的header和body crc从message中拷贝。

    释放请求packet时，由于packet body并不owner数据，执行destructor并不会真正free body。而是由message destruct时释放数据.

返回packet(out packet):

    返回时生成header buf和crc buf，把header, body(out_packet->data_buf), crc三个buf挂载到conn->output_buf_list;

    此时一个request除了返回报文未发送外，其life cycle已经完成，且由于返回的数据对其无依赖，可以被释放；

    等output_buf_list被发送完成后就可以释放用于返回的buf数据。

message释放：

    message被串联在conn中，当其上的request全部执行，且没有空余空间用于接收新的请求，则会被释放掉。

报文模型:

```
conn:
  |----------message 0---------|----------message 1---------|--------message ...--------
  |                            |                            |
  |-packet 0-|--..--|-packet m-|-packet n-|--..--|-packet x-|
```

数据结构模型:

```
conn--|--------------------------|---------- ... -----------|
      |                          |                          |
  message 0--|               message 1--|               message n--|
             |                          |
          request: in_packet 0       request: in_packet n
             |                          |
             .                          .
             .                          .
             |                          |
          request: in_packet m       request: in_packet x
```

### 内存使用统计

输出内存分配统计：

    kill -41 `ps -ef | grep bin/libevkv | grep -v grep | awk '{print $2}'`

除了输出内存统计还会调用s3\_io\_print\_stat()输出conn, message, request等统计信息。函数会遍历conn，不是线程安全的，当有conn断开释放时存在coredump的风险。可注释掉s3\_io\_print\_stat()的调用。

进程退出时会输出资源统计，可用于检测是否存在内存泄露：

```
[2022-04-20 16:32:46.606328]======= print mem usage. =======
mod name                         mem_hold  alloc_times  free_times.
S3_MOD_NONE                             0B           0           0.
S3_MOD_BUF                              0B  9145645691  9145645691.
S3_MOD_BUF_DATA                         0B  6859239383  6859239383.
S3_MOD_COND                             0B           0           0.
S3_MOD_QUEUE                            0B           0           0.
S3_MOD_THREADS_QUEUE                    0B           3           3.
S3_MOD_QUEUE_WORKER_ARG                 0B           2           2.
S3_MOD_THREADS_TASK                     0B  2286406308  2286406308.
S3_MOD_PACKET                           0B  4574444813  4574444813.
S3_MOD_REQUEST                          0B  2286406308  2286406308.
S3_MOD_MESSAGE                          0B       20459       20459.
S3_MOD_CONNECTION                       0B           4           4.
S3_MOD_S3IO                             0B           1           1.
```

内存使用统计是一个非常重要的用于排查问题，保证程序正确的手段。

保证每一个申请的资源在使用完成后或在程序退出前都要被回收。如此，当发现存在新的资源未回收的现象时，能根据泄露的资源类别快速定位问题或发现代码中潜在的bug.


## 组件&依赖

libev, cmake, log, unit test, 单测覆盖，指针检测，内存检查, gdb等参考文档record.md
