///////////////////////////////////////////////////////////////
//
// FileName	: configfile.cpp 
// Creator	: 杨松
// Date		: 2012年6月14日, 15:15:36
// Comment	: 配置文件操作函数的实现
//			  配置文件中主要以键值对存在，如key=value，解析文
//			  件可以得到其key和value,可以读取相应key的value值，
//			  也可以得到配置相应的value;
//////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configfile.h"

#pragma warning (disable:4996)

//尝试释放键值对中的空间
#define tryFryKeyValue(x) do{	\
	if(NULL == (x)) break;		\
	if((x)->key != 0)			\
		free(((x)->key));		\
	if((x)->value != 0)			\
		free(((x)->value));		\
	} while (0)

#define tryFree(x) if (NULL != x) free(x)


//判断是否为空格
#define isSpace(x) ( (	' ' == (x)) ||	/*空格*/	\
					 ( '\r' == (x)) ||	/*回车*/	\
					 ( '\n' == (x)) ||	/*换行*/	\
					 ( '\t' == (x)) ||	/*水平制表*/\
					 ( '\v' == (x)) ||	/*垂直制表*/\
					 ( '\f' == (x)))	/*翻页*/

//////////////////////////////////////////////////////////////////////////
//获取文件的长度,失败的话返回-1
static int getFilelen(FILE* file)
{
	int len = 0;
	int nCurPos = 0;

	//需要保持文件的原来的指针
	if(-1 == (nCurPos = ftell(file)))
		return -1;

	//定位到文件的结束位置
	fseek(file , 0 , SEEK_END);
	
	//获取文件的最后位置 ，也就是文件的大小
	len = ftell(file);

	//还原文件的指针
	fseek(file , nCurPos , SEEK_SET);

	return len;
}

