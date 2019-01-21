/**
* php crypt code tool
*
* @author:  duyunjiang
* @qq: 1666263536
* @github:https://github.com/xyls518/dyjcrypt
*/
#pragma once


#include "cpu.h"
#include "ext\standard\md5.h"
#include "cJSON\cJSON.h"
#include "cache.h"
#include "beast_log.h"



#define DYJFILE_RES FILE*

typedef unsigned char u_char;
typedef const char c_char;
/**
混淆头，用来识别文件是否加密
**/
const u_char dyj_header[] = {
	0x49, 0x20, 0x6c, 0x6f,
	0x76, 0x65, 0x20, 0x63,
	0x68, 0x69, 0x6e, 0x61,
};

/**
秘钥，用来加密数据
**/
u_char dyj_key[200];
/*const u_char dyj_key[] = {
	0x41,0x75,0x74,0x68,0x6f,0x72,0x20,0x62,0x79,0x20,0x64,0x75,0x79,0x75,0x6e,0x6a,0x69,
	0x61,0x6e,0x67
};*/



//加密字符串
const char key_secret[] = "20190114";

const char server_url[] = "http://localhost/";

zend_op_array * (*old_compile_file)(zend_file_handle *, int); //函数指针

zend_op_array * new_compile_file(zend_file_handle *, int); //新的编译文件处理

int dyj_do_decode(DYJFILE_RES fp, struct stat *stat_buf,DYJFILE_RES *res, const char *file_name);

void _dyj_encode(char *, size_t);

void _dyj_decode(char *, size_t);

void _dyj_md5(char * pstr);

//检查秘钥是否正确
BOOL _dyj_checkSn();

//访问接口
zval _dyj_getContent(zval param);

//取时间
void _dyj_getTime(char * nowtime);



/**
默认解密
**/
int dyj_do_decode(DYJFILE_RES fp, struct stat *stat_buf, DYJFILE_RES *res, const char *file_name)
{

	if (_dyj_checkSn() == FALSE) { //秘钥验证失败，不会解密
		return FALSE;
	}

	char *p_data;
	size_t data_len;
	size_t write_len;

	data_len = stat_buf->st_size - sizeof(dyj_header); //除了混淆头剩余的长度
	p_data = (char *)emalloc(data_len); //申请剩余的长度
	fseek(fp, sizeof(dyj_header), SEEK_SET); //重定位文件的指针,偏移一个头部的距离
	fread(p_data, data_len, 1, fp); //跳过头部读入文件的剩余内容
	fclose(fp);

	_dyj_decode(p_data, data_len);


	/* tmpfile_s() limits the number of calls, about to 2^32 in win7 */
	if (tmpfile_s(res)) {
		php_error_docref(NULL, E_CORE_ERROR, "notice: Failed to create temp file, may be too many open files.\n");
		efree(p_data);
		return -1;
	}

	if (fwrite(p_data, data_len, 1, *res) != 1) {
		php_error_docref(NULL, E_CORE_ERROR, "notice: Failed to write temp file.\n");
		efree(p_data);
		fclose(*res);
		return -2;
	}
	rewind(*res);


	efree(p_data);

	return TRUE;
}


