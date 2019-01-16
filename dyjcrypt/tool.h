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


#define DYJFILE_RES FILE*

typedef unsigned char u_char;
typedef const char c_char;
/**
����ͷ������ʶ���ļ��Ƿ����
**/
const u_char dyj_header[] = {
	0x49, 0x20, 0x6c, 0x6f,
	0x76, 0x65, 0x20, 0x63,
	0x68, 0x69, 0x6e, 0x61,
};

/**
��Կ��������������
**/
const u_char dyj_key[] = {
	0x41,0x75,0x74,0x68,0x6f,0x72,0x20,0x62,0x79,0x20,0x64,0x75,0x79,0x75,0x6e,0x6a,0x69,
	0x61,0x6e,0x67
};
//�����ַ���
const char key_secret[] = "20190114";

const char server_url[] = "http://localhost/";

zend_op_array * (*old_compile_file)(zend_file_handle *, int); //����ָ��

zend_op_array * new_compile_file(zend_file_handle *, int); //�µı����ļ�����

int dyj_do_decode(DYJFILE_RES, struct stat *,DYJFILE_RES *,c_char *);

void _dyj_encode(char *, size_t);

void _dyj_decode(char *, size_t);

void _dyj_md5(char * pstr);

//�����Կ�Ƿ���ȷ
BOOL _dyj_checkSn(char * pstr);

//���ʽӿ�
zval _dyj_getContent(zval param);

//ȡʱ��
void _dyj_getTime(char * nowtime);



/**
Ĭ�Ͻ���
**/
int dyj_do_decode(FILE *fp, struct stat *stat_buf, DYJFILE_RES *res, const char *file_name)
{
	char *p_data;
	size_t data_len;
	size_t write_len;

	data_len = stat_buf->st_size - sizeof(dyj_header); //���˻���ͷʣ��ĳ���
	p_data = (char *)emalloc(data_len); //����ʣ��ĳ���
	fseek(fp, sizeof(dyj_header), SEEK_SET); //�ض�λ�ļ���ָ��,ƫ��һ��ͷ���ľ���
	fread(p_data, data_len, 1, fp); //����ͷ�������ļ���ʣ������
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

	return SUCCESS;
}


