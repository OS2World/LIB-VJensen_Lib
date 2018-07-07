/*
 * $Source: e:/source/tools/willtell.c,v $
 * $Revision: 1.1 $
 * $Date: 1996/01/13 14:24:48 $
 * $Author: vitus $
 *
 * Sample application for PLAY/2 package.
 *
 * $Log: willtell.c,v $
 * Revision 1.1  1996/01/13 14:24:48  vitus
 * Initial revision
 *
 */
static char vcid[]="$Id: willtell.c,v 1.1 1996/01/13 14:24:48 vitus Exp $";

#include <assert.h>
#include <stdio.h>
#include <process.h>

#define INCL_DOS
#include <os2.h>

#include "play.h"



char const szWilli[] = "L16T155"
	"o2mnb4p8msbbmnb4p8msbbb8g#8"
	"e8g#8b8g#8b8o3e8o2b8g#8e8g#8"
	"b8g#8b8o3e8o2mnb4p8msbbmnb4"
	"p8msbbmnb4p8msbbmnb4p8msbb"
	"b8bbb8b8b8bbb8b8b8bb"
	"b8b8b8bbb8b8mlb2"
	"b2b8p8p4p4"
	"p8mso1bbb8bbb8bbo2e8f#8g#8o1bb"
	"b8bbo2e8g#g#f#8d#8o1b8bbb8bb"
	"b8bbo2e8f#8g#8eg#mlb4bmsag#f#"
	"e8g#8e8o3bbb8bbb8bbo4e8f#8"
	"g#8o3bbb8bbo4e8g#g#f#8d#8o3b8bb"
	"b8bbb8bbo4e8f#8g#8mleg#b4"
	"bag#f#mse8g#8e8o3g#g#g#8g#g#g#8g#g#"
	"g#8o4c#8o3g#8o4c#8o3g#8o4c#8o3g#8f#8e8d#8"
	"c#8g#g#g#8g#g#g#8g#g#g#8o4c#8o3g#8o4c#8"
	"o3g#8o4c#8o3b8a#8b8a#8b8g#g#g#8g#g#"
	"g#8g#g#g#8o4c#8o3g#8o4c#8o3g#8o4c#8o3g#8f#8"
	"e8d#8c#8g#g#g#8g#g#g#8g#g#g#8o4c#8"
	"o3g#8o4c#8o3g#8o4c#8o3b8a#8b8o2bbb8f#f#"
	"f#8f#f#f#8g#8a8f#4mna8msg#8mne4"
	"msg#8f#8f#8f#8o3f#f#f#8f#f#f#8g#8"
	"a8mnf#4msa8g#8mne4msg#8f#8o2bb"
	"b8o1bbb8bbb8bbo2mne8f#8g#8o1bb"
	"b8bbo2e8g#g#f#8d#8o1b8bbb8bb"
	"b8bbo2e8f#8g#8eg#mlb4mnbag#f#"
	"e8g#8e8o3bbb8bbb8bbo4e8f#8"
	"g#8o3bbb8bbo4e8g#g#f#8d#8o3b8bb"
	"b8bbb8bbo4e8f#8g#8mleg#mlb4"
	"mnbag#f#mne8g#8e8o3mle56f56g56a56b56o4c56d56mne8eee8e8g#4."
	"f#8e8d#8e8c#8mso3bo4c#o3bo4c#o3b"
	"o4c#d#eo3abababo4c#d#o3g#ag#ag#abo4c#o3f#"
	"g#f#g#f#g#f#g#f#g#f#d#o2bo3mlbo4c#d#e8d#8e8"
	"c#8o3msbo4c#o3bo4c#o3bo4c#d#eo3abababo4c#d#o3g#"
	"ag#ag#abo4c#o3f#g#f#g#f#af#emne8p8mlc#4"
	"mnc#o2cmso3c#o2co3d#c#o2baag#ec#c#c#c#c#e"
	"d#o1cg#g#g#g#g#g#o2c#eg#o3c#c#c#c#c#o2co3c#o2co3d#"
	"c#o2baag#ec#c#c#c#c#ed#o1cg#g#g#g#g#mng#"
	"o2c#eg#o3msc#ed#c#d#o2cg#g#g#o3g#ec#d#o2cg#g#g#"
	"o3g#ec#d#o2bg#g#a#gd#d#g#gg#gg#ag#f#e"
	"o1ba#bo2eo1bo2f#o1bo2g#ed#eg#eaf#bo3g#f#ed#"
	"f#ec#o2bo3c#o2bo3c#d#ef#g#o2ababo3c#d#ef#o2g#"
	"ag#aco3c#d#eo2f#g#f#g#f#g#f#g#f#g#f#d#o1b"
	"co2c#d#eo1ba#bo2eo1bo2f#o1bo2g#ed#eg#eaf#b"
	"o3g#f#ed#f#ec#o2bo3c#o2bo3c#d#ef#g#o2ababo3c#"
	"d#ef#o2g#ag#abo3c#d#eo2f#o3c#o2co3c#d#c#o2af#mne"
	"o3mlef#g#abo4c#d#mne8mseee8e8g#4."
	"msf8mse8d#8e8c#8o3bo4c#o3bo4c#o3bo4c#d#eo3a"
	"bababo4c#d#o3g#ag#ag#abo4c#o3f#g#f#g#f#"
	"g#f#g#f#g#f#d#o2bo3mlbo4c#d#mne8eee8e8g#4."
	"msf#8e8d#8e8c#8o3bo4c#o3bo4c#o3b"
	"o4c#d#eo3abababo4c#d#o3g#ag#ag#abo4c#o3f#"
	"g#f#g#f#ag#f#e8o2b8o3e8g#g#g#8mng#g#g#8"
	"g#g#g#8o4c#8o3g#8o4c#8o3g#8o4c#8o3g#8f#8e8"
	"d#8c#8g#g#g#8g#g#g#8g#g#g#8o4c#8o3g#8"
	"o4c#8o3g#8o4c#8o3b8a#8b8a#8b8g#g#g#8"
	"g#g#g#8g#g#g#8o4c#8o3g#8o4c#8o3g#8o4c#8o3g#8"
	"f#8e8d#8c#8g#g#g#8g#g#g#8g#g#g#8"
	"o4c#8o3g#8o4c#8o3g#8o4c#8o3b8a#8b8a#8b8"
	"o2f#f#f#8f#f#f#8g#8a8f#4a8g#8"
	"e4g#8f#8o0b8o1b8o2f#f#f#8f#f#f#8"
	"g#8a8f#4a8g#8e4g#8f#8"
	"bbb8o1bbb8bbb8bbo2e8f#8g#8"
	"o1bbb8bbo2e8g#g#f#8d#8o1b8bbb8"
	"bbb8bbo2e8f#8g#8eg#mlb4mnb"
	"ag#f#e8o1b8o2e8o3bbb8bbb8bbo4e8"
	"f#8g#8o3bbb8bbo4e8g#g#f#8d#8o3b8"
	"bbb8bbb8bbo4e8f#8g#8o3eg#mlb4"
	"mnbag#f#mlef#g#mnamlg#abo4mnc#mlo3bo4c#d#mnemld#"
	"ef#mng#ao3bo4ao3bo4ao3bo4ao3bo4ao3bo4ao3bo4ao3bo4ao3bmle"
	"f#g#mnamlg#abmno4c#mlo3bo4c#d#mnemld#ef#mng#ao3bo4ao3bo4a"
	"o3bo4ao3bo4ao3bo4ao3bo4ao3bo4ao3bp16mlg#o4g#o3mng#p16mld#o4d#o3mnd#p16"
	"mleo4eo3mnep16mlao4ao3mnap16mlg#o4g#o3mng#p16mld#o4d#o3mnd#p16mleo4eo3mnep16"
	"mlao4ao3mnao4go3go4go3go4go3go4go3go4msg8e8c8e8o4mng#"
	"o3g#o4g#o3g#o4g#o3g#o4g#o3g#o4msg#8e8o3b8o4e8mng#o3g#o4g#o3g#o4g#"
	"o3g#o4g#o3g#o4msg#8f8c#8f8mna#o3a#o4a#o3a#o4a#o3a#o4a#o3a#o4msa#8"
	"g8e8g8b8p16mna#p16ap16g#p16f#p16ep16"
	"d#p16c#p16o3bp16a#p16ap16g#p16f#p16ep16d#p16f#mle"
	"f#g#mnamlg#abmno4c#o3mlbo4c#d#mnemld#ef#mng#ao3bo4ao3bo4a"
	"o3bo4ao3bo4ao3bo4ao3bo4ao3bo4ao3bmlef#g#mnamlg#abmno4c#o3mlb"
	"o4c#d#mnemld#ef#mng#ao3bo4ao3bo4ao3bo4ao3bo4ao3bo4ao3bo4a"
	"o3bo4ao3bp16mlg#o4g#o3mng#p16mld#o4d#o3mnd#p16mleo4eo3mnep16mlao4ao3mnap16"
	"mlg#o4g#o3mng#p16mld#o4d#o3mnd#p16mleo4eo3mnep16mlao4ao3mnao4go3go4go3go4g"
	"o3go4go3go4g8e8c8e8g#o3g#o4g#o3g#o4g#o3g#o4g#o3g#o4g#8"
	"e8o3b8o4e8g#o3g#o4g#o3g#o4g#o3g#o4g#o3g#o4msg#8mnf8c#8"
	"f8a#o3a#o4a#o3a#o4a#o3a#o4a#o3a#o4a#8g8e8g8b8"
	"p16a#p16ap16g#p16f#p16ep16d#p16c#p16o3bp16a#p16"
	"ap16g#p16f#p16ep16d#p16fmled#ed#mne8bbb8"
	"bbb8bbo4e8f#8g#8o3bbb8bbb8"
	"bbo4g#8a8b8p8e8f#8g#8p8o3g#8"
	"a8b8p8p2o2bco3c#dd#"
	"eff#gg#aa#bco4c#d#ed#f#d#ed#f#d#e"
	"d#f#d#ed#f#d#ed#f#d#ed#f#d#ed#f#d#e"
	"d#f#d#e8eo3eo4eo3eo4eo3eo4e8o3bo2bo3bo2bo3bo2bo3b8"
	"g#o2g#o3g#o2g#o3g#o2g#o3g8eo2eo3eo2eo3eo2eo3e8eee8"
	"e8e8o2bbb8b8b8g#g#g#8g#8g#8"
	"eee8e8e8o1b8o2e8o1b8o2g#8e8b8"
	"g#8o3e8o2b8o3e8o2b8o3g#8e8b8g#8o4e4"
	"p8eee8e8e8e8e4p8."
	"ee4p8.o2ee2";




char fStopPlay = 0;


void PlayThread(void *dummy)
{
    dummy = dummy;

    /* Change priority so the thread will not pause before a new note.
     * There is no problem running this thread at a higher priority
     * as it does nothing more than waiting.
     */
    DosSetPriority( PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, 0 );
    BasicPlay(szWilli, &fStopPlay);
}    

int main()
{
    assert( _beginthread(PlayThread, NULL, 8192, 0) != -1 );

    printf("Press <RET> to stop playing\n");
    getchar();
    fStopPlay = 1;
    printf("Playing stopped...");
    fflush( stdout );
    DosSleep(2000);
    printf("program stopped\n");
    return 0;
}