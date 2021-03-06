/*
 * $Source: e:/source/tools/play.c,v $
 * $Author: vitus $
 * $Date: 1995/12/16 20:43:57 $
 * $Revision: 1.1 $
 *
 * Simulation of GWBasic PLAY command to OS/2 applications.
 *
 * BUGS:
 *	Duration of notes and breaks between notes isn't optimal, there
 *	are differences between 'normal', 'legato' and 'staccato' modes.
 *	This isn't anything I can't fix with time and a small testprogram,
 *	though.
 *
 * $Log: play.c,v $
 * Revision 1.1  1995/12/16 20:43:57  vitus
 * Initial revision
 *
 */
#if defined(__WATCOMC__)
# pragma off (unreferenced)
#endif
static char vcid[] = "$Id: play.c,v 1.1 1995/12/16 20:43:57 vitus Exp $";
#if defined(__WATCOMC__)
# pragma off (unreferenced)
#endif


#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>

#define INCL_DOS
#include <os2.h>

#include "play.h"				/* check prototype */

/*
 * Defaults for some values. Check with GWBasic Documentation!
 */
#define DEF_Length	4
#define DEF_Speed	120
#define DEF_Octave	0
#define DEF_Mode	NORMAL

#define MAX_Octave	6			/* highest supported octave */



typedef enum _MODUS { NORMAL, STACCATO, LEGATO }		MODUS;
typedef enum _TOKVAL { INV, OCT, LEN, BEAT, MOD, NOTE, PAUSE }	TOKVAL;


typedef struct _TOKEN {
    TOKVAL	kind;
    UCHAR	value;
    UCHAR	note_index;
    UCHAR	flags;				/* hack:
						   BIT0 = 3/2 * spec. length */
} TOKEN;



/* Define table of frequencies for highest octave. Devide
 * by power of 2 to get actual frequency.
 * OBS: define c-flat and c to handle "wrap around"!
 */
static unsigned short aIdx2Freq[] = {
    7920,					/* 0: c-flat 6 */
    8448,					/* 1: c 6 */
    8800,					/* 2: d-flat 6 */
    9504,					/* 3: d 6 */
    10144,					/* 4: e-flat 6 */
    10560,					/* 5: e 6 */
    11264,					/* 6: f 6 */
    11744,					/* 7: g-flat 6 */
    12672,					/* 8: g 6 */
    13504,					/* 9: a-flat 6 */
    14080,					/* 10: a 6 */
    15200,					/* 11: b-flat 6 */
    15840,					/* 12: b 6 */
    16896					/* 13: c 7 */
};


/* There is no way to handle this by a formular */
UCHAR aChar2Noteindex[] = { 
    10,						/* a */
    12,						/* b */
    1,						/* c */
    3,						/* d */
    5,						/* e */
    6,						/* f */
    8						/* g */
};

    


/* Let **csz always point to next not-analyzed character! */
static int GetNote(char const **csz,TOKEN *tk)
{
    int val_allowed = 0;
    char c;

    tk->kind = INV;
    tk->value = (UCHAR)-1;
    tk->flags = 0;

    c = *((*csz)++);
    c = tolower(c);
	
    switch( c )
    {
      case 'c':
      case 'd':
      case 'e':
      case 'f':
      case 'g':
      case 'a':
      case 'b':				/* _not_ 'h'! */
	tk->kind = NOTE;
	tk->note_index = aChar2Noteindex[c-'a'];
	val_allowed = 1;
	switch( **csz )
	{
	  case '#':
	  case '+':
	    ++tk->note_index;
	    ++(*csz);
	    break;
	  case '-':
	    --tk->note_index;
	    ++(*csz);
	    break;
	  default:
	    break;				/* ignore */
	}
	break;

	/* PAUSE */
      case 'p':
	tk->kind = PAUSE;
	val_allowed = 1;
	break;

	/* new beat */
      case 't':
	tk->kind = BEAT;
	val_allowed = 1;
	break;

	/* new default length */
      case 'l':
	tk->kind = LEN;
	val_allowed = 1;
	break;

	/* new default octave */
      case 'o':
	tk->kind = OCT;
	val_allowed = 1;
	break;

	/* new default mode */
      case 'm':
	tk->kind = MOD;
	switch( **csz )
	{
	  case 'n':
	  case 'N':
	    tk->value = (char)NORMAL;
	    ++(*csz);
	    break;
	  case 'l':
	  case 'L':
	    tk->value = (char)LEGATO;
	    ++(*csz);
	    break;
	  case 's':
	  case 'S':
	    tk->value = (char)STACCATO;
	    ++(*csz);
	    break;
	  default:
	    tk->kind = INV;			/* clear token */
	    break;
	}
	break;

      default:
	break;
    }

    if( tk->kind == INV )
	return 1;				/* end of parsing */
    if( val_allowed  &&  isdigit(**csz) )
    {
	char buf[20];
	int  i;

	for( i = 0; i < 20; ++i )
	{
	    if( isdigit(**csz) )
	    {
		buf[i] = **csz;
		++(*csz);
	    }
	    else
		break;
	}
	buf[i] = '\0';				/* I know it's not correct,
						   but 20 chars aren't either */
	tk->value = atoi(buf);
	if( **csz == '.' )
	{
	    tk->flags |= 0x01;
	    ++(*csz);
	}
    }
    return 0;
}



