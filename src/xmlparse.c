/* Copyright (c) 2005 ����ʯwzs  */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <varargs.h>
#include "xmlparse.h"
/*xml�õ��ڵ���ֵ, ����: 1�ڵ��ʶ����ͷ, 2�ڵ��ʶ����β, 3 ע�� , 4Ԫ������  <=0�д��� */
#define XVAL_NBEGIN 	1
#define XVAL_NEND 		2
#define XVAL_NOTE			3
#define XVAL_DATA 		4
#define XVAL_TAG 			5      /* �ձ�־ */
#define XVAL_NONE			0
#define XVAL_ERROR		-1

/*  ��ǰ����ʱ��  */
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

/* д�ļ� */
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

/* д������Ϣ  */
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

/* д������Ϣ  */
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

int isSpace(int c)   /*  �ǿշ�  */
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

int isNameTChar(int c)   /* ��Ч������ǰ����  */
{
	return (c>='A' && c<='Z') || (c>='a' && c<='z') || (c=='_') ;  
}

int EqXString (XSTRING *dxs, XSTRING *sxs )    /*  X�ִ���ֵ  */
{
   dxs->text = sxs->text ;
   dxs->len = sxs->len ;
   return 0;
}

int EqXNode (XNODE *dx, XNODE *sx )    /*  X�ڵ㸳ֵ  */
{
   EqXString ( &(dx->name), &(sx->name) ); 
   dx->index = sx->index ;
   dx->parentNode = sx->parentNode ;
   dx->level = sx->level ;
   dx->tagCount = sx->tagCount ;
   EqXString ( &(dx->data), &(sx->data) );  
   return 0;
}


/*	����ͬ���ڵ�:   */
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

/* 	�����ӽڵ�:   */
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

/*	���ظ��ڵ�:   */
XNODE *XRetParentNode(XTREE *xTree )
{
   if (xTree->curNode->parentNode == NULL ) return NULL;
   xTree->preNode = xTree->curNode ;
   xTree->curNode = xTree->curNode->parentNode ;
   return (xTree->curNode);
}

/*	�޸ĵ�ǰ�ڵ�Ԫ��ֵ: */
int XElementData(XTREE *xTree , XSTRING *data)
{
   if (xTree->curNode == NULL ) return -1 ;
 /*  if (xTree->curNode->tagCount>0) return -2; */
   EqXString ( &(xTree->curNode->data), data) ;  
   return 0;
}

char * DelSpace( char *xbuf)    /* ɾ��ǰ���� */ 
{
	  char *p ;
	  for (p=xbuf ; isSpace(*p) ; p++);   /* ȥ�����ַ� */
	  return (p);
}

int XmlAttr( XTREE *xTree, XSTRING *val)  /* ��������: ����Ҷ�ڵ�, �ӽڵ���Ϊ-1, �������ӵ����Խڵ��� */
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
	    /* ��������ֵ�ڵ�  */
	    if (XAddSubNode(xTree, &xn)==NULL) break ;
	    k++;	
	    if (XElementData(xTree , &xv) <0 ) break ;
	    xTree->curNode->tagCount = -1;
	    XRetParentNode( xTree )	;
	    p=DelSpace(p2+1);
		}
		return (k);
}

