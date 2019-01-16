<?php
if (!extension_loaded('php_dyjcrypt'))
    if (!dl('php_dyjcrypt.'.PHP_SHLIB_SUFFIX)) {
        die("The extension: 'php_dyjcrypt.".PHP_SHLIB_SUFFIX."' not loaded\n");
    }


    echo dyjGetSnCode();





?>