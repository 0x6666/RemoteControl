///////////////////////////////////////////////////////////////
//
// FileName	: configfile.cpp 
// Creator	: ����
// Date		: 2012��6��14��, 15:15:36
// Comment	: �����ļ�����������ʵ��
//			  �����ļ�����Ҫ�Լ�ֵ�Դ��ڣ���key=value��������
//			  �����Եõ���key��value,���Զ�ȡ��Ӧkey��valueֵ��
//			  Ҳ���Եõ�������Ӧ��value;
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configfile.h"

#pragma warning (disable:4996)

//�����ͷż�ֵ���еĿռ�
#define tryFryKeyValue(x) do{	\
	if(NULL == (x)) break;		\
	if((x)->key != 0)			\
		free(((x)->key));		\
	if((x)->value != 0)			\
		free(((x)->value));		\
	} while (0)

#define tryFree(x) if (NULL != x) free(x)


//�ж��Ƿ�Ϊ�ո�
#define isSpace(x) ( (	' ' == (x)) ||	/*�ո�*/	\
					 ( '\r' == (x)) ||	/*�س�*/	\
					 ( '\n' == (x)) ||	/*����*/	\
					 ( '\t' == (x)) ||	/*ˮƽ�Ʊ�*/\
					 ( '\v' == (x)) ||	/*��ֱ�Ʊ�*/\
					 ( '\f' == (x)))	/*��ҳ*/

//////////////////////////////////////////////////////////////////////////
//��ȡ�ļ��ĳ���,ʧ�ܵĻ�����-1
static int getFilelen(FILE* file)
{
	int len = 0;
	int nCurPos = 0;

	//��Ҫ�����ļ���ԭ����ָ��
	if(-1 == (nCurPos = ftell(file)))
		return -1;

	//��λ���ļ��Ľ���λ��
	fseek(file , 0 , SEEK_END);
	
	//��ȡ�ļ������λ�� ��Ҳ�����ļ��Ĵ�С
	len = ftell(file);

	//��ԭ�ļ���ָ��
	fseek(file , nCurPos , SEEK_SET);

	return len;
}

