NAME=miHoYolink
CC = clang

# 编译选项
CFLAGS = -O3 -g -Wall -Werror

version=v4
versionCode=4

all: module/GetmiHoYolink
	echo "id=miHoYolink\nname=米哈游全家桶抽卡记录查询\nversion=$(version)\nversionCode=$(versionCode)\nauthor=黑弩\ndescription=打开抽卡记录时自动跳转到浏览器打开" > module/module.prop
	cd module && zip -r ../$(NAME)-$(version).zip *

module/GetmiHoYolink: GetmiHoYolink.c
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf module/GetmiHoYolink
	rm -rf module/module.prop
	rm -rf $(NAME)-$(version).zip
