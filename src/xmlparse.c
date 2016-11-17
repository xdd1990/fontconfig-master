/* Copyright (c) 2005 王柱石wzs  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <varargs.h>
#include "xmlparse.h"
/*xml得到节点名值, 返回: 1节点标识名开头, 2节点标识名结尾, 3 注释 , 4元素数据  <=0有错误 */
#define XVAL_NBEGIN 	1
#define XVAL_NEND 		2
#define XVAL_NOTE			3
#define XVAL_DATA 		4
#define XVAL_TAG 			5      /* 空标志 */
#define XVAL_NONE			0
#define XVAL_ERROR		-1

/*  当前日期时间  */
char * Xgetdatetime(dt)
char *dt;
{
   struct tm ptm;
   #ifndef WIN32
	time_t  ttm;
   #endif
   tzset();
   #ifdef WIN32
	   _getsystime(&ptm) ;   /* windows  */
	#else
		time(&ttm);
		localtime_r(&ttm, &ptm );   /*Unix C */
   #endif
   sprintf(dt ,"%4d%02d%02d%02d%02d%02d\0",
        1900+ptm.tm_year, ptm.tm_mon+1,ptm.tm_mday, ptm.tm_hour, ptm.tm_min, ptm.tm_sec );
   return dt;
}

/* 写文件 */
void XPrtMess(fname,  fmtstr, va_alist )
char *fname, *fmtstr;
va_dcl
{
 va_list ap;
 FILE *  fp;
 char    tmpstr[50]; 

	va_start(ap);

        if (*fname <=' ') {
                vprintf( fmtstr, ap);
        }
        else { 
			sprintf(tmpstr, "%s%s\0", _XPATH_SAVELOG, fname);
	        fp = fopen (tmpstr, "a");
        	if (fp)
        	{
                	vfprintf(fp, fmtstr, ap);
                	fclose(fp);
        	}
	}
        va_end(ap);
}

/* 写调试信息  */
void XPrtDebug( fmtstr, va_alist )
char *fmtstr;
va_dcl
{
 va_list ap;
 FILE *  fp;
 char    tmpstr[30]; 
/* if ( getenv("DEBUG")==NULL || atoi(getenv("DEBUG"))==0 )
 return ; */
        va_start(ap);

        if (* _XDEBUGLOGFILE <=' ') {
                vprintf( fmtstr, ap);
                printf( "\n");
        }
        else { 
	        fp = fopen(_XDEBUGLOGFILE, "a");
        	if (fp)
        	{
                	fprintf(fp,"%s =>",Xgetdatetime(tmpstr) );
                	vfprintf(fp, fmtstr, ap);
                	fprintf(fp, "\n");
                	fclose(fp);
        	}
	}
        va_end(ap);
}

/* 写运行信息  */
void XPrtErr( fmtstr, va_alist )
char *fmtstr;
va_dcl
{
 va_list ap;
 char    tmpstr[30]; 
 FILE *  fp;

        va_start(ap);

        if (* _XRUNLOGFILE <=' ') {
                vprintf( fmtstr, ap);
                printf( "\n");
        }
        else { 
	        fp = fopen(_XRUNLOGFILE, "a");
        	if (fp)
        	{
                	fprintf(fp,"%s =>",Xgetdatetime(tmpstr) );
                	vfprintf(fp, fmtstr, ap);
                	fprintf(fp, "\n");
                	fclose(fp);
        	}
	}
        va_end(ap);
}

int isSpace(int c)   /*  是空否  */
{
  switch (c) {
  case 0x20:
  case 0xD:
  case 0xA:
  case 0x9:
    return 1;
  }
  return 0;
}

int isNameTChar(int c)   /* 有效的名称前导符  */
{
	return (c>='A' && c<='Z') || (c>='a' && c<='z') || (c=='_') ;  
}

int EqXString (XSTRING *dxs, XSTRING *sxs )    /*  X字串赋值  */
{
   dxs->text = sxs->text ;
   dxs->len = sxs->len ;
   return 0;
}

int EqXNode (XNODE *dx, XNODE *sx )    /*  X节点赋值  */
{
   EqXString ( &(dx->name), &(sx->name) ); 
   dx->index = sx->index ;
   dx->parentNode = sx->parentNode ;
   dx->level = sx->level ;
   dx->tagCount = sx->tagCount ;
   EqXString ( &(dx->data), &(sx->data) );  
   return 0;
}


