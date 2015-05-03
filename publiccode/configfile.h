///////////////////////////////////////////////////////////////
//
// FileName	: configfile.h 
// Creator	: 杨松
// Date		: 2012年6月14日, 15:15:36
// Comment	: 配置文件操作函数的声明
//			这是一个配置文件的操作方法集合
//			操作的配置文件是键值对的形式存在的 
//			如：
//				filename=configfile.h
//				filetype=head
//			配置文件只支持行注释，注释以'#'开头，也就是说凡是
//			以'#'开头的行都作注释处理。文件的每一行不得超过1024
//			个字节，否则可能会出错.只有key而没有value的键值对时
//			没有的用意的，在这里会直接忽略
//////////////////////////////////////////////////////////////

#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

//一行的最大字节数
#define MAX_LINE_SIZE 1024

#ifndef BOOL
typedef int	BOOL;
#endif
#ifndef TRUE
#define TRUE 1
#endif // TRUE
#ifndef FALSE
#define FALSE 0 
#endif // FALSE

//一个键值对的结构体
typedef struct _tagKEY_VALUE{
	char* key;
	char* value;
}KEY_VALUE , *PKEY_VALUE;

//配置文件的操作结果
typedef enum _CONFIG_FILE_RES{
	RCF_OK = 0,			//操作成功
	RCF_OPEN_ERR,		//文件打开失败
	RCF_NO_VALUE,		//不存在指定的值
	RCF_PARAM,			//参数错误
	RCF_BUFF,			//缓存不都
}RCF;

//////////////////////////////////////////////////////////////////////////
//读取配置文件中的指定键的值
//param
//		key		要读取的数据的键
//		value	值的缓冲
//		valunBuflen	值的缓存大小
//		file	配置文件
//return
//		RCF_OK			操作成功
//		RCF_NO_VALUE	没有指定的值
//		RCF_PARAM		输入参数错误
//		RCF_OPEN_ERR	打开文件失败
RCF ReadConfValue(const char *key , char *value , unsigned int valunBuflen, const char *file);


//////////////////////////////////////////////////////////////////////////
//写配置文件,如果指定键不存在的话则添加，否则替换
//param
//		key		要写的配置文件的键
//		value	要写的值 
//		file	配置文件的路径
//return	操作结果
//		RCF_OK			操作成功
//		RCF_PARAM		参数错误
//		RCF_OPEN_ERR	打开文件失败
//////////////////////////////////////////////////////////////////////////
RCF WriteConfValue(const char *key ,const char *value , const char *file);

//////////////////////////////////////////////////////////////////////////
//向配置文件中写入一批键值对
//param
//		kvArr		一个键值对序列
//		kvCount		要写入的键值对数量
//		file		配置文件路径
//		clearFile	是否在写之前先清理文件
//					如果此域为TRUE的话，那就在写配置文件之前先将文件里的数
//					据全部清理干净，也就是说此函数执行完之后文件中的内容就
//					是kvArr中的内容，这种方式会清理文件中的注释，以及kvArr
//					中没有的数据
//					如果此域设置为FALSE的话只是在函数中一条记录一条记录地调
//					用WriteConfValue。这样的话会保存原有的注释，以及kvArr中
//					没有的数据
//return	操作结果
//		RCF_OK			操作成功
//		RCF_PARAM		参数错误
//		RCF_OPEN_ERR	打开文件失败
//////////////////////////////////////////////////////////////////////////
RCF WriteConfValueArr(PKEY_VALUE kvArr , int kvCount , const char* file , BOOL clearFile);

#endif
