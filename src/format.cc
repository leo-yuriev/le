/*
 * Copyright (c) 1993-1997 by Alexander V. Lukyanov (lav@yars.free.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*_____________________________________________________________________________
**
**  File:           format.cc
**  Description:    Format functions for text editor
**_____________________________________________________________________________
*/
#include <config.h>
#include <string.h>
#include <assert.h>
#include "edit.h"
#include <ctype.h>
#include "keymap.h"
#include "format.h"

int   LineLen=63;
int   LeftMargin=0;
int   FirstLineMargin=3;
int   LeftAdj=1;
int   RightAdj=0;
int   wordwrap=0;

void  FormatPara()
{
   num   bcol,bcol1,ncol;
   int   i;

   if(hex || View || buffer_mmapped) /* formatting is not allowed in those modes */
      return;

   flag=1;
   ToLineBegin();
   while(isspace(Char()) && !Eof())
   {
      ExpandTab();
      MoveRight();
   }
   if(Eof())
      return;

   /* fold the paragraph, that is delete all spaces but one and all newlines */
   for(;;)
   {
      while(Char()!=' ' && Char()!='\t' && !Eol())
         MoveRight();
      while((Char()==' ' || Char()=='\t') && !Eol())
         DeleteChar();
      if(Eol())
      {
         DeleteEOL();
         for(i=ncol=0; !Eol() && (Char()==' ' || Char()=='\t'); i++)
         {
            if(Char()=='\t')
               ncol=Tabulate(ncol);
            else
               ncol++;
            MoveRight();
         }
	 // end of paragraph condition:
	 //    empty line, or
	 //    large left margin (in case we ajust left)
         if(Eol() || (ncol>LeftMargin && LeftAdj))
         {
            while(i-->0)
               MoveLeft();
            break;
         }
         while(i-->0)
            BackSpace();   /* delete old indentation */
      }
      InsertChar(' ');
   }

   NewLine();
   stdcol=GetCol();
   MoveUp();

   if(LeftAdj)
   {
      /* create the first line margin */
      for(i=ncol=0; ncol<LeftMargin+FirstLineMargin && isspace(Char()) && !Eol(); i++)
      {
         if(Char()=='\t')
            ncol=Tabulate(ncol);
         else
            ncol++;
         MoveRight();
      }
      if(ncol<LeftMargin+FirstLineMargin)
      {
         while(!Bol())
            BackSpace();
         for(i=FirstLineMargin+LeftMargin; i>0; i--)
            InsertChar(' ');
      }
      else
      {
         while(ncol<LeftMargin+LineLen/2 && isspace(Char()) && !Eol())
         {
            if(Char()=='\t')
               ncol=Tabulate(ncol);
            else
               ncol++;
            MoveRight();
         }
         while(isspace(Char()) && !Eol())
            DeleteChar();
      }
   }
   else
   {
      while(isspace(Char()) && !Eol())
         DeleteChar();
   }

   for(;;)
   {
      if(GetCol()>LineLen+LeftMargin)
      {
         /* if the next word is over limit, then ... */
         while(!Bol() && !isspace(CharRel(-1)))
            MoveLeft();
         while(!Bol() && isspace(CharRel(-1)))   /* one word right */
            MoveLeft();
         if(Bol())
         {
            stdcol=0;   /* the word consumes the whole line */
            while(!Eol() && isspace(Char()))
               MoveRight();
            while(!Eol() && !isspace(Char()))
               MoveRight();
            if(!Eol())
	    {
               DeleteChar();   /* delete space after the word */
	       NewLine();
	       continue;
	    }
            break;
         }
         else
            DeleteChar();
         bcol1=bcol=GetCol();

	 assert(GetCol()<=LeftMargin+LineLen);

	 if(RightAdj && LeftAdj)
         {
            /* insert spaces to extend the line to the width */

            int gap_num=0;
            int spaces_to_insert=LineLen+LeftMargin-bcol;
            int i;

            if(spaces_to_insert>0)
            {
               /* To the line beginning, and count spaces */
	       while(!Bol())
               {
                  MoveLeft();
                  if(isspace(Char()))
                     gap_num++;
               }
	       /* skip indentation */
               while(isspace(Char()) && GetCol()<bcol)
               {
                  MoveRight();
                  gap_num--;
               }
               i=-gap_num/2;
               while(GetCol()<bcol)
               {
                  if(isspace(Char()))
                  {
                     MoveRight();
                     i+=spaces_to_insert;
                     while(i>0)
                     {
                        InsertChar(' ');
                        bcol++;
                        i-=gap_num;
                     }
                  }
                  else
                  {
                     MoveRight();
                  }
               }
            }
            NewLine();
         }
         else if(!LeftAdj && RightAdj)
         {
            ToLineBegin();
            while(bcol<LineLen+LeftMargin)
            {
               InsertChar(' ');
               bcol++;
            }
            GetCol();
            while(GetCol()<bcol)
               MoveRight();
            NewLine();
         }
         else if(!LeftAdj && !RightAdj)
         {
            GetCol();
            while(GetCol()<bcol)
               MoveRight();
            NewLine();
            MoveUp();
            CenterLine();
            MoveDown();
         }
         else
         {
            NewLine();
         }
         for(i=LeftMargin; i>0; i--)
            InsertChar(' ');
      }
      else
      {
	 if(Eol())
	    break;   /* this is the end of line and the paragraph */
         MoveRight();
      }
   }
   if(!LeftAdj && RightAdj)
   {
      bcol=GetCol();
      ToLineBegin();
      while(bcol<LineLen+LeftMargin)
      {
         InsertChar(' ');
         bcol++;
      }
      GetCol();
      while(GetCol()<bcol)
         MoveRight();
   }

   ToLineBegin();
   stdcol=GetCol();
   MoveDown();
}