/*	增加同级节点:   */
XNODE *XAddNode(XTREE *xTree , XSTRING *name)
{
	if (xTree->curNode->parentNode == NULL  || xTree->nodeCount >= xTree->nodeMaxCount ) return NULL ;
	(xTree->nodeCount) ++ ;
	xTree->preNode = xTree->curNode ;
	xTree->curNode = & (xTree->nodeList[ xTree->nodeCount -1 ] ) ;
	xTree->curNode->index =  xTree->nodeCount -1 ;
	EqXString (&(xTree->curNode->name), name);
	xTree->curNode->parentNode =  xTree->preNode->parentNode ;
	xTree->curNode->level =  xTree->preNode->level ;
	(xTree->curNode->parentNode->tagCount)	++ ;
	return (xTree->curNode);
}

/* 	增加子节点:   */
XNODE *XAddSubNode(XTREE *xTree , XSTRING *name)
{
	if (xTree->curNode == NULL  || xTree->nodeCount >= xTree->nodeMaxCount ) return NULL ;
	(xTree->nodeCount) ++ ;
  xTree->preNode = xTree->curNode ;
  xTree->curNode = & (xTree->nodeList[ xTree->nodeCount -1 ]) ;
  xTree->curNode->index =  xTree->nodeCount -1 ;
	EqXString (&(xTree->curNode->name), name);
	xTree->curNode->parentNode =  xTree->preNode ;
  xTree->curNode->level =  xTree->preNode->level +1 ;
  (xTree->preNode->tagCount)	++ ;
  return (xTree->curNode);
}

/*	返回父节点:   */
XNODE *XRetParentNode(XTREE *xTree )
{
   if (xTree->curNode->parentNode == NULL ) return NULL;
   xTree->preNode = xTree->curNode ;
   xTree->curNode = xTree->curNode->parentNode ;
   return (xTree->curNode);
}

/*	修改当前节点元素值: */
int XElementData(XTREE *xTree , XSTRING *data)
{
   if (xTree->curNode == NULL ) return -1 ;
 /*  if (xTree->curNode->tagCount>0) return -2; */
   EqXString ( &(xTree->curNode->data), data) ;  
   return 0;
}

char * DelSpace( char *xbuf)    /* 删除前导空 */ 
{
	  char *p ;
	  for (p=xbuf ; isSpace(*p) ; p++);   /* 去掉空字符 */
	  return (p);
}

int XmlAttr( XTREE *xTree, XSTRING *val)  /* 处理属性: 当作叶节点, 子节点数为-1, 返回增加的属性节点数 */
{
	  XSTRING xn, xv;
	  int k=0;
	  char *p1, *p, *p2;
	  p=DelSpace(val->text);
	  while (p && *p) { 
	  	p1= p ;
	    p2=strchr(p1, '=');
	    if (p2==NULL) break ;
	    xn.text = p1 ;
	    *p2='\0';
	    xn.len = p2- p1 ;
	    p=DelSpace(p2+1);
	    if (*p !='"') break ;
	    p1= p ;
	    p2=strchr(p1+1, '"');
	    if (p2==NULL) break ;
	    xv.text = p1+1;
	    *p2 ='\0';
	    xv.len = p2-p1-1;
	    /* 增加属性值节点  */
	    if (XAddSubNode(xTree, &xn)==NULL) break ;
	    k++;	
	    if (XElementData(xTree , &xv) <0 ) break ;
	    xTree->curNode->tagCount = -1;
	    XRetParentNode( xTree )	;
	    p=DelSpace(p2+1);
		}
		return (k);
}

