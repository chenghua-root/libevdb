# 源文件
set(DIR_THIRD_LOGC_SRCS
	log.c
)

# 生成链接库
add_library(logc STATIC ${DIR_THIRD_LOGC_SRCS})
