<?php
if (version_compare(PHP_VERSION, 7, '<'))
    die("PHP must later than version 7.0\n");
if (php_sapi_name() !== 'cli')
    die("Must run in cli mode\n");
if (!extension_loaded('php_dyjcrypt'))
    if (!dl('php_dyjcrypt.'.PHP_SHLIB_SUFFIX)) {
        die("The extension: 'php_dyjcrypt.".PHP_SHLIB_SUFFIX."' not loaded\n");
    }
if ($argc <= 1)
    die("\nusage: php dyjdecrypt.php file.php ...     encrypt the php file(s) or directory(s)\n\n");



array_shift($argv);
foreach ($argv as $fileName) {
    if (is_file($fileName)) {
        handle($fileName);
    } elseif (is_dir($fileName)) {
        $DirectoriesIt = new RecursiveDirectoryIterator($fileName, FilesystemIterator::SKIP_DOTS);
        $AllIt         = new RecursiveIteratorIterator($DirectoriesIt);
        $it            = new RegexIterator($AllIt, '/^.+\.php$/i', RecursiveRegexIterator::GET_MATCH);
        foreach ($it as $v)
            handle($v[0]);
    } else {
        echo "Unknowing file: '$fileName'\n";
    }
}


function handle($file)
{
    echo 'start decode '.$file."\r\n";
    $fp = fopen($file, 'rb+');
    $fileSize = filesize($file);
    if ($fp && $fileSize > 0 ) {

        $data = dyjdecode(fread($fp, $fileSize));

        if ($data !== false && !empty($data)) {
            if (file_put_contents($file, '') !== false) {
                rewind($fp);
                fwrite($fp, $data);
                echo 'decode '.$file . ' success '."\r\n";
            }
        }else{
            echo 'decode '.$file . ' fail'."\r\n";
        }
        fclose($fp);
    }else{
        echo 'open file '.$file .' fail'."\r\n";
    }
}