/*  xml�õ��ڵ���ֵ, ����: 1�ڵ��ʶ����ͷ, 2�ڵ��ʶ����β, 3 ע�� , 4Ԫ������  0��  -1�д��� */
int XmlReadNodeVal(XTREE *xTree, XSTRING *val )
{
    char *p ,*p1, *p2;
    int ivtype=XVAL_NONE ;
    p=xTree->xBufPos ;
    p=DelSpace( xTree->xBufPos)  ;   /* ɾ��ǰ���� */ 
    if (*p !='<') {     /* ���� */
    	p1 = p;
    	p2=strchr(p1, '<'); 
    	val->text = p1;
    	val->len  = ( p2 ? p2-p1 : 0);	
    	ivtype = ( p2 && p2 < xTree->xBufEnd ?  XVAL_DATA : XVAL_NONE) ; 
    	xTree->xBufPos = (p2 ? p2 : xTree->xBufEnd);
    	goto OKExit33qqq ;	
    }
    
    /*  ǰ��Ϊ<  */
     p++;
     if ( *p=='/' )   {    /* ���β */
      	p1 = p+1;
      	p2=strchr(p1, '>'); 
      	if (p2==NULL) {  /*  ���� */
      		 goto ErrExit33qqq ;
      	} else {
      		ivtype=XVAL_NEND;
      		val->text = p1;
      		val->len = p2-p1;
      		xTree->xBufPos = p2 +1  ; 
        }
    	}
    	else if (isNameTChar(*p))  {   /* ���ͷ  */
 				p1=p;
 				p2=strchr(p1,'>');
      	if (p2==NULL) {  /*  ���� */
      		 goto ErrExit33qqq ;
      	} 				
 				for (p=p1; *p!=' ' && p<p2; p++);
 				if (*(p2-1)=='/') {  /* ����ձ��<abc/> */
 					ivtype=11;
 					val->text = p1;
 					if (p>=p2) p=p2-1;
 					val->len = p - p1;
 					if (	XAddSubNode( xTree , val ) == NULL) goto ErrExit33qqq ;
 					if (p < p2-1 ) /*  �������Խڵ�  */ 
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
          if (p < p2)    /*  �������Խڵ�  */ 
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
      	if (*(p+1)=='-' &&  *(p+2)=='-')   /* ע�� */
      	{ 
      		p+=3;  
      		p1=DelSpace(p);
      		p2=strstr(p1, "-->");   /*ע��β */
      		if (p2==NULL) {  /*  ���� */
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

/*	��xml��������X�� */
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
	  	XPrtErr("��XML����ͷ!");
	  	return -1;
		}	
	  p=strstr(xmlbuf, "?>")	;
	  if (p==NULL) 
	  {	
	  	XPrtErr("XML����ͷ�޽�β!");
	  	return -1;
		}	

	  p1=DelSpace(p+2);
	  for (p=p1, maxnn=1; p<pend ; p++) {
		  if (*p=='>' || *p=='"') maxnn++;
		  if (*p=='>') if (*(p-1) == '/')  maxnn++;  /*  �ձ��  */ 
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
    		XPrtDebug("��XML����");
    		return 0;
  	}
  	
  	XPrtDebug("===����ڵ�洢�ռ�=%d ", maxnn);
	  xTree->nodeList = (XNODE *) calloc( maxnn , sizeof(XNODE) );   /* ����ڵ�洢�ռ� */
	  if (xTree->nodeList == NULL ) 
	  {
	  	XPrtErr("����ڵ�洢�ռ�(%d)ʧ��!", maxnn );
	  	return -2;
	  }	
	  
	  xTree->nodeMaxCount = maxnn ;
	  memset( xTree->nodeList , 0x00, maxnn * sizeof(XNODE) );
		xTree->rootNode = &(xTree->nodeList[0]);   /* ���ڵ� */
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
						XRetParentNode( xTree ) ;  		  /*	���ظ��ڵ�:   */
						break;
				case XVAL_DATA :
						XElementData( xTree , &xstr ) ;	/* 	�޸ĵ�ǰ�ڵ�Ԫ��ֵ: */
						break;
				default :
					  if (vtype>10)	 break;  /* printf(" ===c����%d\n",  vtype -10 ); */
    	}
    }
  *xmlbuf = '\0';   /* ���ѽ�����־  */  
  maxnn = xTree->curNode->index ;   /* ���յ�ǰ�ڵ�� */   
	for (k=0; k<xTree->nodeCount ; k++){
		xn =&( xTree->nodeList[k] );
		if (xn->name.text)	*(xn->name.text + xn->name.len) ='\0'; else xn->name.text=xmlbuf ;
		if (xn->data.text) *(xn->data.text + xn->data.len) ='\0'; else xn->data.text=xmlbuf ;  
	}
  XPrtDebug	 ( " �ڵ���=%d\n", xTree->nodeCount);
  if (maxnn) XPrtErr("XML�ļ�������(%d) ", maxnn );
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
	XPrtDebug("   ********  xTree �ṹ, �ܽڵ���=%d, �ڵ�ռ�=%d  ********  ", xTree->nodeCount, xTree->nodeMaxCount);
	XPrtDebug("\t�ڵ��	���ڵ�	�ڵ㼶	�ӽ���	�ڵ���	�ڵ�����");
	for (k=0; k<xTree->nodeCount ; k++){
		xn =&( xTree->nodeList[k] );
		XPrtDebug("\t%d	%d	%d	%d	[%s]	[%s]",xn->index,(xn->parentNode ? xn->parentNode->index : -1),  xn->level, xn->tagCount,xn->name.text, xn->data.text );
	}
		XPrtDebug("*****************************************************************");
    return 0;
} 

/*	�ӵ�ǰ�ڵ�����ȡ�ýڵ�: */
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

/*		�ӵ�ǰ�ڵ�����ȡ���ӽڵ�(���ݸ��ڵ����ͽڵ���): */
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

/*		�ӵ�ǰ�ڵ�����ȡ���ӽڵ�(���ݸ��ڵ�ͽڵ���): */
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

/*	ȡ��Ҷ�ڵ�: */
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

/*	�������ת��1: */
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

/*	XML�������ת��: */
int XmlDataEnCode(XSTRING *xs )
{
	XEnCode(xs, "&lt;", "<");
	XEnCode(xs, "&gt;", ">");
	XEnCode(xs, "&quot;", "\"");
	XEnCode(xs, "&apos;", "'");
	XEnCode(xs, "&amp;", "&");
/* Ϊ��ֹ���зָ�����ҵ��ϵͳ��ͻ,�Ǳ�׼ת��  */
	XEnCode(xs, "|", ":");
	return (0);
}

/*	XML������ŷ�ת��: */
int XmlDataUnCode(XSTRING *xs )
{
	XEnCode(xs, "&", "&amp;");
	XEnCode(xs, "<", "&lt;");
	XEnCode(xs, ">", "&gt;");
	XEnCode(xs, "\"", "&quot;");
	XEnCode(xs, "'", "&apos;");
	return (0);
}

/* дxmlͷ  */
char * XmlWriteHead(char *xmlbuf, char *encode)
{
	 sprintf(xmlbuf, "<?xml version=\"1.0\" encoding=\"%s\"?>\n\0",encode);
	 return (xmlbuf + strlen(xmlbuf) ); 
}

/*  д��ʶͷ */
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

/*  д��ʶβ */
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

/*  д����Ԫ�� */
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

/*  д��չ(��Ҫ����ת��)������Ԫ�� */
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

/*  дע�� */
char * XmlWriteNote (char *xmlbuf, char *note)
{
	 sprintf(xmlbuf, "<!--%s-->\n\0", note );
	 return (xmlbuf + strlen(note)+8 ); 
}

/*  д���⴮д���� */
char * XWriteBuf (char *buf, char *str)
{
	 sprintf(buf, "%s\0", str );
	 return (buf + strlen(str) ); 
}

  
