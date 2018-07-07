/*
 * $Source$
 * $Revision$
 * $Date$
 * $Locker$
 *
 *	Schnittstelle zu bmp.c
 *
 * $Log$
 * ----------------------------------------
 */

typedef struct {
    short	cx, cy;
    short	colors;
    void	data;
} IMAGE, * PIMAGE;

int	LoadBmp(char const * filename, PIMAGE image);