/*  xml得到节点名值, 返回: 1节点标识名开头, 2节点标识名结尾, 3 注释 , 4元素数据  0无  -1有错误 */
int XmlReadNodeVal(XTREE *xTree, XSTRING *val )
{
    char *p ,*p1, *p2;
    int ivtype=XVAL_NONE ;
    p=xTree->xBufPos ;
    p=DelSpace( xTree->xBufPos)  ;   /* 删除前导空 */ 
    if (*p !='<') {     /* 数据 */
    	p1 = p;
    	p2=strchr(p1, '<'); 
    	val->text = p1;
    	val->len  = ( p2 ? p2-p1 : 0);	
    	ivtype = ( p2 && p2 < xTree->xBufEnd ?  XVAL_DATA : XVAL_NONE) ; 
    	xTree->xBufPos = (p2 ? p2 : xTree->xBufEnd);
    	goto OKExit33qqq ;	
    }
    
    /*  前导为<  */
     p++;
     if ( *p=='/' )   {    /* 标记尾 */
      	p1 = p+1;
      	p2=strchr(p1, '>'); 
      	if (p2==NULL) {  /*  错误 */
      		 goto ErrExit33qqq ;
      	} else {
      		ivtype=XVAL_NEND;
      		val->text = p1;
      		val->len = p2-p1;
      		xTree->xBufPos = p2 +1  ; 
        }
    	}
    	else if (isNameTChar(*p))  {   /* 标记头  */
 				p1=p;
 				p2=strchr(p1,'>');
      	if (p2==NULL) {  /*  错误 */
      		 goto ErrExit33qqq ;
      	} 				
 				for (p=p1; *p!=' ' && p<p2; p++);
 				if (*(p2-1)=='/') {  /* 处理空标记<abc/> */
 					ivtype=11;
 					val->text = p1;
 					if (p>=p2) p=p2-1;
 					val->len = p - p1;
 					if (	XAddSubNode( xTree , val ) == NULL) goto ErrExit33qqq ;
 					if (p < p2-1 ) /*  增加属性节点  */ 
 					{
 						  *(p2-1) ='\0';
 						  p++;
 						  val->text = p ;
 						  val->len = p2-1- p;
 						  ivtype += XmlAttr(xTree, val );
 					}
 					XRetParentNode( xTree )	;	
 				} else {	
 					ivtype=XVAL_NBEGIN;
 					val->text = p1;
 					val->len = p - p1;
          if (p < p2)    /*  增加属性节点  */ 
          {
          	  if (	XAddSubNode( xTree , val ) == NULL) goto ErrExit33qqq ;
          	  *(p2) ='\0';
          	  p++;
 						  val->text = p ;
 						  val->len = p2 - p;
          	  ivtype = 11+ XmlAttr(xTree, val );
        	}						
 				}
				xTree->xBufPos = p2 +1  ;
      }
      else if (*p=='!') {     
      	if (*(p+1)=='-' &&  *(p+2)=='-')   /* 注释 */
      	{ 
      		p+=3;  
      		p1=DelSpace(p);
      		p2=strstr(p1, "-->");   /*注释尾 */
      		if (p2==NULL) {  /*  错误 */
	      		 goto ErrExit33qqq ;
      	  } else {
      	  	ivtype=XVAL_NOTE;
      	  	xTree->xBufPos = p2+3;
      	  	val->text = p1;
      	  	val->len = (p2 - p1) / sizeof(char) ;
      	  }
      		goto OKExit33qqq ;
      	}
      }
    
    OKExit33qqq:
    	return (ivtype) ;
   	ErrExit33qqq :
   	   ivtype=XVAL_ERROR;
       xTree->xBufPos = xTree->xBufEnd ; 
			 return (ivtype) ; 	
}

