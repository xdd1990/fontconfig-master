/* Copyright (c) 2005 王柱石wzs  */

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
} XSTRING ;   /* X字串, 只定义指针(xbuf) */

struct ST_XNODE ;
typedef struct ST_XNODE XNODE ;
struct ST_XNODE {
 	XSTRING name ;   /*  节点名 */ 
 	int index ;      /*  索引   */
 	XNODE *parentNode ;  /* 父节点  */ 
 	int level ;      /* 级别 */
 	int tagCount ;   /* 子节点数 */
 	XSTRING data ;   /* 元素值  */
}  ;

typedef struct {
	char 	*xBuf;     /* x树源存储 */
	char  *xBufEnd;  /* x树源终止 */
	char  *xBufPos;  /* x树源当前位置  */
	XNODE		*nodeList;  /* 节点列表 */  
	XNODE   *rootNode;  /* 根节点指针 */
	XNODE   *preNode;  /* 上次操作节点 */ 
	XNODE   *curNode;   /* 当前节点指针 */
	int 	nodeCount;     /* 节点总数 */
	int   nodeMaxCount ;    /* 节点最大数 */
}	XTREE ;

/*  当前日期时间  */
extern char * Xgetdatetime(char *dt) ;

/* 写文件 */
extern void XPrtMess( ) ;

/* 写调试信息  */
extern void XPrtDebug( ) ;

/* 写运行信息  */
extern void XPrtErr( ) ;

extern int isSpace(int c) ;   /*  是空否  */
extern int isNameTChar(int c);   /* 有效的名称前导符  */
extern char * DelSpace( char *xbuf) ;   /* 删除前导空 */ 

/*	读xml并解析到X树 */
extern int  XmlParseRead(char *xmlbuf, int xmlsize, XTREE *xTree) ;

/* 释放申请的内存 */
extern int XmlParseFree( XTREE *xTree ) ;

extern int printXtree(XTREE *xTree) ;

/*		从当前节点往下取得子节点(根据父节点名和节点名): */
extern XNODE *XmlGetNode (XTREE *,  char *, char *) ;

/*		从当前节点往下取得子节点(根据父节点和节点名): */
extern XNODE *XmlGetChildNode (XTREE *xTree, XNODE *pnode,  char *nodename) ;

/*	从当前节点往下取得节点(节点名) */
extern XNODE *XmlGetNodeByName (XTREE *xTree, char *nodename) ;

/*	取得叶节点: */
extern XNODE *XmlGetTag (XTREE *xTree,  XNODE *pnode, char *tagname);

/*	代码编码转换1: */
extern int XEnCode(XSTRING *xs, char *scode, char *dcode ) ;

/*	XML代码编码转换: */
extern int XmlDataEnCode(XSTRING *xs ) ;

/*	XML代码编码反转换: */
extern int XmlDataUnCode(XSTRING *xs ) ;

/* 写xml头  */
extern char * XmlWriteHead(char *xmlbuf, char *encode) ;

/*  写标识头 */
extern char * XmlWriteNodeBeg(char *xmlbuf, char *Tag);

/*  写标识尾 */
extern char * XmlWriteNodeEnd(char *xmlbuf, char *Tag);

/*  写数据元素 */
extern char * XmlWriteTag(char *xmlbuf, char *Tag, char *data ) ;

/*  写扩展(需要符号转换)的数据元素 */
extern char * XmlWriteExTag(char *xmlbuf, char *Tag, char *data ) ;

/*  写注释 */
extern char * XmlWriteNote (char *xmlbuf, char *note);

/*  写任意串写缓冲 */
extern char * XWriteBuf (char *buf, char *str) ;

#endif 


