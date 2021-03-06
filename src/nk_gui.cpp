#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_IMPLEMENTATION
#define NK_GDI_IMPLEMENTATION
#include "nk_gui.h"

#include "nuklear/nuklear.h"
#include "nuklear/nuklear_gdi.h"
#include "window.h"
#include "filehandling.h"
#include <codecvt>
#include <locale>
#include <cstring>

GdiFont* font;
struct nk_context *ctx;
unsigned int windowWidth;
unsigned int windowHeight;

// was a file loaded?
bool fileOpen=false;
// duel number chosen through the combobox
int selectedDuel;
// series names displayed in the series combobox
std::vector<std::string> seriesStrings;
// informative string displayed in the log at the bottom
std::string infoString;
// strings edited by the textboxes
std::string editStringDuelNameID;
std::string editStringLeftAltId;
std::string editStringRightAltId;
std::string editStringDuelName;
std::string editStringDuelTip;

void SetupGui(WINDOW_DATA &windowData,unsigned int initialWindowWidth, unsigned int initialWindowHeight){
    font = nk_gdifont_create("Segoe UI", 18);
    windowWidth=initialWindowWidth;
    windowHeight=initialWindowHeight;
    ctx = nk_gdi_init(font, windowData.dc, initialWindowWidth, initialWindowHeight);
    
    selectedDuel=0;
    seriesStrings.push_back("DM");
    seriesStrings.push_back("GX");
    seriesStrings.push_back("5DS");
    seriesStrings.push_back("ZEXAL");
    seriesStrings.push_back("ARC-V");
    seriesStrings.push_back("VRAINS");
    
    infoString="No file loaded. Open a dueldata_X.bin file to begin.";
    
}

void HandleInput(WINDOW_DATA &windowData){
        MSG msg;
        nk_input_begin(ctx);
        if (windowData.needs_refresh == 0) {
            if (GetMessageW(&msg, NULL, 0, 0) <= 0)
                windowData.running = 0;
            else {
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            windowData.needs_refresh = 1;
        } else windowData.needs_refresh = 0;

        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                windowData.running = 0;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
            windowData.needs_refresh = 1;
        }
        nk_input_end(ctx);
        
}

int HandleEvent(const EVENT_DATA &eventData){
    switch (eventData.msg)
    {
	case WM_SIZE:
        windowWidth=LOWORD(eventData.lparam);
        windowHeight=HIWORD(eventData.lparam);
		return nk_gdi_handle_event(eventData.wnd, eventData.msg, eventData.wparam, eventData.lparam);
    default:
        return nk_gdi_handle_event(eventData.wnd, eventData.msg, eventData.wparam, eventData.lparam);
    }
    
}

