# 源文件
set(DIR_THIRD_CTEST_SRCS
	ctest_buf.c
	ctest_hash.c
	ctest_pool.c
	ctest_string.c
)

# 生成链接库
add_library(ctest STATIC ${DIR_THIRD_CTEST_SRCS})
