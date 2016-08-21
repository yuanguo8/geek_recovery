#ifndef _MGYUNUI_H_
#define _MGYUNUI_H_
#include <pthread.h>
#define MENU_TEXT_COLOR 0, 191, 255, 255
#define NORMAL_TEXT_COLOR 200, 200, 200, 255
#define HEADER_TEXT_COLOR NORMAL_TEXT_COLOR
#define RED_COLOR 255, 0, 0, 255
#define GREEN_COLOR 0, 255, 0, 255
#define BLUE_COLOR 0, 0, 255, 255
#define BLACK_COLOR 0, 0, 0, 255
#define WHITE_COLOR 255, 255, 255, 255
#define ORANGE_COLOR 255, 174, 0, 255

#define CHINESE					0
#define ENGLISH					1
#define UNKNOWN_LANGUAGE        100
#define WIDGET_BUTTON           0
#define WIDGET_LABEL            1
#define WIDGET_BUTTON_BOX       2
#define WIDGET_PLANE            3
#define SHOW_FALSE              0
#define SHOW_TURE               1
#define RIGHT_ALIGN             1
#define CENTRE_ALIGN            2
#define LEFT_ALIGN              3
#define BUTTON_BOX_NO_BUTTON    -1
#define WIPE_CACHE              1
#define WIPE_DATA               2
#define WIPE_ALL_DATA           3

typedef void (*Mg_Function)(int);
static  pthread_mutex_t gUpdateMutex = PTHREAD_MUTEX_INITIALIZER;
gr_surface gMg_background;
gr_surface gMask_button;
gr_surface gLogo_status_cn;
gr_surface gLogo_status_en;
gr_surface gLogo_battery_full;
gr_surface gLogo_battery_normal;
gr_surface gLogo_battery_low;
gr_surface gDividing_line;
gr_surface gChoose_language;
gr_surface gLang_ch[2];
gr_surface gLang_en[2];
gr_surface gMenu_text_tip;

static const struct { gr_surface* surface; const char *name; } MG_FRAME[] = {
    { &gMg_background, "background" },
    { &gMask_button, "mask_button" },
    { &gDividing_line, "dividing_line" },
    { &gLogo_status_cn, "logo_status_cn" },
    { &gLogo_status_en, "logo_status_en" },
    { &gLogo_battery_full, "battery_full" },
    { &gLogo_battery_normal, "battery_normal" },
    { &gLogo_battery_low, "battery_low" },
    { &gChoose_language, "choose_language" },
    { &gLang_ch[0], "lang_ch_normal" },
    { &gLang_ch[1], "lang_ch_selected" },
    { &gLang_en[0], "lang_en_normal" },
    { &gLang_en[1], "lang_en_selected" },
    { &gMenu_text_tip, "menu_text_tip" }, 
    { NULL,                             NULL },
};

gr_surface gRecovery_mian_menu;
gr_surface gRecovery_main_menu_text_tip;
gr_surface gUpgrade_method_text;
gr_surface gSelect_updatezip_text;
gr_surface gUpgrade_method_text;
gr_surface gUpgrade_method_text;
gr_surface gUpdate_confirm_tips;
gr_surface gUpdate_success_tips;
gr_surface gUpdate_fail_tips;
gr_surface gWipe_data_text;
gr_surface gWipe_confirm_text;
gr_surface gAdvanced_settings_text;
gr_surface gUpdateing_tiptex;
gr_surface gWipedataing_tiptex;
gr_surface gBackuping_system_tiptex;
gr_surface gRestoning_system_tiptex;
gr_surface gPushing_rom_tiptex;
gr_surface gMount_storage_text;
gr_surface gMount_storage;
gr_surface gBackup_and_restore_tips;
gr_surface gConfirm_Backup_tips;
gr_surface gConfirm_restore_tips;
gr_surface gBack_to_previous_button;

gr_surface gRecovery_update[2];
gr_surface gRecovery_wipe_data[2];
gr_surface gadvanced_settings[2];
gr_surface gRecovery_reboot_system[2];
gr_surface gRecovery_shutdown[2];
gr_surface gFrom_emmc_update_button[2];
gr_surface gFrom_sdcard_update_button[2];
gr_surface gSideload_mode_button[2];
gr_surface gConfirm_button_ture[2];
gr_surface gCancel[2];
gr_surface gWipe_cache_button[2];
gr_surface gWipe_data_button[2];
gr_surface gWipe_all_button[2];
gr_surface gBack_button[2];
gr_surface gBackup_restore_button[2];
gr_surface gMount_storage_button[2];
gr_surface gBackup_system_button[2];
gr_surface gRestore_system_button[2];

