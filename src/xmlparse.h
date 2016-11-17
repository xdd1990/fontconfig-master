/* Copyright (c) 2005 ����ʯwzs  */

#ifndef XmlParse_INCLUDED
#define XmlParse_INCLUDED 1
#ifdef WIN32 
	#define _XPATH_SAVELOG   "log/"
	#define _XDEBUGLOGFILE   "log/bbbb.log"
	#define _XRUNLOGFILE     "log/dddd.log"
#else
	#define _XPATH_SAVELOG   "/export/home/chcard/log"
	#define _XDEBUGLOGFILE   "/export/home/chcard/log/bbbb.log"
	#define _XRUNLOGFILE     "/export/home/chcard/log/dddd.log"
#endif
typedef struct {
  char *text ;
  int len ;
} XSTRING ;   /* X�ִ�, ֻ����ָ��(xbuf) */

struct ST_XNODE ;
typedef struct ST_XNODE XNODE ;
struct ST_XNODE {
 	XSTRING name ;   /*  �ڵ��� */ 
 	int index ;      /*  ����   */
 	XNODE *parentNode ;  /* ���ڵ�  */ 
 	int level ;      /* ���� */
 	int tagCount ;   /* �ӽڵ��� */
 	XSTRING data ;   /* Ԫ��ֵ  */
}  ;

typedef struct {
	char 	*xBuf;     /* x��Դ�洢 */
	char  *xBufEnd;  /* x��Դ��ֹ */
	char  *xBufPos;  /* x��Դ��ǰλ��  */
	XNODE		*nodeList;  /* �ڵ��б� */  
	XNODE   *rootNode;  /* ���ڵ�ָ�� */
	XNODE   *preNode;  /* �ϴβ����ڵ� */ 
	XNODE   *curNode;   /* ��ǰ�ڵ�ָ�� */
	int 	nodeCount;     /* �ڵ����� */
	int   nodeMaxCount ;    /* �ڵ������ */
}	XTREE ;

/*  ��ǰ����ʱ��  */
extern char * Xgetdatetime(char *dt) ;

/* д�ļ� */
extern void XPrtMess( ) ;

/* д������Ϣ  */
extern void XPrtDebug( ) ;

/* д������Ϣ  */
extern void XPrtErr( ) ;

extern int isSpace(int c) ;   /*  �ǿշ�  */
extern int isNameTChar(int c);   /* ��Ч������ǰ����  */
extern char * DelSpace( char *xbuf) ;   /* ɾ��ǰ���� */ 

/*	��xml��������X�� */
extern int  XmlParseRead(char *xmlbuf, int xmlsize, XTREE *xTree) ;

/* �ͷ�������ڴ� */
extern int XmlParseFree( XTREE *xTree ) ;

extern int printXtree(XTREE *xTree) ;

/*		�ӵ�ǰ�ڵ�����ȡ���ӽڵ�(���ݸ��ڵ����ͽڵ���): */
extern XNODE *XmlGetNode (XTREE *,  char *, char *) ;

/*		�ӵ�ǰ�ڵ�����ȡ���ӽڵ�(���ݸ��ڵ�ͽڵ���): */
extern XNODE *XmlGetChildNode (XTREE *xTree, XNODE *pnode,  char *nodename) ;

/*	�ӵ�ǰ�ڵ�����ȡ�ýڵ�(�ڵ���) */
extern XNODE *XmlGetNodeByName (XTREE *xTree, char *nodename) ;

/*	ȡ��Ҷ�ڵ�: */
extern XNODE *XmlGetTag (XTREE *xTree,  XNODE *pnode, char *tagname);

/*	�������ת��1: */
extern int XEnCode(XSTRING *xs, char *scode, char *dcode ) ;

/*	XML�������ת��: */
extern int XmlDataEnCode(XSTRING *xs ) ;

/*	XML������뷴ת��: */
extern int XmlDataUnCode(XSTRING *xs ) ;

/* дxmlͷ  */
extern char * XmlWriteHead(char *xmlbuf, char *encode) ;

/*  д��ʶͷ */
extern char * XmlWriteNodeBeg(char *xmlbuf, char *Tag);

/*  д��ʶβ */
extern char * XmlWriteNodeEnd(char *xmlbuf, char *Tag);

/*  д����Ԫ�� */
extern char * XmlWriteTag(char *xmlbuf, char *Tag, char *data ) ;

/*  д��չ(��Ҫ����ת��)������Ԫ�� */
extern char * XmlWriteExTag(char *xmlbuf, char *Tag, char *data ) ;

/*  дע�� */
extern char * XmlWriteNote (char *xmlbuf, char *note);

/*  д���⴮д���� */
extern char * XWriteBuf (char *buf, char *str) ;

#endif 


