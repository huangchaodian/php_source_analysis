## funtrace
    运行期间：统计一次请求所有执行的函数以及函数执行次数,
## develop_env
    编译期间：替换编译文件.
``` bash
/home/hucd/php/bin/phpize
./configure --enable-develop_env --with-php-config=/home/hucd/php/bin/php-config
make && make install
echo 'extension=/home/hucd/php/lib/php/extensions/no-debug-non-zts-20121212/develop_env.so' >>/home/hucd/php/etc/php.ini
/home/hucd/php/bin/php -m
```