void HandleGui(FILE_DATA &fileData){
    if (nk_begin(ctx, "Demo", nk_rect(0, 0, windowWidth, windowHeight),
        0))
    {
        nk_layout_row_static(ctx, 0, 100, 2);
        if (nk_menu_begin_label(ctx,"FILE",NK_TEXT_LEFT,nk_vec2(100, 100))){
            nk_layout_row_dynamic(ctx, 0, 1);
            if(nk_menu_item_label(ctx, "Open", NK_TEXT_LEFT)){
                std::string filename=OpenFilename("YGO LOTD LE Duel Data Files (*.bin)\0*.*\0");
                if (!filename.empty()){
                    bool success=ReadFile(filename,fileData);
                    if (success){
                        fileOpen=true;
                        selectedDuel=0;
                        UpdateTextEdits(selectedDuel,fileData);
                        infoString=std::to_string(fileData.duelCount)+" duels loaded from file "+filename;
                    }
                    else{
                        infoString="Either the file could not be opened, or there was a read error.";
                    }
                }
            }
            if(nk_menu_item_label(ctx, "Save", NK_TEXT_LEFT) && fileOpen){
                std::string filename=SaveFilename("YGO LOTD LE Duel Data Files (*.bin)\0*.*\0");
                if (!filename.empty()){
                    UpdateStrings(selectedDuel,fileData);
                    bool success=SaveFile(filename,fileData);
                    if (success){
                        infoString="Successfully saved data to "+filename;
                    }else{
                        infoString="Either the file could not be created, or there was a write error.";
                    }

                }
            }
            nk_menu_end(ctx);
        }
        if (nk_menu_begin_label(ctx,"EDIT",NK_TEXT_LEFT,nk_vec2(100, 100))){
            
            nk_layout_row_dynamic(ctx, 0, 1);
            if(nk_menu_item_label(ctx, "Add Duel Slot", NK_TEXT_LEFT) && fileOpen){
                UpdateStrings(selectedDuel,fileData);
                fileData.duelCount++;
                selectedDuel=fileData.duelCount-1;
                fileData.field1.push_back(fileData.field1[selectedDuel-1]+1);
                fileData.field2.push_back(0);
                fileData.field3.push_back(1);
                fileData.field4.push_back(1);
                fileData.field5.push_back(1);
                fileData.field6.push_back(1);
                fileData.field7.push_back(1);
                fileData.field8.push_back(1);
                fileData.field9.push_back(0xFFFFFFFF);
                fileData.field10.push_back(0xFFFFFFFF);
                fileData.field11.push_back(0);
                fileData.string1.push_back("Duel"+std::to_string(fileData.field1[selectedDuel]));
                fileData.string2.push_back("");
                fileData.string3.push_back("");
                fileData.string4.push_back("Duel "+std::to_string(fileData.field1[selectedDuel]));
                fileData.string5.push_back(u"");
                fileData.string6.push_back("");
                UpdateTextEdits(selectedDuel,fileData);
            }
            nk_menu_end(ctx);
        }

        
        float ratioSelectDuel[]={0.2,0.6,0.2};
        nk_layout_row(ctx, NK_DYNAMIC, 0, 3, ratioSelectDuel);
        
        if (nk_button_label(ctx, "Prev") && fileOpen){
            if (selectedDuel>0){
                UpdateStrings(selectedDuel,fileData);
                selectedDuel--;
                UpdateTextEdits(selectedDuel,fileData);
            }
        }

        int comboWasClicked;
        std::string comboLabel;
        if (fileOpen)
            comboLabel=std::to_string(fileData.field1[selectedDuel])+" - "+fileData.string1[selectedDuel];
        if(nk_combo_begin_label(ctx, comboLabel.c_str(),nk_vec2(237,190),&comboWasClicked)){
            if (comboWasClicked){
                nk_popup_set_scroll(ctx,0,selectedDuel*(ctx->current->layout->row.min_height+ctx->style.window.spacing.y));
            }
            if (fileOpen){
                nk_layout_row_dynamic(ctx, 0, 1);
                for (Long i=0; i< fileData.duelCount; i++){
                    int selected=nk_false;
                    if (nk_selectable_label(ctx,(std::to_string(fileData.field1[i])+" - "+fileData.string1[i]).c_str(),NK_TEXT_LEFT,&selected)){
                        if (selectedDuel!=i){
                            UpdateStrings(selectedDuel,fileData);
                            selectedDuel=i;
                            UpdateTextEdits(selectedDuel,fileData);
                        }
                        nk_combo_close(ctx);
                    }
                }
            }
            nk_combo_end(ctx);
        }

        if (nk_button_label(ctx, "Next") && fileOpen){
            if (selectedDuel+1<fileData.duelCount){
                UpdateStrings(selectedDuel,fileData);
                selectedDuel++;
                UpdateTextEdits(selectedDuel,fileData);
            }
        }
        float ratioEdit[]={0.15,0.1,0.15,0.6};
        nk_layout_row(ctx, NK_DYNAMIC, 460, 4, ratioEdit);

        if(nk_group_begin_titled(ctx, "group_fields", "Fields", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            nk_layout_row_dynamic(ctx, 0, 1);
            nk_label(ctx,"Duel ID",NK_TEXT_LEFT);
            nk_label(ctx,"Series ID",NK_TEXT_LEFT);
            nk_label(ctx,"Number in Series",NK_TEXT_LEFT);
            nk_label(ctx,"Left Char ID",NK_TEXT_LEFT);
            nk_label(ctx,"Right Char ID",NK_TEXT_LEFT);
            nk_label(ctx,"Left Char Deck ID",NK_TEXT_LEFT);
            nk_label(ctx,"Right Char Deck ID",NK_TEXT_LEFT);
            nk_label(ctx,"Arena ID",NK_TEXT_LEFT);
            nk_label(ctx,"Reward Pack ID",NK_TEXT_LEFT);
            nk_label(ctx,"Field 10",NK_TEXT_LEFT);
            nk_label(ctx,"Field 11",NK_TEXT_LEFT);
            nk_group_end(ctx);
        }
        
        if(nk_group_begin_titled(ctx, "group_hex", "Hex", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            if (fileOpen){
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_label(ctx,IntToHexString(fileData.field1[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field2[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field3[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field4[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field5[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field6[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field7[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field8[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field9[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field10[selectedDuel]).c_str(),NK_TEXT_LEFT);
                nk_label(ctx,IntToHexString(fileData.field11[selectedDuel]).c_str(),NK_TEXT_LEFT);
            }
            nk_group_end(ctx);
        }

        if(nk_group_begin_titled(ctx, "group_edit", "Edit", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            if (fileOpen){
                nk_layout_row_dynamic(ctx, 0, 1);
                nk_label(ctx,std::to_string((int)(fileData.field1[selectedDuel])).c_str(),NK_TEXT_LEFT);
                if(nk_combo_begin_label(ctx, seriesStrings[fileData.field2[selectedDuel]].c_str(),nk_vec2(111,265))){
                    nk_layout_row_dynamic(ctx, 0, 1);
                    for (Long i=0; i< seriesStrings.size(); i++){
                        int selected=nk_false;
                        if (nk_selectable_label(ctx,seriesStrings[i].c_str(),NK_TEXT_LEFT,&selected)){
                            fileData.field2[selectedDuel]=i;
                            nk_combo_close(ctx);
                            
                        }
                    }
                    nk_combo_end(ctx);
                }
                
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field3[selectedDuel])),INT_MAX,1,1.0);
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field4[selectedDuel])),INT_MAX,1,1.0);
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field5[selectedDuel])),INT_MAX,1,1.0);
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field6[selectedDuel])),INT_MAX,1,1.0);
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field7[selectedDuel])),INT_MAX,1,1.0);
                nk_property_int(ctx,"#",1,reinterpret_cast<int*>(&(fileData.field8[selectedDuel])),INT_MAX,1,1.0);
                nk_property_int(ctx,"#",-1,reinterpret_cast<int*>(&(fileData.field9[selectedDuel])),INT_MAX,1,1.0);

                nk_label(ctx,std::to_string((int)(fileData.field10[selectedDuel])).c_str(),NK_TEXT_LEFT);
                int checkedField11=fileData.field11[selectedDuel];
                if (nk_checkbox_label(ctx,"???",&checkedField11)){
                    fileData.field11[selectedDuel]=checkedField11;
                }
            }
            nk_group_end(ctx);
        }

        if(nk_group_begin_titled(ctx, "group_strings", "Strings", NK_WINDOW_TITLE|NK_WINDOW_BORDER)){
            if (fileOpen){
                float ratio[]={0.2,0.8};
                nk_layout_row(ctx, NK_DYNAMIC, 0, 2, ratio);
                nk_label(ctx,"Duel ID Name: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringDuelNameID.c_str()),50,0);
                nk_label(ctx,"Left Char Alt ID: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringLeftAltId.c_str()),50,0);
                nk_label(ctx,"Right Char Alt ID: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringRightAltId.c_str()),50,0);
                nk_label(ctx,"Duel Name: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringDuelName.c_str()),50,0);
                nk_label(ctx,"Descr. (unused): ",NK_TEXT_LEFT);
                std::string u8_conv = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(fileData.string5[selectedDuel]);
                nk_label(ctx,u8_conv.c_str(),NK_TEXT_LEFT);
                nk_label(ctx,"Duel Tip: ",NK_TEXT_LEFT);
                nk_edit_string_zero_terminated(ctx,NK_EDIT_FIELD|NK_EDIT_SELECTABLE ,const_cast<char*>(editStringDuelTip.c_str()),200,0);
            }
            nk_group_end(ctx);
        }
        nk_layout_row_dynamic(ctx,0,1);
        nk_edit_string_zero_terminated(ctx,NK_EDIT_INACTIVE,const_cast<char*>(("INFO: "+infoString).c_str()),INT_MAX,nk_filter_default);
    }
    nk_end(ctx);
}


void UpdateStrings(int idx, FILE_DATA &fileData){
    fileData.string1[idx]=editStringDuelNameID;
    fileData.string1[idx].resize(strlen(&editStringDuelNameID[0]));
    fileData.string2[idx]=editStringLeftAltId;
    fileData.string2[idx].resize(strlen(&editStringLeftAltId[0]));
    fileData.string3[idx]=editStringRightAltId;
    fileData.string3[idx].resize(strlen(&editStringRightAltId[0]));
    fileData.string4[idx]=editStringDuelName;
    fileData.string4[idx].resize(strlen(&editStringDuelName[0]));
    fileData.string6[idx]=editStringDuelTip;
    fileData.string6[idx].resize(strlen(&editStringDuelTip[0]));
}

void UpdateTextEdits(int idx, FILE_DATA &fileData){
    editStringDuelNameID=fileData.string1[idx];
    editStringDuelNameID.resize(50);
    editStringLeftAltId=fileData.string2[idx];
    editStringLeftAltId.resize(50);
    editStringRightAltId=fileData.string3[idx];
    editStringRightAltId.resize(50);
    editStringDuelName=fileData.string4[idx];
    editStringDuelName.resize(50);
    editStringDuelTip=fileData.string6[idx];
    editStringDuelTip.resize(200);
}

void RenderGui(){
    nk_gdi_render(nk_rgb(30,30,30));
}

void CleanupGui(){
    nk_gdifont_del(font);
}

void UpdateWindowSize(unsigned int width, unsigned int height){
    windowWidth=width;
    windowHeight=height;
}