//////////////////////////////////////////////////////////////////////////
//���ü�ֵ���еļ�
//param
//		pkv	��ֵ��
//		key	��ְ���еļ�
//return	�����Ƿ�ɹ�
static BOOL setKey(PKEY_VALUE  pkv, const char* key)
{
	int nLen;
	//�жϲ����Ƿ�
	if (NULL == pkv || NULL == key)
		return FALSE;

	//���ԭ���������ݵĻ�������ɾ��
	tryFree(pkv->key);
	
	nLen = strlen(key);
	pkv->key = (char*)malloc(nLen + 1);
	strcpy(pkv->key , key);

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//���ü�ֵ���е�ֵ
//param
//		pkv		��ֵ��
//		value	�����õ�ֵ
//return	�����Ƿ�ɹ�
static BOOL setValue(PKEY_VALUE  pkv ,const char* value)
{
	int nLen;

	if (NULL == pkv || NULL == value)
		return FALSE;

	//���ԭ�����л���ռ�Ļ����ͷ�
	tryFree(pkv->value);

	nLen = strlen(value);
	pkv->value = (char*)malloc(nLen + 1);
	strcpy(pkv->value , value);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//ȥ���ַ����Ҷ˿ո�
static char *trim_right(char *pstr)
{
	int i;
	i = strlen(pstr) - 1;
	while (isSpace(pstr[i]) && (i >= 0))
		pstr[i--] = '\0';
	return pstr;
}
//////////////////////////////////////////////////////////////////////////
//ȥ���ַ�����˿ո�
static char *trim_left(char *pstr)
{
	int i = 0,j;
	j = strlen(pstr) - 1;
	while (isSpace(pstr[i]) && (i <= j))
		i++;
	if (0 < i)
		strcpy(pstr, &pstr[i]);
	return pstr;
}

//////////////////////////////////////////////////////////////////////////
//ȥ���ַ���pstr���ߵĿո�
static char* StringTrim(char *pstr)
{
	char *p;
	p = trim_right(pstr);
	strcpy( pstr ,  trim_left(p));
	return  pstr;
}

//////////////////////////////////////////////////////////////////////////
//�������ļ���һ���ж���key��value,����itemָ��
//line--�������ļ�������һ��
//����0 �ɹ�
//����1 ����
//����2 ע��
//����3 ��Ч��ֵ��
//////////////////////////////////////////////////////////////////////////
static int  getItemFromLine(char *line, PKEY_VALUE item)
{
	char *p  = StringTrim(line);
	int  len = strlen(p);

	if( 0 >= len )			
		return 1;	//����
	else if('#' == p[0])
		return 2;	//ע��
	else{
		char* p2 = strchr(p , '=');

		if( NULL == p2 || (p2 + 1) == NULL )//��Ч��ֵ��
			return 3;

		//��'='���ж� 
		*p2 = '\0';
		p2++;
		setKey(item , StringTrim(p));
		setValue( item , StringTrim(p2));
	}
	return 0;//��ѯ�ɹ�
}

RCF ReadConfValue(const char *key , char *value , unsigned int valunBuflen, const char *file)
{
	char		line[MAX_LINE_SIZE + 1];
	FILE*		fp;
	KEY_VALUE	item = {0};
	int			flag = 0;

	//�������
	if (NULL == key || NULL == value || NULL == file) 
		return RCF_PARAM;

	//�������ļ�
	fp = fopen(file , "rb");
	strcpy(value , "");
	
	//�ļ��򿪴���
	if( NULL == fp )
		return RCF_OPEN_ERR;

	//���������ļ�
	while (fgets(line , MAX_LINE_SIZE , fp))
	{
		if(0 == getItemFromLine(line , &item))	
		{
			if(0 == strcmp(item.key , key))
			{	
				flag = 1;
				//�ҵ���ָ����ֵ��
				
				break;
			}
		}//
	}//while
	
	//�رմ򿪵��ļ�
	fclose(fp);
	
	if (flag == 1)
	{//�ҵ���
		if (strlen(item.value) >= valunBuflen)
		{//ò�ƻ��治��
			tryFryKeyValue(&item);
			return RCF_BUFF;
		}
		else
		{
			strcpy(value , item.value);
		}
	}

	//��Ҫ�Ļ��ͷż�ֵ�ԵĿռ�
	tryFryKeyValue(&item);
	
	if(1 == flag)	
		return RCF_OK;//�ɹ�
	else
		return RCF_NO_VALUE;
}


RCF WriteConfValue(const char* key , const char* value , const char* file)
{
	int		flag	= 0;	//�ж�Ҫ��ӵļ�ֵ���Ƿ��Ѿ�����
	FILE*	fp		= NULL;	//�����ļ�
	int		fLen	= 0;	//�����ļ�ԭ�еĳ���
	char	line[MAX_LINE_SIZE + 1] = {0};
	int		lastPos	= 0;	//��һ�ζ�ȡ�ļ���λ��
	int		curPos	= 0;	//�ļ��ĵ�ǰλ��
	char*	k		= NULL;
	char*	v		= NULL;
	char*	buf		= NULL;

	//�������
	if (NULL == key || NULL == value || NULL == file)
		return RCF_PARAM;

	//���ļ�
	fp = fopen(file,"r+b");
	if(fp == NULL)
	{//�ļ����ܲ�����
		//�����ļ�
		fp = fopen(file , "w");
		if(NULL == fp)//�����ļ�ʧ��
			return RCF_OPEN_ERR;
		fclose(fp);

		fp = fopen(file,"r+b");
		if(NULL == fp)//���ļ�ʧ��
			return RCF_OPEN_ERR;
	}
	//����ļ��ĳ���
	fLen = getFilelen(fp);

	//���������ļ�
	while(fgets(line, MAX_LINE_SIZE, fp))
	{
		k = StringTrim(line);
		if(0 >= strlen(k) )
			continue;	//�մ�
		else if(k[0] == '#')	
			continue;	//ע��
		else
		{//һ����Ч�ļ�ֵ��
			v = strchr(k , '=');
			if(v == NULL) 
				continue;	//ֻ��key��û��value�ļ�ֵ����û�������

			//����ǰ�нضϳɼ���ֵ�����Ӵ�
			*v = '\0';
			v++;
			k = StringTrim(k);	//��
			if (0 == strcmp(k , key)) //�ҵ���
			{
				flag = 1;//�ҵ�
				break;
			}
		}
		//����һ�¶�ȡ�ļ���λ��
		lastPos = ftell(fp);
	}//while

	if (0 == flag)		//û���ҵ�ָ����ֵ,ֱ��д�����
	{
		fseek(fp , 0 , SEEK_END);
		fprintf(fp , "%s=%s\r\n" , key , value);
		fclose(fp);
		return RCF_OK;
	}

	//������ָ���ļ����ڵ�ʱ��Ĳ���

	//�ȶ�ȡԭ�е�����
	curPos = ftell(fp);
	fLen -= curPos;
	if (0 == fLen)		
	{//��ʵ�Ѿ����ļ�β��
		fseek(fp , lastPos , SEEK_SET);		//��λ����һ�ζ�ȡ��λ�ã�����������д��
		fprintf(fp , "%s=%s\r\n" , key , value);
		fclose(fp);
		return RCF_OK;
	}

	//��Ҫ�ƶ�����
	buf = (char*)malloc(fLen);
	fread(buf , 1 , fLen , fp);		//��ǰ���ݺ��������

	//��Ҫ���õ�ֵд���ļ�
	fseek(fp , lastPos , SEEK_SET);		
	fprintf(fp , "%s=%s\r\n" , key , value);
	fwrite(buf , 1 , fLen , fp);
	fclose(fp);

	free(buf);

	return RCF_OK;
}

RCF WriteConfValueArr(PKEY_VALUE kvArr , int kvCount , const char* file , BOOL clearFile)
{
	int i;
	RCF res  = RCF_OK;

	if (NULL == kvArr || NULL == file || 0 > kvCount)
		return RCF_PARAM;
	
	if (FALSE == clearFile)
	{//����Ҫ������ԭ�е��ļ�
		for (i = 0 ; i < kvCount ; ++i )
		{
			res = WriteConfValue(kvArr[i].key , kvArr[i].value , file);
			if (RCF_OK != res)
				return res ;
		}
		return res;
	}
	else
	{
		//��Ҫ������ԭ�е��ļ�
		FILE* fp = NULL;

		fp = fopen(file , "wb");
		if (NULL == fp) 
			return RCF_OPEN_ERR;

		//һ��д��ÿһ����ֵ��
		for (i = 0 ; i < kvCount  ; ++i)
			fprintf(fp , "%s=%s\r\n" , kvArr[i].key , kvArr[i].value);

		fclose(fp);
		return RCF_OK;
	}
}
