

/*
 * Public entry into 'play.obj' module.
 * See GWBasic or QBasic documentation for format of 'cszPlay'.
 *
 * Following an excerpt from QBasic help file:
 *
 * Topic:  PLAY Statement (Music) 
 *
 * Plays musical notes.
 * 
 * PLAY commandstring$
 *
 *   þ commandstring$    A string expression that contains one or more of
 *                        the following PLAY commands:
 *
 *      Octave and tone commands:
 *        Ooctave    Sets the current octave (0 - 6).
 *        < or >     Moves up or down one octave.
 *        A - G      Plays the specified note in the current octave.
 *        Nnote      Plays a specified note (0 - 84) in the seven-octave
 *                   range (0 is a rest).
 + <<< Nnote is currently not supported! >>> +
 *
 *      Duration and tempo commands:
 *        Llength    Sets the length of each note (1 - 64). L1 is whole note,
 *                   L2 is a half note, etc.
 *        ML         Sets music legato.
 *        MN         Sets music normal.
 *        MS         Sets music staccato.
 *        Ppause     Specifies a pause (1 - 64). P1 is a whole-note pause,
 *                   P2 is a half-note pause, etc.
 *        Ttempo     Sets the tempo in quarter notes per minute (32 - 255).
 *
 *      Mode commands:
 *        MF          Plays music in foreground.
 *        MB          Plays music in background.
 *
 *      Suffix commands:
 *        # or +      Turns preceding note into a sharp.
 *        -           Turns preceding note into a flat.
 *        .           Plays the preceding note 3/2 as long as specified.
 *
 */
void _System BasicPlay(char const * cszPlay,char volatile *pfbAbort);

