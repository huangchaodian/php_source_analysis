PHP_ARG_ENABLE(funtrace, whether to enable funtrace support,
[  --enable-funtrace           Enable funtrace support])

if test "$PHP_FUNTRACE" != "no"; then
  PHP_NEW_EXTENSION(funtrace, funtrace.c, $ext_shared)
fi
