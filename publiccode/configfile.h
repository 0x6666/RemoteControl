///////////////////////////////////////////////////////////////
//
// FileName	: configfile.h 
// Creator	: ����
// Date		: 2012��6��14��, 15:15:36
// Comment	: �����ļ���������������
//			����һ�������ļ��Ĳ�����������
//			�����������ļ��Ǽ�ֵ�Ե���ʽ���ڵ� 
//			�磺
//				filename=configfile.h
//				filetype=head
//			�����ļ�ֻ֧����ע�ͣ�ע����'#'��ͷ��Ҳ����˵����
//			��'#'��ͷ���ж���ע�ʹ����ļ���ÿһ�в��ó���1024
//			���ֽڣ�������ܻ����.ֻ��key��û��value�ļ�ֵ��ʱ
//			û�е�����ģ��������ֱ�Ӻ���
//////////////////////////////////////////////////////////////

#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

//һ�е�����ֽ���
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

//һ����ֵ�ԵĽṹ��
typedef struct _tagKEY_VALUE{
	char* key;
	char* value;
}KEY_VALUE , *PKEY_VALUE;

//�����ļ��Ĳ������
typedef enum _CONFIG_FILE_RES{
	RCF_OK = 0,			//�����ɹ�
	RCF_OPEN_ERR,		//�ļ���ʧ��
	RCF_NO_VALUE,		//������ָ����ֵ
	RCF_PARAM,			//��������
	RCF_BUFF,			//���治��
}RCF;

//////////////////////////////////////////////////////////////////////////
//��ȡ�����ļ��е�ָ������ֵ
//param
//		key		Ҫ��ȡ�����ݵļ�
//		value	ֵ�Ļ���
//		valunBuflen	ֵ�Ļ����С
//		file	�����ļ�
//return
//		RCF_OK			�����ɹ�
//		RCF_NO_VALUE	û��ָ����ֵ
//		RCF_PARAM		�����������
//		RCF_OPEN_ERR	���ļ�ʧ��
RCF ReadConfValue(const char *key , char *value , unsigned int valunBuflen, const char *file);


//////////////////////////////////////////////////////////////////////////
//д�����ļ�,���ָ���������ڵĻ�����ӣ������滻
//param
//		key		Ҫд�������ļ��ļ�
//		value	Ҫд��ֵ 
//		file	�����ļ���·��
//return	�������
//		RCF_OK			�����ɹ�
//		RCF_PARAM		��������
//		RCF_OPEN_ERR	���ļ�ʧ��
//////////////////////////////////////////////////////////////////////////
RCF WriteConfValue(const char *key ,const char *value , const char *file);

//////////////////////////////////////////////////////////////////////////
//�������ļ���д��һ����ֵ��
//param
//		kvArr		һ����ֵ������
//		kvCount		Ҫд��ļ�ֵ������
//		file		�����ļ�·��
//		clearFile	�Ƿ���д֮ǰ�������ļ�
//					�������ΪTRUE�Ļ����Ǿ���д�����ļ�֮ǰ�Ƚ��ļ������
//					��ȫ������ɾ���Ҳ����˵�˺���ִ����֮���ļ��е����ݾ�
//					��kvArr�е����ݣ����ַ�ʽ�������ļ��е�ע�ͣ��Լ�kvArr
//					��û�е�����
//					�����������ΪFALSE�Ļ�ֻ���ں�����һ����¼һ����¼�ص�
//					��WriteConfValue�������Ļ��ᱣ��ԭ�е�ע�ͣ��Լ�kvArr��
//					û�е�����
//return	�������
//		RCF_OK			�����ɹ�
//		RCF_PARAM		��������
//		RCF_OPEN_ERR	���ļ�ʧ��
//////////////////////////////////////////////////////////////////////////
RCF WriteConfValueArr(PKEY_VALUE kvArr , int kvCount , const char* file , BOOL clearFile);

#endif