/* Return time in milliseconds to play this tone */
static ULONG CalcToneLength(UCHAR speed,MODUS modus,TOKEN *token)
{
    double rc;
    double beat = 60000. / speed * 4.;		/* full note in ms */
    
    rc = beat / token->value;			/* whole duration of this note */
    switch( modus )
    {
      case NORMAL:
	rc *= 0.9;
	break;
      case STACCATO:
	rc *= 0.75;
	break;
      case LEGATO:
	rc *= 1.1;
	break;					/* inget mellanrum */
    }
    if( token->flags & 0x01 )			/* '.' added? */
	rc *= 1.5;
    return ((ULONG)rc == 0 ? 1 : (ULONG)rc);
}




/* Return time in milliseconds to wait before next tone */
static ULONG CalcBreakLength(UCHAR speed,MODUS modus,TOKEN *token)
{
    double rc;
    double beat = 60000. / speed * 4.;		/* full note in ms */
    
    rc = beat / token->value;			/* whole duration of this note */
    switch( modus )
    {
      case NORMAL:
	rc *= 0.1;
	break;
      case STACCATO:
	rc *= 0.25;				/* .30 better than .25? Why? */
	break;
      case LEGATO:
	rc = 0.;					/* inget mellanrum */
	break;
    }
    if( token->flags & 0x01 )			/* '.' added? */
	rc *= 1.5;
    return (ULONG)rc;
}





/*
 * Public entry into this module.
 * See GWBasic or QBasic documentation for format
 * of 'cszPlay' or 'play.h'
 */
void _System BasicPlay(char const * cszPlay,char volatile *pfbAbort)
{
    UCHAR octave = DEF_Octave;
    UCHAR speed = DEF_Speed;
    UCHAR length = DEF_Length;
    MODUS modus = DEF_Mode;

    char const *cp = cszPlay;
    TOKEN	token;

    assert( cszPlay != NULL );
    assert( pfbAbort != NULL );
    for(; *pfbAbort == 0  &&  GetNote(&cp, &token) == 0 ;)
    {
	switch( token.kind )
	{
	  case MOD:
	    modus = (MODUS)token.value;
	    break;

	  case BEAT:
	    speed = token.value;
	    break;

	  case LEN:
	    length = token.value;
	    break;

	  case OCT:
	    octave = token.value;
	    break;

	  case PAUSE:
	    if( token.value == (UCHAR)-1 )
		token.value = (UCHAR)length;
	    DosSleep( CalcToneLength(speed, modus, &token)
		     + CalcBreakLength(speed, modus, &token) );
	    break;

	  case NOTE:
	    {
		ULONG freq = aIdx2Freq[token.note_index]
		    / pow(2,MAX_Octave-octave);
		if( token.value == -1 )
		    token.value = length;
		DosBeep( freq, CalcToneLength(speed, modus, &token) );
		DosSleep( CalcBreakLength(speed, modus, &token) );
	    }
	    break;

	  default:
	    break;
	}
    }
}

