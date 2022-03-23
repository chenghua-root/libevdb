## cmake
Install

## libev
Install

### 默认ev\_loop
```
struct ev_loop *loop = EV_DEFAULT;
struct ev_loop *loop = ev_default_loop(0);
```

### ev\_run
若loop未注册任何watcher，则ev\_run(loop, 0)会直接返回ret=0, errno=0

## protobuf-c
```
protoc --c_out=. s3_packet_header.proto
```
同一结构体的protobuf编码长度不固定，不适合做网络包协议