static const struct  {gr_surface* surface; const char *name; } MG_RECOVERY_SOURCE[]={
    { &gRecovery_mian_menu, "recovery_mian_menu_text" },
    { &gRecovery_update[0], "update_normal" },
    { &gRecovery_update[1], "update_selected" },
    { &gRecovery_wipe_data[0], "wipe_data_normal" },
    { &gRecovery_wipe_data[1], "wipe_data_selected" },
    { &gadvanced_settings[0], "advanced_settings_normal" },
    { &gadvanced_settings[1], "advanced_settings_selected" },
    { &gRecovery_reboot_system[0], "reboot_system_normal" },
    { &gRecovery_reboot_system[1], "reboot_system_selected" },
    { &gRecovery_shutdown[0], "shutdown_normal" },
    { &gRecovery_shutdown[1], "shutdown_selected" },
    { &gRecovery_main_menu_text_tip, "main_menu_text_tip" },
    { &gUpgrade_method_text, "upgrade_method_text" },
    { &gSelect_updatezip_text, "selected_zip_text" },
    { &gBack_to_previous_button, "back_to_previous" },
    { &gFrom_emmc_update_button[0], "from_emmc_update_normal" },
    { &gFrom_emmc_update_button[1], "from_emmc_update_selected" },
    { &gFrom_sdcard_update_button[0], "from_sdcard_update_normal" },
    { &gFrom_sdcard_update_button[1], "from_sdcard_update_selected" },
    { &gSideload_mode_button[0], "from_sideload_mode_normal" },
    { &gSideload_mode_button[1], "from_sideload_mode_selected" },
    { &gUpdate_confirm_tips, "update_confirm_tips" },
    { &gUpdate_success_tips, "update_success_tips" },
    { &gUpdate_fail_tips, "update_fail_tips" },
    { &gConfirm_button_ture[0], "btn_confirm_normal" },
    { &gConfirm_button_ture[1], "btn_confirm_selected" },
    { &gCancel[0], "btn_cancel_normal" },
    { &gCancel[1], "btn_cancel_selected" },
    { &gWipe_data_text, "wipe_data_text" },
    { &gWipe_confirm_text, "wipe_data_confirm_tips" },
    { &gWipe_cache_button[0], "wipe_cache_normal" },
    { &gWipe_cache_button[1], "wipe_cache_selected" },
    { &gWipe_data_button[0], "wipe_data_normal" },
    { &gWipe_data_button[1], "wipe_data_selected" },
    { &gWipe_all_button[0], "wipe_all_data_normal" },
    { &gWipe_all_button[1], "wipe_all_data_selected" },
    { &gBack_button[0], "btn_back_normal" },
    { &gBack_button[1], "btn_back_selected" },
    { &gAdvanced_settings_text, "advanced_settings_text" },
    { &gBackup_restore_button[0], "btn_backup_restore_normal" },
    { &gBackup_restore_button[1], "btn_backup_restore_selected" },
    { &gMount_storage_button[0], "btn_mount_storage_normal" },
    { &gMount_storage_button[1], "btn_mount_storage_selected" },
    { &gBackup_system_button[0], "btn_backup_normal" },
    { &gBackup_system_button[1], "btn_backup_selected" },
    { &gRestore_system_button[0], "btn_restore_normal" },
    { &gRestore_system_button[1], "btn_restore_selected" },
    { &gUpdateing_tiptex, "update_text_tips" },
    { &gWipedataing_tiptex, "wipeing_data_tips" },
    { &gBackuping_system_tiptex, "backuping_system_tips" },
    { &gRestoning_system_tiptex, "restoreping_system_tips" },
    { &gMount_storage, "mount_storage"},
    { &gConfirm_Backup_tips, "confirm_backup_system_tips"},
    { &gConfirm_restore_tips, "confirm_restore_system_tips"},
    { &gMount_storage_text, "mount_storage_text"},
    { &gBackup_and_restore_tips, "back_and_restore_title"},
    { &gPushing_rom_tiptex, "pushing_rom_tips" },
    { NULL,                             NULL },
};
typedef struct Widget{
    int type;
    Mg_Function fun;
    int display;
	int x;
	int y;
	int width;
	int height;
    gr_surface normal_data;
    gr_surface select_data;
    struct Widget *next;
}Mg_Widget;

typedef struct Widget_Button_Box
{
    int x;
    int y;
    int dx;
    int dy;
    char **text;
    int files_number;
    int show_start_num;
    int show_end_num;
    int button_nun;
    int button_selected;
    gr_surface back_button;
    gr_surface mask_button;
    gr_surface dividing_line;
}Button_Box;

typedef Mg_Widget Mg_Plane;

typedef struct Page_tree{
    struct Page_tree *father;
    struct Page_tree *children[8];
    Mg_Plane  *show_page;
    Button_Box *button_box;
    int button_nun;
    int button_selected;

}Mg_Page_tree;

void mgui_init();
void creat_mgui_frame();
void show_mgui_plane(Mg_Widget *activities);
int get_batt_stats(void);
void  mg_main_ui_wait();
int mg_get_menu_selection();
int mg_analysis_choose();
int mgui_menu_select(int selected);
int mgui_get_selected_item();
void mg_first_screen();
void  mg_confirm_language(int selected);
void mg_show_root_menu();
void creat_main_menu_page();
void mg_load_picture(char* path);
void mgui_update_screen();
Mg_Plane* new_plane(int x,int y,int dx, int dy,int align, gr_surface picture);
void add_widget_to_plane(Mg_Plane *head, int x,int y,int dx, int dy,int type,int align, gr_surface normal_picture,gr_surface select_picture,Mg_Function function);
int get_plane_y(Mg_Plane *plane);
int get_plane_x(Mg_Plane *plane);
int get_plane_height(Mg_Plane *plane);
int get_plane_width(Mg_Plane *plane);
void mg_show_ui_plane(Mg_Page_tree *activities);
void creat_mgui_frame();
void creat_mgui_init();
int get_batt_stats(void);
void mg_ui_update_file_manage(Button_Box *file_box);

Mg_Plane  *show_menu_head;
Mg_Page_tree *root_page;
Mg_Page_tree *current_directory;
char update_zip_path[PATH_MAX];
int wipe_mark;
char mg_picture_source_path[PATH_MAX];
int mg_resolution;
#endif