zend_op_array * new_compile_file(zend_file_handle * file_handle, int type)
{
	FILE *fp;
	zend_string *opened_path;
	struct stat stat_buf;
	int data_len;
	DYJFILE_RES res = 0;

	/*�����- �Ͳ�����*/
	if (!strcmp(file_handle->filename, "-")) 
		goto final;

	int size_phar = sizeof("phar") - 1; //phar �ļ�������
	if (strlen(file_handle->filename) > size_phar && !memcmp(file_handle->filename, "phar", size_phar)) {
		goto final;
	}

	if (file_handle->filename) { //�ļ��������
		fp = zend_fopen(file_handle->filename, &opened_path);
		if (fp == NULL)  //��ʧ��
			goto final;
	}

	fstat(fileno(fp), &stat_buf);
	data_len = stat_buf.st_size;
	if (data_len >= sizeof(dyj_header)) { //����Ȼ���ͷ������˳�
		char *t = emalloc(sizeof(dyj_header)); //��������ڴ�ʧ���򲻻᷵���κ�ֵ������ֱ���˳�����
		size_t read_cnt = fread(t, sizeof(dyj_header), 1, fp); //�ļ���һ����λ����t
																   /* If not the encoded file */
		if (memcmp(t, dyj_header, sizeof(dyj_header))) { //�������ȣ�û�м��ܹ��ľͲ����ˡ�
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
	if (dyj_do_decode(fp, &stat_buf, &res, file_handle->filename)) //��ʼ���ܴ���
		goto final;

	if (file_handle->type == ZEND_HANDLE_FP) fclose(file_handle->handle.fp);
	if (file_handle->type == ZEND_HANDLE_FD) close(file_handle->handle.fd);


	file_handle->handle.fp = res;   //�ƻ���ľ
	file_handle->type = ZEND_HANDLE_FP;


	final:
	return old_compile_file(file_handle, type);
}

/**
���ļ��ܺ���
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
			data[i] = ~(data[i] ^ t); //���������  Ȼ���ٷ�ת����
		}
	}
}


/**
���Ľ��ܺ���
**/
void _dyj_decode(char *data, size_t len)
{
	size_t i, p = 0;
	const int key1 = 4;
	const int key2 = 2019;
	for (i = 0; i < len; ++i) {
		if (i & 1) { //������and ����
			p += dyj_key[p] + i;
			p = ((p * key1) + key2) % sizeof(dyj_key);
			u_char t = dyj_key[p];
			data[i] = ~data[i] ^ t; //�ȷ�ת���� Ȼ�����������
		}
	}
}


/**
��ȡcpu ������
**/
void _dyj_getcpucode(char * pCpuId) {

	get_cpuId(pCpuId);
	_dyj_md5(pCpuId);

}

//����php ��׼���md5����
void _dyj_md5(char * pstr) {
	unsigned char decrypt[16];
	PHP_MD5_CTX md5_ctx;
	PHP_MD5Init(&md5_ctx);
	PHP_MD5Update(&md5_ctx, pstr, (int)strlen(pstr));//ֻ�Ǹ��м䲽��
	PHP_MD5Final(decrypt, &md5_ctx);//32λ
	ZeroMemory(pstr, (int)strlen(pstr)); //��0
	for (int i = 0; i<16; i++) { //byte to char Ҫ��Ȼ������
		sprintf(pstr + i * 2, "%02x", decrypt[i]);
	}
}

//ȡʱ��
void _dyj_getTime(char * nowtime) {
	time_t t;
	t = time(NULL);
	struct tm *lt;
	int ii = time(&t);
	t = time(NULL);
	lt = localtime(&t);
	//char nowtime[24];
	memset(nowtime, 0, sizeof(nowtime));
	strftime(nowtime, 24, "%Y%m%d%H%M%S", lt);
}

//�����Կ�Ƿ���ȷ��
BOOL _dyj_checkSn() {
	//��ȡ���صĻ�����
	char cpuCode[64];
	_dyj_getcpucode(cpuCode);

	//����url
	char url[250];
	//����time
	char time[24];
	_dyj_getTime(time);

	char token[250];
	/*php_printf("%s\r\n", cpuCode);
	php_printf("%s\r\n",key_secret);
	php_printf("%s\r\n", time);*/
	//����token
	memcpy(token,cpuCode, (int)strlen(cpuCode));
	strcat(token, key_secret);
	strcat(token, time);

	//php_printf("%s\r\n", cpuCode);
	//php_printf("%s\r\n", token);
	_dyj_md5(token);

	sprintf(url, "%sserver.php?sn=%s&time=%s&token=%s", server_url, cpuCode, time,token);
	//php_printf("%s", url);

	
	//������Կ
	strcat(cpuCode, key_secret);
	_dyj_md5(cpuCode);

	//��ȡԶ�̵���Կ
	zval param, retval;
	char *ret;
	ZVAL_STRING(&param, url, 1);
	retval = _dyj_getContent(param);

	convert_to_string(&retval);
	ret = Z_STRVAL_P(&retval);

	//php_printf("%s\r\n", ret);

	cJSON* cjson = cJSON_Parse(ret);

	if (cjson == NULL) {
		php_printf("json pack into cjson error...");
		return FALSE;
	}else {

		char* msg = cJSON_GetObjectItem(cjson, "msg")->valuestring;
		char* code = cJSON_GetObjectItem(cjson, "code")->valueint;
		char* data = cJSON_GetObjectItem(cjson, "data")->valuestring;
		if (code == 0) { //���,code = 0
			if (strcmp(data, cpuCode) == 0) {//�Ƚ���Կ�Ƿ����
				cJSON_Delete(cjson);
				return TRUE;
			}
			else {
				//php_printf("secret key validate not pass");
			}
		}
		else {
			//php_printf("code is not 0");
		}

	}
	cJSON_Delete(cjson);
	return FALSE;

}

//������ʽӿڣ�����php ԭ����file_get_contents����
zval _dyj_getContent(zval param) {
	zval            fun, retval;
	zval            args[1];
	ZVAL_STRING(&fun, "file_get_contents", 1);
	args[0] = param;
	call_user_function_ex(EG(function_table), NULL, &fun, &retval, 1, args, 0, NULL);
	//RETURN_ZVAL(&retval, 0, 1);
	return retval;
}










