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
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig
protoc --c_out=. s3_packet_header.proto
```
