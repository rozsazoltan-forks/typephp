<?php

function main(int $argc, array $argv): void
{
    echo "Hello World!";
    var_dump(PHP_VERSION);
    var_dump($argc);
    var_dump($argv);
    var_dump(date('Y-m-d H:i:s', time()));
    var_dump(php_uname());
}