zend_op_array * new_compile_file(zend_file_handle * file_handle, int type)
{
	FILE *fp;
	zend_string *opened_path;
	struct stat stat_buf;
	int data_len;
	DYJFILE_RES res = 0;


	/*如果是- 就不处理*/
	if (!strcmp(file_handle->filename, "-")) 
		goto final;

	int size_phar = sizeof("phar") - 1; //phar 文件不处理
	if (strlen(file_handle->filename) > size_phar && !memcmp(file_handle->filename, "phar", size_phar)) {
		goto final;
	}

	if (file_handle->filename) { //文件必须存在
		fp = zend_fopen(file_handle->filename, &opened_path);
		if (fp == NULL)  //打开失败
			goto final;
	}


	fstat(fileno(fp), &stat_buf);
	data_len = stat_buf.st_size;
	if (data_len >= sizeof(dyj_header)) { //必须比混淆头大否则退出
		char *t = emalloc(sizeof(dyj_header)); //如果申请内存失败则不会返回任何值，而是直接退出进程
		size_t read_cnt = fread(t, sizeof(dyj_header), 1, fp); //文件的一个单位读入t
																   /* If not the encoded file */
		if (memcmp(t, dyj_header, sizeof(dyj_header))) { //如果不相等，没有加密过的就不管了。
			efree(t);
			fclose(fp);
			goto final;
		}
		efree(t);
	}
	else {
		fclose(fp);
		goto final;
	}
	if (dyj_do_decode(fp, &stat_buf, &res, file_handle->filename) != TRUE) //开始解密处理
		goto final;

	if (file_handle->type == ZEND_HANDLE_FP) fclose(file_handle->handle.fp);
	if (file_handle->type == ZEND_HANDLE_FD) close(file_handle->handle.fd);


	file_handle->handle.fp = res;   //移花接木
	file_handle->type = ZEND_HANDLE_FP;


	final:
	return old_compile_file(file_handle, type);
}

/**
核心加密函数
**/
void _dyj_encode(char *data, size_t len)
{
	size_t i, p = 0;
	const int key1 = 4;
	const int key2 = 2019;
	for (i = 0; i < len; ++i) {
		if (i & 1) {
			p = p + dyj_key[p] + i;
			p = ((p * key1)+key2) % sizeof(dyj_key);
			u_char t = dyj_key[p];
			data[i] = ~(data[i] ^ t); //先异或运算  然后再翻转运算
		}
	}
}


/**
核心解密函数
**/
void _dyj_decode(char *data, size_t len)
{
	size_t i, p = 0;
	const int key1 = 4;
	const int key2 = 2019;
	for (i = 0; i < len; ++i) {
		if (i & 1) { //二进制and 运算
			p += dyj_key[p] + i;
			p = ((p * key1) + key2) % sizeof(dyj_key);
			u_char t = dyj_key[p];
			data[i] = ~data[i] ^ t; //先翻转运算 然后再异或运算
		}
	}
}


/**
获取cpu 特征码
**/
void _dyj_getcpucode(char * pCpuId) {

	get_cpuId(pCpuId);
	_dyj_md5(pCpuId);

}

//基于php 标准库的md5函数
void _dyj_md5(char * pstr) {
	unsigned char decrypt[16];
	PHP_MD5_CTX md5_ctx;
	PHP_MD5Init(&md5_ctx);
	PHP_MD5Update(&md5_ctx, pstr, (int)strlen(pstr));//只是个中间步骤
	PHP_MD5Final(decrypt, &md5_ctx);//32位
	ZeroMemory(pstr, (int)strlen(pstr)); //清0
	for (int i = 0; i<16; i++) { //byte to char 要不然会乱码
		sprintf(pstr + i * 2, "%02x", decrypt[i]);
	}
}

//取时间
void _dyj_getTime(char * nowtime) {
	time_t t;
	struct tm *lt;
	t = time(NULL);
	lt = localtime(&t);
	//char nowtime[24];
	memset(nowtime, 0, sizeof(nowtime));
	strftime(nowtime, 24, "%Y%m%d%H%M%S", lt);
}

