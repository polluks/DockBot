/************************************
**
**  DockBot - A Dock For AmigaOS 3
**
**  � 2016 Andrew Kennan
**
************************************/

#include "dock.h"

#include <clib/alib_protos.h>
#include <clib/dos_protos.h>
#include <clib/intuition_protos.h>

#include "dock_gadget.h"

#include "dockbot_protos.h"
#include "dockbot_pragmas.h"

#define CONFIG_FILE "ENV:DockBot.prefs"

#define S_ALIGN "align"
#define S_POSITION "position"
#define S_GADGET "gadget"

struct Values AlignValues[] = {
    { "left", DA_LEFT },
    { "center", DA_CENTER },
    { "right", DA_RIGHT },
    { NULL, 0 }
};

struct Values PositionValues[] = {
    { "left", DP_LEFT },
    { "right", DP_RIGHT },
    { "top", DP_TOP },
    { "bottom", DP_BOTTOM },
    { NULL, 0 }
};


BOOL create_dock_gadget(struct DockWindow *dock, struct DockSettings *settings)
{
    struct DockSettingValue v;
    Object *gad;
    char gadName[50];

    while( ReadSetting(settings, &v) ) {
        if( IS_KEY(S_GADGET, v) ) {
            CopyMem(v.Value, &gadName, v.ValueLength);
            if( gad = NewObjectA(NULL, gadName, TAG_DONE ) ) {
                dock_gadget_read_settings(gad, settings);
                add_dock_gadget(dock, gad);

                return TRUE;                
            }
            break;
        }
    }
    return FALSE;
}


BOOL load_config(struct DockWindow *dock)
{
    struct DockSettings *s;
    struct DockSettingValue v;
    struct Values* vals;
    UWORD l;
    BOOL r;

    r = TRUE;
    if( s = OpenSettingsRead(CONFIG_FILE) ) {

        if( ReadBeginBlock(s) ) {

            while( TRUE ) {

                if( ReadBeginBlock(s) ) {
                    if( ! create_dock_gadget(dock, s) ) {
                        r = FALSE;
                        break;
                    }
                    if( !ReadEndBlock(s) ) {
                        r = FALSE;
                        break;
                    }
                }
                if( ReadEndBlock(s) ) {
                    break;
                }
                if( ReadSetting(s, &v) ) {
                    if( IS_KEY(S_ALIGN, v) ) {
                        GET_VALUE(v, AlignValues, vals, l, dock->align)
                    }

                    else if( IS_KEY(S_POSITION, v) ) {
                        GET_VALUE(v, PositionValues, vals, l, dock->pos)
                    }
                }
            }
        }
        CloseSettings(s);
    }
    return r;
    
}


BOOL init_config_notification(struct DockWindow *dock)
{
    if( dock->notifyPort = CreateMsgPort() ) {
        dock->notifyReq.nr_Name = CONFIG_FILE;
        dock->notifyReq.nr_Flags = NRF_SEND_MESSAGE | NRF_WAIT_REPLY;
        dock->notifyReq.nr_stuff.nr_Msg.nr_Port = dock->notifyPort;
    
        if( StartNotify(&dock->notifyReq) ) {
            return TRUE;
        }
    }
    return FALSE; 
}