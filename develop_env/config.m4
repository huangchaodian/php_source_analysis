PHP_ARG_ENABLE(develop_env, whether to enable develop_env support,
[  --enable-develop_env           Enable develop_env support])

if test "$PHP_DEVELOP_ENV" != "no"; then
  PHP_NEW_EXTENSION(develop_env, develop_env.c, $ext_shared)
fi