/*	读xml并解析到X树 */
int  XmlParseRead(char *xmlbuf, int xmlsize, XTREE *xTree) 
{
		char *p, *p1, *pend = xmlbuf + xmlsize;
		XSTRING  xstr ;
		XNODE *xn ;
		register int k;
		int maxnn=0, vtype=XVAL_NONE,  vtype0=XVAL_NONE  ;
	  
	  memset( xTree, 0x00, sizeof(XTREE) );
    /*memset( xTree, 0x00, sizeof(xTree) );*/

	  if (memcmp(xmlbuf,"<?xml version=", 14)!=0) 
	  {	
	  	XPrtErr("无XML内容头!");
	  	return -1;
		}	
	  p=strstr(xmlbuf, "?>")	;
	  if (p==NULL) 
	  {	
	  	XPrtErr("XML内容头无结尾!");
	  	return -1;
		}	

	  p1=DelSpace(p+2);
	  for (p=p1, maxnn=1; p<pend ; p++) {
		  if (*p=='>' || *p=='"') maxnn++;
		  if (*p=='>') if (*(p-1) == '/')  maxnn++;  /*  空标记  */ 
	  }
	  
	  maxnn = maxnn / 2 + 2 ;	
	  xTree->xBuf = xmlbuf;
	  xTree->xBufEnd = xmlbuf + xmlsize ;
    xTree->xBufPos = p1 ;
    
	  for (k=0; k<99; k++) {
	  	vtype = XmlReadNodeVal( xTree, &xstr); 	
	  	if (vtype==XVAL_NBEGIN) break;
	  	else if (vtype!=XVAL_NOTE) break ;
		}
		
		xTree->nodeCount = 0;
    if 	(vtype!=XVAL_NBEGIN) 
    {
    		xTree->nodeMaxCount = 0;
    		XPrtDebug("空XML内容");
    		return 0;
  	}
  	
  	XPrtDebug("===申请节点存储空间=%d ", maxnn);
	  xTree->nodeList = (XNODE *) calloc( maxnn , sizeof(XNODE) );   /* 申请节点存储空间 */
	  if (xTree->nodeList == NULL ) 
	  {
	  	XPrtErr("申请节点存储空间(%d)失败!", maxnn );
	  	return -2;
	  }	
	  
	  xTree->nodeMaxCount = maxnn ;
	  memset( xTree->nodeList , 0x00, maxnn * sizeof(XNODE) );
		xTree->rootNode = &(xTree->nodeList[0]);   /* 根节点 */
		xTree->rootNode->parentNode = NULL ;
		xTree->rootNode->index = xTree->rootNode->level =0;
		*(xstr.text+xstr.len)='\0';
		EqXString (&(xTree->rootNode->name), &xstr);
		xTree->curNode = xTree->rootNode ;
		xTree->nodeCount=1;
		k=0;
    while (k<maxnn  )
    {
  		vtype0 = vtype ;
    	vtype = XmlReadNodeVal( xTree, &xstr);
    	if ( vtype <= XVAL_NONE) {  break;  }
    	switch	(vtype) {
    		case XVAL_NOTE : 
    				vtype = vtype0 ;
    				break;
    		case XVAL_NBEGIN :
						if (	XAddSubNode( xTree , &xstr ) == NULL) k=maxnn+1 ;			
    		  	break ;
    		case XVAL_NEND :
						XRetParentNode( xTree ) ;  		  /*	返回父节点:   */
						break;
				case XVAL_DATA :
						XElementData( xTree , &xstr ) ;	/* 	修改当前节点元素值: */
						break;
				default :
					  if (vtype>10)	 break;  /* printf(" ===c增加%d\n",  vtype -10 ); */
    	}
    }
  *xmlbuf = '\0';   /* 置已解析标志  */  
  maxnn = xTree->curNode->index ;   /* 最终当前节点号 */   
	for (k=0; k<xTree->nodeCount ; k++){
		xn =&( xTree->nodeList[k] );
		if (xn->name.text)	*(xn->name.text + xn->name.len) ='\0'; else xn->name.text=xmlbuf ;
		if (xn->data.text) *(xn->data.text + xn->data.len) ='\0'; else xn->data.text=xmlbuf ;  
	}
  XPrtDebug	 ( " 节点数=%d\n", xTree->nodeCount);
  if (maxnn) XPrtErr("XML文件不完整(%d) ", maxnn );
	xTree->curNode= xTree->rootNode ;  
  return (maxnn);
}

int XmlParseFree( XTREE *xTree )
{
	if (xTree->nodeMaxCount>0)  free(xTree->nodeList);
	xTree->nodeMaxCount=xTree->nodeCount = 0;	
	return 0;	
}
 
int printXtree(XTREE *xTree)
{
	int k ;
	XNODE *xn ;
	XPrtDebug("   ********  xTree 结构, 总节点数=%d, 节点空间=%d  ********  ", xTree->nodeCount, xTree->nodeMaxCount);
	XPrtDebug("\t节点号	父节点	节点级	子节数	节点名	节点数据");
	for (k=0; k<xTree->nodeCount ; k++){
		xn =&( xTree->nodeList[k] );
		XPrtDebug("\t%d	%d	%d	%d	[%s]	[%s]",xn->index,(xn->parentNode ? xn->parentNode->index : -1),  xn->level, xn->tagCount,xn->name.text, xn->data.text );
	}
		XPrtDebug("*****************************************************************");
    return 0;
} 

