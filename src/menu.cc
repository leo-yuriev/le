/*
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <config.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include "edit.h"
#include "keymap.h"

char  ItemChar(char *i)
{
   for(; *i; i++)
      if(*i=='&')
         return(toupper(i[1]));
   return(0);
}
int   ItemLen(char *i)
{
   int   len=0;
   for(; *i; i++)
      if(*i!='&' || !i[1])
         len++;
   return(len);
}
void  DisplayItem(int x,int y,char *i,attr *a)
{
   Absolute(&x,ItemLen(i),Upper->w);
   attr r=*a;
   r.attr=a->so_attr;
   for(; *i; i++)
   {
      if(*i=='&')
      {
	 SetAttr(&r);
         PutCh(x++,y,(byte)(*++i));
      }
      else
      {
	 SetAttr(a);
         PutCh(x++,y,(byte)(*i));
      }
   }
   SetAttr(a);
}

void  display(struct menu *mi,attr *a)
{
   DisplayItem(mi->x,mi->y,mi->text,a);
}

int   ReadMenu(struct menu *m,int dir,attr *a,attr *ca)
{
   int   curr = 0;
   int   i,action,key;

   curs_set(0);
   do
   {
      for(i=0; m[i].text; i++)
      {
         if(i!=curr)
            display(&m[i],a);
         else
            display(&m[i],ca);
      }
      move(LINES-1,COLS-1);
      action=GetNextAction();
      switch(action)
      {
      case(CHAR_LEFT):
         if(dir==HORIZ)
         {
	    if(curr==0)
               while(m[curr].text)
                  curr++;
            curr--;
         }
         break;
      case(CHAR_RIGHT):
         if(dir==HORIZ)
         {
right:         curr++;
            if(m[curr].text==NULL)
               curr=0;
         }
         break;
      case(LINE_UP):
         if(dir==VERT)
         {
            if(curr==0)
               while(m[curr].text)
                  curr++;
            curr--;
         }
         break;
      case(LINE_DOWN):
         if(dir==VERT)
         {
            curr++;
            if(m[curr].text==NULL)
               curr=0;
         }
         break;
      case(CANCEL):
         return(0);
      case(NEWLINE):
         return(ItemChar(m[curr].text));
      default:
         if(StringTypedLen!=1)
            break;
         if(StringTyped[0]==9)
            goto right;
         key=toupper(StringTyped[0]);
         for(i=0; m[i].text; i++)
            if(key==ItemChar(m[i].text))
               return(key);
      }
   }
   while(1);
/*NOTREACHED*/
}

void  GetTextGeometry(const char *s,int *w,int *h)
{
   int	 x;

   *w=0;
   *h=0;
   while(*s)
   {
      (*h)++;
      x=0;
      while(*s && *s!='\n')
	 x++,s++;
      if(*s)
	 s++;
      if(x>*w)
	 *w=x;
   }
}

int   ReadMenuBox(struct menu *m,int dir,const char *msg,const char *title,
		  attr *a,attr *a1)
{
   int	 w,h;
   int	 len;
   int	 pos;
   int	 i;

   GetTextGeometry(msg,&w,&h);

   if(dir==HORIZ)
   {
      len=-2;
      for(i=0; m[i].text; i++)
      {
	 len+=ItemLen(m[i].text)+2;
	 m[i].y=FDOWN-2;
      }
      if(len>w)
	 w=len;
      if(w<COLS/4)
	 w=COLS/4;
      if(h>0)
	 h+=2;
      else
	 h+=1;
      pos=(w-len)/2;
      for(i=0; m[i].text; i++)
      {
	 m[i].x=pos+2;
	 pos+=ItemLen(m[i].text)+2;
      }
   }
   else
      abort();

   WIN	 *win=CreateWin(MIDDLE,MIDDLE,w+4,h+4,a,title);
   DisplayWin(win);
   PutStr(2,2,msg);
   int res=ReadMenu(m,dir,a,a1);
   CloseWin();
   DestroyWin(win);

   return(res);
}
