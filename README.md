# dyjcrypt
php 7.X 以上版本 高级扩展加密库，支持服务端秘钥认证。支持动态秘钥加密。

1.编译环境说明。
win10 64 
vs2015 （为什么使用这玩意儿开发php7.X的扩展，因为本例是用vc14 .所以这个恐怕要找微软。。）
vs 和vc 库对应关系
Visual Studio 6 ： vc6 
Visual Studio 2003 ： vc7 
Visual Studio 2005 ： vc8 
Visual Studio 2008 ： vc9 
Visual Studio 2010 ： vc10 
Visual Studio 2012 ： vc11 
Visual Studio 2013 ： vc12 
Visual Studio 2015 ： vc14

php 环境
vc++14.0 x86 nts
php-7.1.13

2.参考资料
https://blog.csdn.net/xunying6967/article/details/78460068 
https://github.com/lihancong/tonyenc 
https://github.com/liexusong/php-beast 
sdk和deps 下载 https://windows.php.net/downloads/php-sdk/
解决不能运行js文件的问题。 https://blog.csdn.net/ikscher/article/details/7934756
php执行顺序和原理
https://www.cnblogs.com/jingzhishen/p/4328748.html

3.为什么要开发这个加密扩展。
因为普通的混淆加密都能被破解。扩展加密是文件级别的加密，只要守住php扩展文件，不被泄露和破解，php文件几乎没有被破解的可能性。
更可贵的是本例对php文件加密采用秘钥动态加密方式。不同机器机器码产生的秘钥是不同的，所以就保证了php文件加密的安全性。
唯一有可能破解的就是，劫持的zend_compile_file方法，在代码到达zend引擎编译函数之前，完成解密的。 但是要做到这一步，破解者必须要拿到你的扩展，本例扩展采用服务器端秘钥授权方式运行，别人拿到扩展也是运行不了。保证了只有被授权的机器运行扩展才有可能能被破解。所以为了防止这种情况。可以对php源码进行混淆ASCII 加密或者其他加密，进一步保证了安全性。

4.本例解决的技术难题。
根据php7的特性，C++调用了PHP标准库的md5函数。同时，_dyj_getContent使用call_user_function_ex 调用了file_get_contents。完成了接口的访问。不用依赖curl 等第三方的一些库。




