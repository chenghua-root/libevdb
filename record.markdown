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

## protobuf-c
```
protoc --c_out=. s3_packet_header.proto
```
同一结构体的protobuf编码长度不固定，不适合做网络packet协议，适合做RPC协议