void  FormatAll()
{
   static  struct  menu FAmenu[]={
   {"   &Ok   ",MIDDLE-6,FDOWN-2},
   {" &Cancel ",MIDDLE+6,FDOWN-2},
   {NULL}};

   if(hex || View)
      return;

   switch(ReadMenuBox(FAmenu,HORIZ,"ALL text will be formatted (no undo)",
      " Verify ",VERIFY_WIN_ATTR,CURR_BUTTON_ATTR))
   {
   case(0):
   case('C'):
      return;
   }
   MessageSync("Formatting all document...");
   TextPoint oldpos=CurrentPos;
   CurrentPos=TextBegin;
   while(!Eof())
      FormatPara();
   CurrentPos=oldpos;
}

void  CenterLine()
{
   num shift;
   if(hex || View)
      return;
   flag=REDISPLAY_LINE;
   ToLineBegin();
   while((Char()==' ' || Char()=='\t') && !Eol())
      DeleteChar();
   if(Eol())
      return; /* nothing to center */
   ToLineEnd();
   while(isspace(CharRel(-1)))
      BackSpace();
   shift=(LineLen-GetCol())/2+LeftMargin;
   if(shift>0) /* not too long line */
   {
      ToLineBegin();
      while(shift--)
	 InsertChar(' ');
   }
   ToLineBegin();
   stdcol=GetCol();
}

void  ShiftRightLine()
{
   num shift;
   if(hex || View)
      return;
   flag=REDISPLAY_LINE;
   ToLineBegin();
   while((Char()==' ' || Char()=='\t') && !Eol())
      DeleteChar();
   if(Eol())
      return; /* nothing to shift */
   ToLineEnd();
   while(isspace(CharRel(-1)))
      BackSpace();
   shift=(LineLen-GetCol())+LeftMargin;
   if(shift>0) /* not too long line */
   {
      ToLineBegin();
      while(shift--)
	 InsertChar(' ');
   }
   ToLineBegin();
   stdcol=GetCol();
}

void  FormatFunc()
{
   int   action;

   if(hex || View)
      return;
   ToLineBegin();
   stdcol=GetCol();
again:
   ClearMessage();
   CenterView();
   SyncTextWin();
   StatusLine();
   Message("Format: F-Format all P-format Paragraph C-Center line R-align Right");
   SetCursor();

   action=GetNextAction();
   switch(action)
   {
   case(LINE_UP):
       UserLineUp();
       goto again;
   case(LINE_DOWN):
       UserLineDown();
       goto again;
   case(REFRESH_SCREEN):
       UserRefreshScreen();
       break;
   default:
      if(StringTypedLen!=1)
         break;
      switch(StringTyped[0])
      {
      case('P'):
      case('p'):
         MessageSync("Formatting one paragraph...");
         FormatPara();
         RedisplayAll();
         goto again;
      case('R'):
      case('r'):
      {
         MessageSync("Shifting right...");
	 ShiftRightLine();
	 UserLineDown();
         RedisplayLine();
         goto again;
      }
      case('C'):
      case('c'):
         MessageSync("Centering...");
         CenterLine();
         UserLineDown();
         RedisplayLine();
         goto again;
      case('F'):
      case('f'):
         FormatAll();
         flag=REDISPLAY_ALL;
         break;
      }
   }
}

void WordWrapInsertHook()
{
   if(GetCol()<LineLen+LeftMargin)
      return;
   offs pos=CurrentPos;
   while(!BolAt(pos) && !(CharAt(pos-1)==' ' || CharAt(pos-1)=='\t'))
      pos--;
   if(pos==CurrentPos)
      return;
   offs word_begin=pos;
   while(!BolAt(pos) && (CharAt(pos-1)==' ' || CharAt(pos-1)=='\t'))
      pos--;
   if(BolAt(pos))
      return;
   TextPoint old(CurrentPos);
   CurrentPos=word_begin;
   DeleteBlock(word_begin-pos,0);
   NewLine();
   for(int i=LeftMargin; i>0; i--)
      InsertChar(' ');
   CurrentPos=old;
   stdcol=GetCol();
}
