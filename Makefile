# NOT USED.

CC = g++

CFLAGS = -Wall -I.

# 项目源文件列表
SRCS = $(wildcard *.cpp)
# 项目头文件列表
HDRS = $(wildcard *.hpp)

# 目标文件列表
OBJS = $(SRCS:.cpp=.o)

# 可执行文件名称
TARGET = my_spandex

# 编译规则
$(TARGET): $(OBJS)
	$(CC)	$(CFLAGS)	$^	-o	$@

# 清理规则
.PHONY: clean
clean:
	rm	-f	*.o