/*	从当前节点往下取得节点: */
XNODE *XmlGetNodeByName (XTREE *xTree, char *nodename)
{
	 register int k;
	XNODE *xn ;
	for (k=xTree->curNode->index+1 ; k< xTree->nodeCount ; k++) 
	{
		xn =&( xTree->nodeList[k] );
		if (memcmp(xn->name.text, nodename, xn->name.len )==0 ) break;
	}
	if (k>= xTree->nodeCount) return NULL ;
	xTree->curNode = xn ;
	return ( xn ) ; 
}

/*		从当前节点往下取得子节点(根据父节点名和节点名): */
XNODE *XmlGetNode (XTREE *xTree, char *pname, char *nodename)
{
	 register int k;
	XNODE *xn ;
	for (k=xTree->curNode->index+1 ; k< xTree->nodeCount ; k++) 
	{
		xn =&( xTree->nodeList[k] );
		if (memcmp(xn->name.text, nodename, xn->name.len )==0  && memcmp(xn->parentNode->name.text, pname, xn->parentNode->name.len)==0 ) break;
	}
	if (k>= xTree->nodeCount) return NULL ;
	xTree->curNode = xn ;
	return ( xTree->curNode ) ; 
}

/*		从当前节点往下取得子节点(根据父节点和节点名): */
XNODE *XmlGetChildNode (XTREE *xTree, XNODE *pnode,  char *nodename)
{
	 register int k;
	XNODE *xn ;
	for (k=xTree->curNode->index+1 ; k< xTree->nodeCount ; k++) 
	{
		xn =&( xTree->nodeList[k] );
		if (xn->level <= pnode->level ) return NULL ;
		if (xn->parentNode == pnode  &&  memcmp(xn->name.text, nodename, xn->name.len )==0) break;
	}
	if (k>= xTree->nodeCount) return NULL ;
	xTree->curNode = xn ;
	return ( xTree->curNode ) ; 
}

/*	取得叶节点: */
XNODE *XmlGetTag (XTREE *xTree,  XNODE *pnode, char *tagname)
{
	 register int k;
	XNODE *xn ;
	if (pnode == NULL)  return NULL ;
	for (k = pnode->index+1 ; k< xTree->nodeCount ; k++) 
	{
		xn =&( xTree->nodeList[k] );
		if (xn->level <= pnode->level ) return NULL ;
		if (xn->parentNode == pnode  &&  memcmp(xn->name.text, tagname, xn->name.len )==0) break;
	}
	if (k>= xTree->nodeCount) return NULL ;
	xTree->curNode = xn ;
	return ( xTree->curNode ) ; 
}

/*	代码编码转换1: */
int XEnCode(XSTRING *xs, char *scode, char *dcode )
{
	int k, lens=strlen(scode), lend=strlen(dcode), nrr=0;
	char *tmpbuf, *p,*p0, *pt,  *pend  ;
  if (strstr(xs->text, scode)==NULL) 	return 0;
  pend=xs->text + xs->len ;	
	k=lend - lens ;
	if (k<0) k=0;
	tmpbuf = (char *) malloc(  xs->len + k * 100 + 1  );
	if (tmpbuf==NULL) return -1;
	memset(tmpbuf, 0x00, xs->len + k * 100 );
	for (p0=p=xs->text, pt=tmpbuf; p && nrr<100 ; nrr++) {
		p=strstr(p0, scode);
		if (p==NULL || p>pend ) {
			memcpy(pt, p0, pend - p0);
			pt += pend - p0 ;
			break;
		}	
	  memcpy(pt, p0, p-p0); pt +=p-p0;
	  memcpy(pt, dcode, lend);  pt += lend ;
	  p0=p+lens ;
	}
	pt='\0';
	lens=strlen(tmpbuf) ;
	memcpy(xs->text, tmpbuf, lens   );
	xs->len = lens ;
	*(xs->text + lens)='\0';
	free(tmpbuf);
	return (nrr);
}

/*	XML代码符号转换: */
int XmlDataEnCode(XSTRING *xs )
{
	XEnCode(xs, "&lt;", "<");
	XEnCode(xs, "&gt;", ">");
	XEnCode(xs, "&quot;", "\"");
	XEnCode(xs, "&apos;", "'");
	XEnCode(xs, "&amp;", "&");
/* 为防止包中分隔符与业务系统冲突,非标准转换  */
	XEnCode(xs, "|", ":");
	return (0);
}