//////////////////////////////////////////////////////////////////////////
//设置键值对中的键
//param
//		pkv	键值对
//		key	兼职对中的键
//return	操作是否成功
static BOOL setKey(PKEY_VALUE  pkv, const char* key)
{
	int nLen;
	//判断参数是否
	if (NULL == pkv || NULL == key)
		return FALSE;

	//如果原来就有数据的话，就先删除
	tryFree(pkv->key);
	
	nLen = strlen(key);
	pkv->key = (char*)malloc(nLen + 1);
	strcpy(pkv->key , key);

	return TRUE;
}
//////////////////////////////////////////////////////////////////////////
//设置键值对中的值
//param
//		pkv		键值对
//		value	眼设置的值
//return	操作是否成功
static BOOL setValue(PKEY_VALUE  pkv ,const char* value)
{
	int nLen;

	if (NULL == pkv || NULL == value)
		return FALSE;

	//如果原来就有缓存空间的话先释放
	tryFree(pkv->value);

	nLen = strlen(value);
	pkv->value = (char*)malloc(nLen + 1);
	strcpy(pkv->value , value);

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//去除字符串右端空格
static char *trim_right(char *pstr)
{
	int i;
	i = strlen(pstr) - 1;
	while (isSpace(pstr[i]) && (i >= 0))
		pstr[i--] = '\0';
	return pstr;
}
//////////////////////////////////////////////////////////////////////////
//去除字符串左端空格
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
//去掉字符串pstr两边的空格
static char* StringTrim(char *pstr)
{
	char *p;
	p = trim_right(pstr);
	strcpy( pstr ,  trim_left(p));
	return  pstr;
}

//////////////////////////////////////////////////////////////////////////
//从配置文件的一行中读出key或value,返回item指针
//line--从配置文件读出的一行
//返回0 成功
//返回1 空行
//返回2 注释
//返回3 无效键值对
//////////////////////////////////////////////////////////////////////////
static int  getItemFromLine(char *line, PKEY_VALUE item)
{
	char *p  = StringTrim(line);
	int  len = strlen(p);

	if( 0 >= len )			
		return 1;	//空行
	else if('#' == p[0])
		return 2;	//注释
	else{
		char* p2 = strchr(p , '=');

		if( NULL == p2 || (p2 + 1) == NULL )//无效键值对
			return 3;

		//从'='处切断 
		*p2 = '\0';
		p2++;
		setKey(item , StringTrim(p));
		setValue( item , StringTrim(p2));
	}
	return 0;//查询成功
}

RCF ReadConfValue(const char *key , char *value , unsigned int valunBuflen, const char *file)
{
	char		line[MAX_LINE_SIZE + 1];
	FILE*		fp;
	KEY_VALUE	item = {0};
	int			flag = 0;

	//参数检测
	if (NULL == key || NULL == value || NULL == file) 
		return RCF_PARAM;

	//打开配置文件
	fp = fopen(file , "rb");
	strcpy(value , "");
	
	//文件打开错误
	if( NULL == fp )
		return RCF_OPEN_ERR;

	//遍历配置文件
	while (fgets(line , MAX_LINE_SIZE , fp))
	{
		if(0 == getItemFromLine(line , &item))	
		{
			if(0 == strcmp(item.key , key))
			{	
				flag = 1;
				//找到了指定键值对
				
				break;
			}
		}//
	}//while
	
	//关闭打开的文件
	fclose(fp);
	
	if (flag == 1)
	{//找到了
		if (strlen(item.value) >= valunBuflen)
		{//貌似缓存不够
			tryFryKeyValue(&item);
			return RCF_BUFF;
		}
		else
		{
			strcpy(value , item.value);
		}
	}

	//需要的话释放键值对的空间
	tryFryKeyValue(&item);
	
	if(1 == flag)	
		return RCF_OK;//成功
	else
		return RCF_NO_VALUE;
}


RCF WriteConfValue(const char* key , const char* value , const char* file)
{
	int		flag	= 0;	//判断要添加的键值对是否已经存在
	FILE*	fp		= NULL;	//配置文件
	int		fLen	= 0;	//配置文件原有的长度
	char	line[MAX_LINE_SIZE + 1] = {0};
	int		lastPos	= 0;	//上一次读取文件的位置
	int		curPos	= 0;	//文件的当前位置
	char*	k		= NULL;
	char*	v		= NULL;
	char*	buf		= NULL;

	//参数检测
	if (NULL == key || NULL == value || NULL == file)
		return RCF_PARAM;

	//打开文件
	fp = fopen(file,"r+b");
	if(fp == NULL)
	{//文件可能不存在
		//创建文件
		fp = fopen(file , "w");
		if(NULL == fp)//创建文件失败
			return RCF_OPEN_ERR;
		fclose(fp);

		fp = fopen(file,"r+b");
		if(NULL == fp)//打开文件失败
			return RCF_OPEN_ERR;
	}
	//获得文件的长度
	fLen = getFilelen(fp);

	//遍历配置文件
	while(fgets(line, MAX_LINE_SIZE, fp))
	{
		k = StringTrim(line);
		if(0 >= strlen(k) )
			continue;	//空串
		else if(k[0] == '#')	
			continue;	//注释
		else
		{//一个有效的键值对
			v = strchr(k , '=');
			if(v == NULL) 
				continue;	//只有key而没有value的键值对是没有意义的

			//将当前行截断成键和值两个子串
			*v = '\0';
			v++;
			k = StringTrim(k);	//键
			if (0 == strcmp(k , key)) //找到？
			{
				flag = 1;//找到
				break;
			}
		}
		//保存一下读取文件的位置
		lastPos = ftell(fp);
	}//while

	if (0 == flag)		//没有找到指定的值,直接写到最后
	{
		fseek(fp , 0 , SEEK_END);
		fprintf(fp , "%s=%s\r\n" , key , value);
		fclose(fp);
		return RCF_OK;
	}

	//以下是指定的键存在的时候的操作

	//先读取原有的数据
	curPos = ftell(fp);
	fLen -= curPos;
	if (0 == fLen)		
	{//其实已经到文件尾了
		fseek(fp , lastPos , SEEK_SET);		//定位到上一次读取的位置，将数据重新写入
		fprintf(fp , "%s=%s\r\n" , key , value);
		fclose(fp);
		return RCF_OK;
	}

	//需要移动数据
	buf = (char*)malloc(fLen);
	fread(buf , 1 , fLen , fp);		//当前数据后面的数据

	//将要设置的值写入文件
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
	{//不需要先清理原有的文件
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
		//需要先清理原有的文件
		FILE* fp = NULL;

		fp = fopen(file , "wb");
		if (NULL == fp) 
			return RCF_OPEN_ERR;

		//一次写入每一个键值对
		for (i = 0 ; i < kvCount  ; ++i)
			fprintf(fp , "%s=%s\r\n" , kvArr[i].key , kvArr[i].value);

		fclose(fp);
		return RCF_OK;
	}
}