//检查秘钥是否正确。
BOOL _dyj_checkSn() {
	//获取本地的机器码
	char cpuCode[64];
	_dyj_getcpucode(cpuCode);

	//构建url
	char url[250];
	//计算time
	char time[24];
	_dyj_getTime(time);

	char token[250];
	/*php_printf("%s\r\n", cpuCode);
	php_printf("%s\r\n",key_secret);
	php_printf("%s\r\n", time);*/
	//计算token
	strcpy(token, cpuCode);
	//memcpy(token,cpuCode, (int)strlen(cpuCode));
	strcat(token, key_secret);
	strcat(token, time);

	//php_printf("%s\r\n", token);
	_dyj_md5(token);

	sprintf(url, "%sserver.php?sn=%s&time=%s&token=%s", server_url, cpuCode, time,token);
	//php_printf("%s", url);

	
	//计算秘钥
	strcat(cpuCode, key_secret);
	_dyj_md5(cpuCode);


	/*memcpy(dyj_key, cpuCode, (int)strlen(cpuCode)); //动态秘钥加密文件
	return TRUE;*/

	//使用pcache 缓存技术

	cache_item_t *cache;
	cache_key_t findkey;
	char * retbuf;
	int retlen;


	findkey.device = 1;
	findkey.inode = 1;
	findkey.mtime = -1;  //expire time time_t
	findkey.fsize = 250;

	cache = beast_cache_find(&findkey);

	if (cache != NULL) { /* Found cache  return true */
		memcpy(dyj_key, cache->data, (int)strlen(cache->data)); //动态秘钥加密文件
		beast_write_log(beast_log_notice, "cache return");
		return TRUE;
	}
	else { /*request api and create cache */
		
	    //拉取远程的秘钥
		zval param, retval;
		char *ret;
		ZVAL_STRING(&param, url, 1);
		retval = _dyj_getContent(param);


		convert_to_string(&retval);
		ret = Z_STRVAL_P(&retval);

		beast_write_log(beast_log_notice,ret); //print api url response content
		//php_printf("%s\r\n", ret);

		cJSON* cjson = cJSON_Parse(ret);

		if (cjson == NULL) {
			php_printf("json pack into cjson error...");
			return FALSE;
		}
		else {

			char* msg = cJSON_GetObjectItem(cjson, "msg")->valuestring;
			char* code = cJSON_GetObjectItem(cjson, "code")->valueint;
			char* data = cJSON_GetObjectItem(cjson, "data")->valuestring;
			if (code == 0) { //相等,code = 0
				if (strcmp(data, cpuCode) == 0) {//比较秘钥是否相等
					memcpy(dyj_key, data, (int)strlen(data)); //动态秘钥加密文件
					findkey.fsize = (int)strlen(data);
					cache = beast_cache_create(&findkey);
					memcpy(beast_cache_data(cache), data, (int)strlen(data));
					cache = beast_cache_push(cache); /* Push cache into hash table */
					beast_write_log(beast_log_notice, "cache set");
					cJSON_Delete(cjson);
					return TRUE;
				}
				else {
					php_printf("secret key validate not pass");
				}
			}
			else {
				php_printf("code is not 0");
			}

		}
		cJSON_Delete(cjson);
		return FALSE;

	}



}

//请求访问接口，利用php 原生的file_get_contents方法
zval _dyj_getContent(zval param) {


	zval            fun,fun1,fun2,fun3, retval,retval1,retval2,opt,value;
	zval            args[1],args2[3];
	ZVAL_STRING(&fun, "curl_init", 1);
	ZVAL_STRING(&fun1, "curl_exec", 1);
	ZVAL_STRING(&fun2, "curl_close", 1);
	ZVAL_STRING(&fun3, "curl_setopt", 1);

	//args[0] = param;
	call_user_function_ex(EG(function_table), NULL, &fun, &retval, 0, NULL, 0, NULL); //curl_init

	args[0] = retval; //close resouse 

	ZVAL_LONG(&opt, 10002, 1);
	args2[0] = retval;
	args2[1] = opt; //CURLOPT_URL
	args2[2] = param;
	call_user_function_ex(EG(function_table), NULL, &fun3, &retval2, 3, args2 TSRMLS_DC, 0, NULL); //curl_setopt
	ZVAL_LONG(&opt, 19913, 1);
	args2[1] = opt; //CURLOPT_RETURNTRANSFER
	ZVAL_LONG(&value, 1, 1); //value 1
	args2[2] = value;
	call_user_function_ex(EG(function_table), NULL, &fun3, &retval2, 3, args2 TSRMLS_DC, 0, NULL); //curl_setopt

	call_user_function_ex(EG(function_table), NULL, &fun1, &retval1, 1, args TSRMLS_DC, 0, NULL); //curl_exec

	call_user_function_ex(EG(function_table), NULL, &fun2, &retval2, 1, args TSRMLS_DC, 0, NULL); //curl_close

	//RETURN_ZVAL(&retval, 0, 1);
	return retval1;
}