/*	XML代码符号反转换: */
int XmlDataUnCode(XSTRING *xs )
{
	XEnCode(xs, "&", "&amp;");
	XEnCode(xs, "<", "&lt;");
	XEnCode(xs, ">", "&gt;");
	XEnCode(xs, "\"", "&quot;");
	XEnCode(xs, "'", "&apos;");
	return (0);
}

/* 写xml头  */
char * XmlWriteHead(char *xmlbuf, char *encode)
{
	 sprintf(xmlbuf, "<?xml version=\"1.0\" encoding=\"%s\"?>\n\0",encode);
	 return (xmlbuf + strlen(xmlbuf) ); 
}

/*  写标识头 */
char * XmlWriteNodeBeg(char *xmlbuf, char *Tag)
{
	int k;

	if (*Tag>'9') {
	  sprintf(xmlbuf, "<%s>\n\0", Tag);
	  return (xmlbuf + strlen(Tag)+3 );
	} else {
		for (k=0; k < (*Tag - '0'); k ++) *(xmlbuf + k)='\t';
		sprintf(xmlbuf + k, "<%s>\n\0", Tag+1);
		return (xmlbuf + k+ strlen(Tag)+2  );
	}	
	  
}

/*  写标识尾 */
char * XmlWriteNodeEnd(char *xmlbuf, char *Tag)
{
	 int k ;
	 if (*Tag>'9') {
		 sprintf(xmlbuf, "</%s>\n\0", Tag);
		 return (xmlbuf + strlen(Tag)+4 ); 
		} else {
			for (k=0; k < (*Tag - '0'); k ++) *(xmlbuf + k)='\t';
			sprintf(xmlbuf+k, "</%s>\n\0", Tag+1);
			return (xmlbuf+k + strlen(Tag)+3 ); 
		}	 
}

/*  写数据元素 */
char * XmlWriteTag(char *xmlbuf, char *Tag, char *data )
{
	 int k ;
	 if (*Tag>'9') {
		 sprintf(xmlbuf, "\t\t<%s>%s</%s>\n\0", Tag, data, Tag);
		 return (xmlbuf + strlen(Tag)*2 + 8 + strlen(data)  ); 
		} else {
		 for (k=0; k < (*Tag - '0'); k ++) *(xmlbuf + k)='\t';
		 sprintf(xmlbuf+k, "<%s>%s</%s>\n\0", Tag+1, data, Tag+1);
		 return (xmlbuf + k + strlen(Tag)*2 + 4 + strlen(data)  ); 
		} 
}

/*  写扩展(需要符号转换)的数据元素 */
char * XmlWriteExTag(char *xmlbuf, char *Tag, char *data )
{
 	 XSTRING xs ;
   char xdata[512];  
 	 int len=strlen(data); 
	 int k ;

 	 xs.text =xdata ;   /* xmlbuf + strlen(Tag) +2  ;   */
 	 xs.len = len ;
	 memcpy( xs.text, data, len);   
 	 XmlDataUnCode( &xs );
 	 *(xs.text + xs.len) = '\0';
	 if (*Tag>'9') {
		 sprintf(xmlbuf, "\t\t<%s>%s</%s>\n\0", Tag, xs.text, Tag);
		 return (xmlbuf + strlen(Tag)*2 + 8 + xs.len  ); 
		} else {
		 for (k=0; k < (*Tag - '0'); k ++) *(xmlbuf + k)='\t';
		 sprintf(xmlbuf + k, "<%s>%s</%s>\n\0", Tag+1, xs.text, Tag+1);
		 return (xmlbuf + k + strlen(Tag)*2 + 4 + xs.len  ); 
		}	
}

/*  写注释 */
char * XmlWriteNote (char *xmlbuf, char *note)
{
	 sprintf(xmlbuf, "<!--%s-->\n\0", note );
	 return (xmlbuf + strlen(note)+8 ); 
}

/*  写任意串写缓冲 */
char * XWriteBuf (char *buf, char *str)
{
	 sprintf(buf, "%s\0", str );
	 return (buf + strlen(str) ); 
}

  
