#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/time.h>
#include <time.h>

#include "bootloader.h"
#include "common.h"
#include "cutils/properties.h"
#include "cutils/android_reboot.h"
#include "install.h"
#include "minui/minui.h"
#include "minzip/DirUtil.h"
#include "roots.h"
#include "recovery_ui.h"
#include "voldclient/voldclient.h"
#include "adb_install.h"
#include "minadbd/adb.h"
#include "firmware.h"
#include "extendedcommands.h"
#include "flashutils/flashutils.h"
#include "dedupe/dedupe.h"
#include "voldclient/voldclient.h"
#include "mgyunui.h"
//#include "recovery_cmds.h"
#include "mgyunui.h"
static int system_language=0;

int get_batt_stats(void)
{
#ifdef MG_RECOVERY_BATTERY_PATH
    static int level = -1;
    char value[4];
    FILE * capacity;
    if ( (capacity = fopen(MG_RECOVERY_BATTERY_PATH,"r")) ) {
        fgets(value, 4, capacity);
        fclose(capacity);
    } else {
    	printf("recovery battery path is error!\n");

    }
    level = atoi(value);
    if (level > 100)
        level = 100;
    if (level < 0)
        level = 0;
    return level;
#else
    return 0;
#endif
}
void creat_mgui_init()
{
	sprintf(mg_picture_source_path,"/res/%d/default\0",gr_fb_width());
	int i;
    for (i = 0; MG_FRAME[i].name != NULL; i++) {
        int result =res_create_surface(mg_picture_source_path,MG_FRAME[i].name, MG_FRAME[i].surface);
        if (result < 0) {
            LOGE("Missing bitmap %s\n(Code %d)\n", MG_FRAME[i].name, result);
        }
    }
}

void creat_mgui_frame()
{
    gr_surface gLogo_status;
    gr_surface gLogo_battery;
    int battery_capacity;
    char text_battery[8];
    if (system_language == CHINESE)
    {
        gLogo_status = gLogo_status_cn;
    }
    else{
    	gLogo_status = gLogo_status_en;
    }
    battery_capacity=get_batt_stats();
    if (battery_capacity >= 80)
    {
        gLogo_battery = gLogo_battery_full;
        gr_color(ORANGE_COLOR );
    }
    else if (battery_capacity >=30 && battery_capacity <80)
    {
        gLogo_battery = gLogo_battery_normal;
        gr_color(ORANGE_COLOR);
    }
     else{
        gLogo_battery = gLogo_battery_low;
        gr_color(RED_COLOR);
    }
    
    pthread_mutex_lock(&gUpdateMutex);
    {
        int bw = gr_get_width(gMg_background);
        int bh = gr_get_height(gMg_background);
        int bx = 0;
        int by = 0;
        for (by = 0; by < gr_fb_height(); by += bh) {
            for (bx = 0; bx < gr_fb_width(); bx += bw) {
                gr_blit(gMg_background, 0, 0, bw, bh, bx, by);
            }
        }

    }
    
    {
       int bw = gr_get_width(gLogo_status);
       int bh = gr_get_height(gLogo_status);
       int bx = 0;
       int by = 0;
       gr_blit(gLogo_status, 0, 0, bw, bh, bx, by);

    }
    {
       int bw = gr_get_width(gLogo_battery);
       int bh = gr_get_height(gLogo_battery);
       int bx = gr_fb_width()-bw-20;
       int by = bh/4;
       gr_blit(gLogo_battery, 0, 0, bw, bh, bx, by);

    }
    {
    	sprintf(text_battery,"%d%%",battery_capacity);
    	int bx = 0;
       	int by = 0;
       	int text_length;
       	text_length = gr_measure(text_battery);
       	//gr_font_size(bx,by);
       	bx= gr_get_font_width();
       	by= gr_get_font_height();
     
       	by = gr_get_height(gLogo_status) /2;
       	//bx = gr_fb_width() - gr_get_width(gLogo_battery) - text_length -22;
       	bx = gr_fb_width()  - (gr_get_width(gLogo_battery) + text_length )/2 - 20;
    	gr_text(bx,by,text_battery);
    }
    gr_flip();
    pthread_mutex_unlock(&gUpdateMutex);
}
void mg_show_ui_plane(Mg_Page_tree *activities)
{
	Mg_Widget  *prefix,*suffix;
	prefix=activities->show_page;
	suffix=activities->show_page;
	pthread_mutex_lock(&gUpdateMutex);
	{
		int count=0,i;
		do{
			if (WIDGET_BUTTON == prefix->type)
			{
				count++;
			}
			if (SHOW_FALSE == prefix->display)
			{
				suffix=prefix;
	       		prefix=prefix->next;
	       		continue;
			}
			if (NULL == prefix->normal_data)
			{
				suffix=prefix;
	       		prefix=prefix->next;
	       		continue;
			}
	       int bw = prefix->width;
	       int bh = prefix->height;
	       int bx = prefix->x;
	       int by = prefix->y;
	       if (WIDGET_BUTTON == prefix->type && count == activities->button_selected)
	       {
	       		gr_blit(prefix->select_data, 0, 0, bw, bh, bx, by);
	       }
	       else
	       {
	       	gr_blit(prefix->normal_data, 0, 0, bw, bh, bx, by);
	       }
	       suffix=prefix;
	       prefix=prefix->next;
	   }while(suffix->next !=NULL);

    }
	gr_flip();
    pthread_mutex_unlock(&gUpdateMutex);
    
}
int get_plane_width(Mg_Plane *plane)
{
	return plane->width;
}
int get_plane_height(Mg_Plane *plane)
{
	return plane->height;
}
int get_plane_x(Mg_Plane *plane)
{
	return plane->x;
}
int get_plane_y(Mg_Plane *plane)
{
	return plane->y;
}
void add_widget_to_plane(Mg_Plane *head, int x,int y,int dx, int dy,int type,int align, gr_surface normal_picture,gr_surface select_picture,Mg_Function function)
{
	Mg_Widget *widget;
	Mg_Plane *prefix,*suffix;
	prefix = head;
	suffix = head;
	widget=calloc(sizeof(Mg_Widget),1);
	widget->type=type;

	if (CENTRE_ALIGN ==align)
	{
		widget->x = (get_plane_width(head) - dx)/2 + head->x;
		widget->y = get_plane_y(head) + y;
	}
	else if (RIGHT_ALIGN == align)
	{
		widget->x = (get_plane_width(head)-dx) + head->x;
		widget->y = get_plane_y(head) + y;
	}
	else
	{
		widget->x=x;
		widget->y=get_plane_y(head) + y;
	}
	widget->fun = function;
	widget->display = SHOW_TURE;
	widget->width=dx;
	widget->height=dy;
	widget->normal_data= normal_picture;
	widget->select_data= select_picture;
	widget->next = NULL;
	while(prefix->next !=NULL) prefix = prefix->next;
	prefix->next = widget;

}

Mg_Plane* new_plane(int x,int y,int dx, int dy,int align, gr_surface picture)
{
	Mg_Plane *plane;
	plane = calloc(sizeof(Mg_Plane),1);
	plane->type = WIDGET_PLANE;
	plane->display = SHOW_TURE;
	plane->width = dx;
	plane->height = dy;
	plane->normal_data= picture;
	plane->select_data=NULL;
	plane->next=NULL;
	if (CENTRE_ALIGN == align)
	{
		plane->x = (gr_fb_width() - dx)/2;
		plane->y = y;
	}
	else if (RIGHT_ALIGN == align)
	{
		plane->x = gr_fb_width() - dx;
		plane->y =y;
	}
	else
	{
		plane->x = x;
		plane->y = y;
	}
	return plane;
}
void mgui_update_screen()
{
	mg_show_ui_plane(current_directory);

}

void mg_load_picture(char* path)
{
	int i;
    for (i = 0; MG_RECOVERY_SOURCE[i].name != NULL; i++) {
        int result =res_create_surface(path,MG_RECOVERY_SOURCE[i].name, MG_RECOVERY_SOURCE[i].surface);
        if (result < 0) {
            LOGE("Missing bitmap %s\n(Code %d)\n", MG_RECOVERY_SOURCE[i].name, result);
        }
    }
}

void mg_show_root_menu()
{
	if (CHINESE == system_language)
	{
		strcat(mg_picture_source_path,"/cn");
		mg_load_picture(mg_picture_source_path);
		creat_main_menu_page();
	}
	else if (ENGLISH == system_language)
	{
		strcat(mg_picture_source_path,"/en");
		mg_load_picture(mg_picture_source_path);
		creat_main_menu_page();

	}
}

void  mg_confirm_language(int selected)
{
	if (1 == selected)
	{
		printf("language is  chinese \n");
		system_language = CHINESE;
	}
	else if (2 == selected)
	{
		printf("language is  english \n");
		system_language = ENGLISH;
	}
	else{
		printf("unknow language default is chinese\n");
		system_language = CHINESE;
	}
	mg_show_root_menu();
}

int mgui_get_selected_item()
{
	if (BUTTON_BOX_NO_BUTTON == current_directory->button_nun)
	{
		return current_directory->button_box->button_selected;
	}
	else
	{
	return current_directory->button_selected;
	}
}


int mgui_menu_select(int selected)
{

	if (BUTTON_BOX_NO_BUTTON == current_directory->button_nun)
	{
		int start = current_directory->button_box->show_start_num;
		int end  = current_directory->button_box->show_end_num;
		if (selected < start )
		{
			if (0==selected)
			{	
			}
			else if (start >1)
			{
				start--;
				end--;
				current_directory->button_box->show_start_num = start;
				current_directory->button_box->show_end_num =end;
			}
			else if (start <=1)
			{
				current_directory->button_box->show_end_num = current_directory->button_box->files_number;
				current_directory->button_box->show_start_num = current_directory->button_box->files_number - current_directory->button_box->button_nun +1;
				selected = current_directory->button_box->show_end_num;
			}
		}
		else if (selected > end)
		{
			if(end < current_directory->button_box->files_number){
				start++;
				end++;
				current_directory->button_box->show_start_num = start;
				current_directory->button_box->show_end_num =end;
			}
			if (end >= current_directory->button_box->files_number)
			{
				current_directory->button_box->show_start_num = 1;
				current_directory->button_box->show_end_num =current_directory->button_box->button_nun;
				selected=0;
			}
		}
		
		current_directory->button_box->button_selected=selected;
		printf("%d\n",current_directory->button_box->button_selected);
		mg_ui_update_file_manage(current_directory->button_box);
		return current_directory->button_box->button_selected;
	}
	
	
	else if (selected <= 0)
	{
		selected=current_directory->button_nun;
	}
	else if (selected > current_directory->button_nun)
	{
		selected=1;
	}
	current_directory->button_selected=selected;
	mgui_update_screen();
	printf("%d\n",current_directory->button_selected );
	return current_directory->button_selected;
}
int mg_analysis_choose()
{
	int selected = mgui_get_selected_item();
	if (BUTTON_BOX_NO_BUTTON==current_directory->button_nun)
	{
		printf("BUTTON_BOX_NO_BUTTON%d\n",selected);
		return selected;
	}
	else{
		int count=0;
		Mg_Widget *tmp;
		tmp = current_directory->show_page;
		while(NULL != tmp->next)
		{
			if (WIDGET_BUTTON  == tmp->type)
			{
				count++;
			}
			if (count == current_directory->button_selected)
			{
				(tmp->fun)(current_directory->button_selected);
				break;
			}
			tmp=tmp->next;

			}
		}
	return selected;
}
int mg_get_menu_selection()
{
	//mgui_update_screen();
    ui_clear_key_queue();
    int chosen_item = -1;
    int selected = mgui_get_selected_item();
    while (chosen_item < 0 && chosen_item != GO_BACK) {
        int key = ui_wait_key();
        //int visible = ui_text_visible();

        if (key == -1) {   // ui_wait_key() timed out
            if (ui_text_ever_visible()) {
                continue;
            } else {
                LOGI("timed out waiting for key input; rebooting.\n");
                //ui_end_menu();
                return ITEM_REBOOT;
            }
        }
        else if (key == GO_BACK) {   // we are returning from ui_cancel_wait_key(): trigger a GO_BACK
            return GO_BACK;
        }
        else if (key == REFRESH) {   // an USB device was plugged in (returning from ui_wait_key())
            return REFRESH;
        }

        int action = ui_handle_key(key, 1);
        int old_selected = selected;
        selected = mgui_get_selected_item();
        if (action < 0) {
            switch (action) {
                case HIGHLIGHT_UP:
                    --selected;
                    selected = mgui_menu_select(selected);
                    continue;
                case HIGHLIGHT_DOWN:
                    ++selected;
                    selected = mgui_menu_select(selected);
                    continue;
                case SELECT_ITEM:
                    chosen_item = selected;
                   	mg_analysis_choose();
                    break;
                case NO_ACTION:
                    break;
                case GO_BACK:
                    chosen_item = GO_BACK;
                    break;
            }
        } 
    }
    ui_clear_key_queue();
    return chosen_item;
}

void  mg_main_ui_wait() {
	creat_mgui_init();
    creat_mgui_frame();
    mg_first_screen();
	while(1){
		mg_get_menu_selection();
	}
}

void mg_clear_plane(Mg_Plane *plane)
{
	int x,y,dx,dy;
	int bx,by,bw,bh;
	x = get_plane_x(plane);
	y = get_plane_y(plane);
	dx =get_plane_width(plane);
	dy =get_plane_height(plane) + y;
	bw = gr_get_width(gMg_background);
    bh = gr_get_height(gMg_background);

    pthread_mutex_lock(&gUpdateMutex);
	for (by = y; by < dy; by += bh) {
            for (bx = x; bx < dx; bx += bw) {
                gr_blit(gMg_background, 0, 0, bw, bh, bx, by);
            }
        }
    gr_flip();
    pthread_mutex_unlock(&gUpdateMutex);
}

void mg_clear_button_box_select(Button_Box *box)
{
	int x,y,dx,dy;
	int bx,by,bw,bh;
	x = box->x;
	y = box->y;
	dx =box->dx;
	dy =box->dy+y;
	bw = gr_get_width(gMg_background);
    bh = gr_get_height(gMg_background);

    pthread_mutex_lock(&gUpdateMutex);
	for (by = y; by < dy; by += bh) {
            for (bx = x; bx < dx; bx += bw) {
                gr_blit(gMg_background, 0, 0, bw, bh, bx, by);
            }
        }
    gr_flip();
    pthread_mutex_unlock(&gUpdateMutex);
}
void mg_ui_update_file_manage(Button_Box *file_box)
{	gr_color(ORANGE_COLOR);
	mg_clear_button_box_select(file_box);
	int i,span,x,y,dx,dy,fontx,fonty;
	span = gr_fb_height() * 75/1000;
	x = gr_fb_width() *88/1000;
	y = file_box->y;
	dx = gr_get_width(file_box->back_button);
	dy = gr_get_height(file_box->back_button);
	pthread_mutex_lock(&gUpdateMutex);
	gr_blit(file_box->back_button, 0, 0, dx, dy, x, y);
	
	x=0;
	y = y+span;
	dx = gr_get_width(file_box->dividing_line);
	dy = gr_get_height(file_box->dividing_line);
	gr_blit(file_box->dividing_line, 0, 0, dx, dy, x, y);

	fontx=gr_fb_width() *88/1000;

	for (i = file_box->show_start_num; i <=file_box->show_end_num; ++i)
	{
		fonty=y + (span + gr_get_font_height())/2;
		gr_text(fontx,fonty,file_box->text[i -1]);

		y=y+span;
		gr_blit(file_box->dividing_line, 0, 0, dx, dy, x, y);
	}
	x = file_box->x;
	y = file_box->y + span *(file_box->button_selected - file_box->show_start_num+1) +1;
	dx = gr_get_width(file_box->mask_button);
	dy = gr_get_height(file_box->mask_button);
	gr_blit(file_box->mask_button, 0, 0, dx, dy, x, y);
	//gr_blit(gRecovery_shutdown[1], 0, 0, dx, dy, x, y);
	gr_flip();
	pthread_mutex_unlock(&gUpdateMutex);

}

void mg_upgrade_method_choose()
{
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[0];
	mg_show_ui_plane(current_directory);
}
void mg_advanced_settings(){
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[2];
	mg_show_ui_plane(current_directory);
}

void mg_wipe_data_main(){
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[1];
	mg_show_ui_plane(current_directory);
}

void mg_return_previous_screen()
{
	mg_clear_plane(current_directory->show_page);
	current_directory->button_selected=1;
	current_directory = current_directory->father;
	mg_show_ui_plane(current_directory);
}

void mg_reboot_system()
{
	 reboot_main_system(ANDROID_RB_RESTART, 0, 0);
}
void mg_power_off()
{
	 reboot_main_system(ANDROID_RB_POWEROFF, 0, 0);
}

void mg_upgrade_from_emmc()
{
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[0];
	mg_show_ui_plane(current_directory);
	char* primary_path = get_primary_storage_path();
	printf("%s\n",primary_path);
	mg_show_choose_zip_menu(primary_path);
	


}
void mg_upgrade_from_sdcard()
{
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[0];
	mg_show_ui_plane(current_directory);
	char* extra_path = get_extra_storage_paths();
	printf("%s\n",extra_path);
	mg_show_choose_zip_menu(extra_path);
}

void mg_upgrade_from_sideload_mode()
{
	

}
void mg_install_update_zip()
{
	mg_clear_plane(current_directory->show_page);
	mg_draw_tiptext(gUpdateing_tiptex);
	int status=install_zip(update_zip_path);
	mg_clear_plane(current_directory->show_page);
	if (INSTALL_SUCCESS == status)
	{
		current_directory=current_directory->father;
		current_directory=current_directory->children[2];
	}
	else
	{
		current_directory=current_directory->father;
		current_directory=current_directory->children[3];

	}
	mgui_update_screen();

}
void mg_wipe_confirm_choose()
{
	mg_clear_plane(current_directory->show_page);
	mg_draw_tiptext(gWipedataing_tiptex);
	if (WIPE_DATA == wipe_mark)
	{
		erase_volume("/data");
		if (has_datadata()) {
        	erase_volume("/datadata");
    	}
    	erase_volume("/sd-ext");
    	erase_volume(get_android_secure_path());
    	printf("Data wipe complete.\n");

	}
	else if (WIPE_CACHE == wipe_mark)
	{
		erase_volume("/cache");
		printf("Cache  wipe complete.\n");

	}
	else if (WIPE_ALL_DATA == wipe_mark)
	{
		device_wipe_data();
    	erase_volume("/data");
    	erase_volume("/cache");
    	if (has_datadata()) {
        	erase_volume("/datadata");
    	}
    	erase_volume("/sd-ext");
    	erase_volume(get_android_secure_path());
    	printf("All data  wipe complete.\n");
	}
	else{

	}
	ui_reset_progress();
	current_directory->button_selected=1;
	mg_clear_plane(current_directory->show_page);
	current_directory=current_directory->father;
	mgui_update_screen();

}
void mg_wipe_data(int selected)
{
	mg_clear_plane(current_directory->show_page);
	current_directory=current_directory->children[0];
	if (1 == selected)
	{
		wipe_mark = WIPE_CACHE;
	}
	else if (2 == selected)
	{
		wipe_mark = WIPE_DATA ;
	}
	else if (3 == selected)
	{
		wipe_mark = WIPE_ALL_DATA;

	}
	else
	{
		current_directory=current_directory->father;
	}
	mgui_update_screen();
	
}
void mg_mount_storage()
{
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[1];
	mgui_update_screen();
	mg_show_partition_menu();
	//mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->father;
	mgui_update_screen();

}
void cancel_mount_usb_storage()
{
    cancel_mount_usb_storage_contral();
}
void mg_back_and_restore_system()
{
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[0];
	mgui_update_screen();

}

void mg_backup_system()
{
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->children[0];
	mgui_update_screen();
}

void mg_restore_system()
{
	current_directory =current_directory->children[2];
	char* chosen_path = NULL;
    char* primary_path = get_primary_storage_path();
    char* extra_paths = get_extra_storage_paths();
    if (ensure_path_mounted(extra_paths) == 0) {
        chosen_path = extra_paths;
    }
    else{
        chosen_path = primary_path;
        if (ensure_path_mounted(primary_path) !=0)
        {
            printf("no spcae\n");
            return 0;   
        }
    }
    char backup_path[PATH_MAX];
    sprintf(backup_path, "%s/.mgyun/backup/", chosen_path);
	char *file = mg_choose_file_menu(backup_path,NULL,NULL);
	 if (file==NULL)
    {
        mg_clear_plane(current_directory->show_page);
        current_directory=current_directory->father;
        mgui_update_screen();
    }
    else
    {
        mg_clear_plane(current_directory->show_page);
        current_directory=current_directory->father;
        current_directory=current_directory->children[1];
        mgui_update_screen();
        strcpy(update_zip_path,file);
        printf("restore path:%s\n",update_zip_path);
    }
}

void mg_backup_system_start()
{
	mg_clear_plane(current_directory->show_page);
	mg_draw_tiptext(gBackuping_system_tiptex);
	ui_show_indeterminate_progress();
	mg_backup_system_to_sdcard();
	ui_reset_progress();
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->father;
	mgui_update_screen();

}
void mg_restore_system_start()
{
	mg_clear_plane(current_directory->show_page);
	mg_draw_tiptext(gRestoning_system_tiptex);
	ui_show_indeterminate_progress();
	mg_restore_system_to_sdcard(update_zip_path);
	ui_reset_progress();
	mg_clear_plane(current_directory->show_page);
	current_directory = current_directory->father;
	mgui_update_screen();
}
void mg_first_screen()
{
	int x,y,dx,dy;
	x=0;
	y=gr_fb_height() *23 /100;
	dx=gr_fb_width();
	dy=gr_fb_height() *2/3;
	Mg_Plane  *main_menu = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

	root_page = calloc(sizeof(Mg_Page_tree),1);
	root_page->father=NULL;
	root_page->children[0]=NULL;
	root_page->show_page=main_menu;
	root_page->button_nun=2;
	root_page->button_selected=1;
	current_directory = root_page;

	x=0;
	y=0;
	dx=gr_get_width(gChoose_language);
	dy=gr_get_height(gChoose_language);
	add_widget_to_plane(main_menu,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gChoose_language,NULL,NULL);

	x=0;
	y=gr_fb_height() * 22/100;
	dx=gr_get_width(gLang_ch[0]);
	dy=gr_get_height(gLang_ch[0]);
	add_widget_to_plane(main_menu,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gLang_ch[0],gLang_ch[1],mg_confirm_language);
	
	x=0;
	y=y+ gr_fb_height() * 1125/10000;
	dx=gr_get_width(gLang_en[0]);
	dy=gr_get_height(gLang_en[0]);
	add_widget_to_plane(main_menu,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gLang_en[0],gLang_en[1],mg_confirm_language);
	
	x=0;
	y=gr_fb_height() *9/10 - gr_get_height(gMenu_text_tip) - get_plane_y(main_menu);
	dx=gr_get_width(gMenu_text_tip);
	dy=gr_get_height(gMenu_text_tip);
	add_widget_to_plane(main_menu,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gMenu_text_tip,NULL,NULL);
	mg_show_ui_plane(current_directory);
	
}

void creat_main_menu_page()
{
	Mg_Page_tree *main_menu_tree;
	Mg_Page_tree *upgrade_method_tree;
	Mg_Page_tree *wipe_date_tree;
	Mg_Page_tree *advanced_settings_tree;
	Mg_Page_tree *backup_restore_tree;
	int x,y,dx,dy;
	{
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *menu_page = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		main_menu_tree= calloc(sizeof(Mg_Page_tree),1);
		main_menu_tree->father=root_page;
		main_menu_tree->children[0]=NULL;
		main_menu_tree->show_page=menu_page;
		main_menu_tree->button_nun=5;
		main_menu_tree->button_selected=1;
		current_directory = main_menu_tree;
		root_page->children[0]=main_menu_tree;
		root_page->children[1]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gRecovery_mian_menu);
		dy=gr_get_height(gRecovery_mian_menu);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_mian_menu,NULL,NULL);
		
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRecovery_update[1]);
		dy=gr_get_height(gRecovery_update[1]);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gRecovery_update[0],gRecovery_update[1],mg_upgrade_method_choose);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRecovery_wipe_data[0]);
		dy=gr_get_height(gRecovery_wipe_data[0]);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gRecovery_wipe_data[0],gRecovery_wipe_data[1],mg_wipe_data_main);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gadvanced_settings[0]);
		dy=gr_get_height(gadvanced_settings[0]);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gadvanced_settings[0],gadvanced_settings[1],mg_advanced_settings);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRecovery_reboot_system[0]);
		dy=gr_get_height(gRecovery_reboot_system[0]);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gRecovery_reboot_system[0],gRecovery_reboot_system[1],mg_reboot_system);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRecovery_shutdown[0]);
		dy=gr_get_height(gRecovery_shutdown[0]);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gRecovery_shutdown[0],gRecovery_shutdown[1],mg_power_off);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(menu_page,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
	}
	{
		//Mg_Page_tree *upgrade_method_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *upgrade_method_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		upgrade_method_tree= calloc(sizeof(Mg_Page_tree),1);
		upgrade_method_tree->father=main_menu_tree;
		upgrade_method_tree->children[0]=NULL;
		upgrade_method_tree->show_page=upgrade_method_plane;
		upgrade_method_tree->button_nun=4;
		upgrade_method_tree->button_selected=1;
		main_menu_tree->children[0]=upgrade_method_tree;
		main_menu_tree->children[1]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gUpgrade_method_text);
		dy=gr_get_height(gUpgrade_method_text);
		add_widget_to_plane(upgrade_method_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gUpgrade_method_text,NULL,NULL);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gFrom_emmc_update_button[0]);
		dy=gr_get_height(gFrom_emmc_update_button[0]);

		add_widget_to_plane(upgrade_method_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gFrom_emmc_update_button[0],gFrom_emmc_update_button[1],mg_upgrade_from_emmc);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gFrom_sdcard_update_button[0]);
		dy=gr_get_height(gFrom_sdcard_update_button[0]);
		add_widget_to_plane(upgrade_method_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gFrom_sdcard_update_button[0],gFrom_sdcard_update_button[1],mg_upgrade_from_sdcard);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gSideload_mode_button[0]);
		dy=gr_get_height(gSideload_mode_button[0]);
		add_widget_to_plane(upgrade_method_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gSideload_mode_button[0],gSideload_mode_button[1],mg_upgrade_from_sideload_mode);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gBack_button[0]);
		dy=gr_get_height(gBack_button[0]);
		add_widget_to_plane(upgrade_method_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBack_button[0],gBack_button[1],mg_return_previous_screen);

		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(upgrade_method_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);

	}
	{
		
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *wipe_data_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		wipe_date_tree= calloc(sizeof(Mg_Page_tree),1);
		wipe_date_tree->father=main_menu_tree;
		wipe_date_tree->children[0]=NULL;
		wipe_date_tree->show_page=wipe_data_plane;
		wipe_date_tree->button_nun=4;
		wipe_date_tree->button_selected=1;
		main_menu_tree->children[1]=wipe_date_tree;
		main_menu_tree->children[2]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gWipe_data_text);
		dy=gr_get_height(gWipe_data_text);
		add_widget_to_plane(wipe_data_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gWipe_data_text,NULL,NULL);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gWipe_cache_button[0]);
		dy=gr_get_height(gWipe_cache_button[0]);

		add_widget_to_plane(wipe_data_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gWipe_cache_button[0],gWipe_cache_button[1],mg_wipe_data);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gWipe_data_button[0]);
		dy=gr_get_height(gWipe_data_button[0]);
		add_widget_to_plane(wipe_data_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gWipe_data_button[0],gWipe_data_button[1],mg_wipe_data);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gWipe_all_button[0]);
		dy=gr_get_height(gWipe_all_button[0]);
		add_widget_to_plane(wipe_data_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gWipe_all_button[0],gWipe_all_button[1],mg_wipe_data);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gBack_button[0]);
		dy=gr_get_height(gBack_button[0]);
		add_widget_to_plane(wipe_data_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBack_button[0],gBack_button[1],mg_return_previous_screen);

		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(wipe_data_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);

	}
	
	{
		
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *advanced_settings_page = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		advanced_settings_tree= calloc(sizeof(Mg_Page_tree),1);
		advanced_settings_tree->father=main_menu_tree;
		advanced_settings_tree->children[0]=NULL;
		advanced_settings_tree->show_page=advanced_settings_page;
		advanced_settings_tree->button_nun=3;
		advanced_settings_tree->button_selected=1;
		main_menu_tree->children[2]=advanced_settings_tree;
		main_menu_tree->children[3]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gAdvanced_settings_text);
		dy=gr_get_height(gAdvanced_settings_text);
		add_widget_to_plane(advanced_settings_page,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gAdvanced_settings_text,NULL,NULL);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gBackup_restore_button[0]);
		dy=gr_get_height(gBackup_restore_button[0]);

		add_widget_to_plane(advanced_settings_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBackup_restore_button[0],gBackup_restore_button[1],mg_back_and_restore_system);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gMount_storage_button[0]);
		dy=gr_get_height(gMount_storage_button[0]);
		add_widget_to_plane(advanced_settings_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gMount_storage_button[0],gMount_storage_button[1],mg_mount_storage);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gBack_button[0]);
		dy=gr_get_height(gBack_button[0]);
		add_widget_to_plane(advanced_settings_page,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBack_button[0],gBack_button[1],mg_return_previous_screen);

		x=0;
		y=y+gr_fb_height()*3*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(advanced_settings_page,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);

	}

	{
		Mg_Page_tree *select_updatezip_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *select_updatezip_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		select_updatezip_tree= calloc(sizeof(Mg_Page_tree),1);
		select_updatezip_tree->father=upgrade_method_tree;
		select_updatezip_tree->children[0]=NULL;
		select_updatezip_tree->show_page=select_updatezip_plane;
		select_updatezip_tree->button_nun=BUTTON_BOX_NO_BUTTON;
		select_updatezip_tree->button_selected=0;
		upgrade_method_tree->children[0]=select_updatezip_tree;
		upgrade_method_tree->children[1]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gSelect_updatezip_text);
		dy=gr_get_height(gSelect_updatezip_text);
		add_widget_to_plane(select_updatezip_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gSelect_updatezip_text,NULL,NULL);
	}

	{

		Mg_Page_tree *update_confirm_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *update_confirm_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		update_confirm_tree= calloc(sizeof(Mg_Page_tree),1);
		update_confirm_tree->father=upgrade_method_tree;
		update_confirm_tree->children[0]=NULL;
		update_confirm_tree->show_page=update_confirm_plane;
		update_confirm_tree->button_nun=2;
		update_confirm_tree->button_selected=1;
		upgrade_method_tree->children[1]=update_confirm_tree;
		upgrade_method_tree->children[2]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gUpdate_confirm_tips);
		dy=gr_get_height(gUpdate_confirm_tips);
		add_widget_to_plane(update_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gUpdate_confirm_tips,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*3*1125/10000;
		dx=gr_get_width(gCancel[0]);
		dy=gr_get_height(gCancel[0]);

		add_widget_to_plane(update_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gCancel[0],gCancel[1],mg_return_previous_screen);
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gConfirm_button_ture[0]);
		dy=gr_get_height(gConfirm_button_ture[0]);

		add_widget_to_plane(update_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gConfirm_button_ture[0],gConfirm_button_ture[1],mg_install_update_zip);
		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(update_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
		

	}
	{

		Mg_Page_tree *update_success_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *update_success_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		update_success_tree= calloc(sizeof(Mg_Page_tree),1);
		update_success_tree->father=upgrade_method_tree;
		update_success_tree->children[0]=NULL;
		update_success_tree->show_page=update_success_plane;
		update_success_tree->button_nun=2;
		update_success_tree->button_selected=1;
		upgrade_method_tree->children[2]=update_success_tree;
		upgrade_method_tree->children[3]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gUpdate_success_tips);
		dy=gr_get_height(gUpdate_success_tips);
		add_widget_to_plane(update_success_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gUpdate_success_tips,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*3*1125/10000;
		dx=gr_get_width(gConfirm_button_ture[0]);
		dy=gr_get_height(gConfirm_button_ture[0]);

		add_widget_to_plane(update_success_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gConfirm_button_ture[0],gConfirm_button_ture[1],mg_reboot_system);
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gCancel[0]);
		dy=gr_get_height(gCancel[0]);

		add_widget_to_plane(update_success_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gCancel[0],gCancel[1],mg_return_previous_screen);
		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(update_success_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
		

	}
	{

		Mg_Page_tree *update_fail_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *update_fail_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		update_fail_tree= calloc(sizeof(Mg_Page_tree),1);
		update_fail_tree->father=upgrade_method_tree;
		update_fail_tree->children[0]=NULL;
		update_fail_tree->show_page=update_fail_plane;
		update_fail_tree->button_nun=1;
		update_fail_tree->button_selected=1;
		upgrade_method_tree->children[3]=update_fail_tree;
		upgrade_method_tree->children[4]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gUpdate_fail_tips);
		dy=gr_get_height(gUpdate_fail_tips);
		add_widget_to_plane(update_fail_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gUpdate_fail_tips,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*4*1125/10000;
		dx=gr_get_width(gBack_button[0]);
		dy=gr_get_height(gBack_button[0]);

		add_widget_to_plane(update_fail_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBack_button[0],gBack_button[1],mg_return_previous_screen);
		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(update_fail_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
		

	}
	{
		Mg_Page_tree *wipe_confirm_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *wipe_confirm_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		wipe_confirm_tree= calloc(sizeof(Mg_Page_tree),1);
		wipe_confirm_tree->father=wipe_date_tree;
		wipe_confirm_tree->children[0]=NULL;
		wipe_confirm_tree->show_page=wipe_confirm_plane;
		wipe_confirm_tree->button_nun=2;
		wipe_confirm_tree->button_selected=1;
		wipe_date_tree->children[0]=wipe_confirm_tree;
		wipe_date_tree->children[1]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gWipe_confirm_text);
		dy=gr_get_height(gWipe_confirm_text);
		add_widget_to_plane(wipe_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gWipe_confirm_text,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*3*1125/10000;
		dx=gr_get_width(gCancel[0]);
		dy=gr_get_height(gCancel[0]);

		add_widget_to_plane(wipe_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gCancel[0],gCancel[1],mg_return_previous_screen);
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gConfirm_button_ture[0]);
		dy=gr_get_height(gConfirm_button_ture[0]);

		add_widget_to_plane(wipe_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gConfirm_button_ture[0],gConfirm_button_ture[1],mg_wipe_confirm_choose);
		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(wipe_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);

	}

	{
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *backup_restore_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		backup_restore_tree= calloc(sizeof(Mg_Page_tree),1);
		backup_restore_tree->father=advanced_settings_tree;
		backup_restore_tree->children[0]=NULL;
		backup_restore_tree->show_page=backup_restore_plane;
		backup_restore_tree->button_nun=3;
		backup_restore_tree->button_selected=1;
		advanced_settings_tree->children[0]=backup_restore_tree;
		advanced_settings_tree->children[1]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gBackup_and_restore_tips);
		dy=gr_get_height(gBackup_and_restore_tips);
		add_widget_to_plane(backup_restore_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gBackup_and_restore_tips,NULL,NULL);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gBackup_system_button[0]);
		dy=gr_get_height(gBackup_system_button[0]);
		add_widget_to_plane(backup_restore_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBackup_system_button[0],gBackup_system_button[1],mg_backup_system);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRestore_system_button[0]);
		dy=gr_get_height(gRestore_system_button[0]);
		add_widget_to_plane(backup_restore_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gRestore_system_button[0],gRestore_system_button[1],mg_restore_system);

		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gBack_button[0]);
		dy=gr_get_height(gBack_button[0]);
		add_widget_to_plane(backup_restore_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBack_button[0],gBack_button[1],mg_return_previous_screen);

		x=0;
		y=y+gr_fb_height()* 3* 1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(backup_restore_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);


	}
	{
		Mg_Page_tree *mount_storage_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *mount_storage_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		mount_storage_tree= calloc(sizeof(Mg_Page_tree),1);
		mount_storage_tree->father=advanced_settings_tree;
		mount_storage_tree->children[0]=NULL;
		mount_storage_tree->show_page=mount_storage_plane;
		mount_storage_tree->button_nun=BUTTON_BOX_NO_BUTTON;
		mount_storage_tree->button_selected=0;
		advanced_settings_tree->children[1]=mount_storage_tree;
		advanced_settings_tree->children[2]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gMount_storage_text);
		dy=gr_get_height(gMount_storage_text);
		add_widget_to_plane(mount_storage_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gMount_storage_text,NULL,NULL);
	}
	{
		Mg_Page_tree *mounted_storage_tree;
		x=0;
		y=gr_fb_height() * 11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *mounted_storage_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		mounted_storage_tree= calloc(sizeof(Mg_Page_tree),1);
		mounted_storage_tree->father=advanced_settings_tree;
		mounted_storage_tree->children[0]=NULL;
		mounted_storage_tree->show_page=mounted_storage_plane;
		mounted_storage_tree->button_nun=1;
		mounted_storage_tree->button_selected=1;
		advanced_settings_tree->children[2]=mounted_storage_tree;
		advanced_settings_tree->children[3]=NULL;

		x=0;
		y=gr_fb_height()*1125/10000;
		dx=gr_get_width(gMount_storage);
		dy=gr_get_height(gMount_storage);
		add_widget_to_plane(mounted_storage_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gMount_storage,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*4*1125/10000;
		dx=gr_get_width(gBack_button[0]);
		dy=gr_get_height(gBack_button[0]);
		add_widget_to_plane(mounted_storage_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gBack_button[0],gBack_button[1],cancel_mount_usb_storage);
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(mounted_storage_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
	}
	{

		Mg_Page_tree *backup_confirm_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *backup_confirm_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		backup_confirm_tree= calloc(sizeof(Mg_Page_tree),1);
		backup_confirm_tree->father=backup_restore_tree;
		backup_confirm_tree->children[0]=NULL;
		backup_confirm_tree->show_page=backup_confirm_plane;
		backup_confirm_tree->button_nun=2;
		backup_confirm_tree->button_selected=1;
		backup_restore_tree->children[0]=backup_confirm_tree;
		backup_restore_tree->children[1]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gConfirm_Backup_tips);
		dy=gr_get_height(gConfirm_Backup_tips);
		add_widget_to_plane(backup_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gConfirm_Backup_tips,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*3*1125/10000;
		dx=gr_get_width(gCancel[0]);
		dy=gr_get_height(gCancel[0]);

		add_widget_to_plane(backup_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gCancel[0],gCancel[1],mg_return_previous_screen);
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gConfirm_button_ture[0]);
		dy=gr_get_height(gConfirm_button_ture[0]);

		add_widget_to_plane(backup_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gConfirm_button_ture[0],gConfirm_button_ture[1],mg_backup_system_start);
		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(backup_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
		

	}
	{

		Mg_Page_tree *restore_confirm_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *restore_confirm_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		restore_confirm_tree= calloc(sizeof(Mg_Page_tree),1);
		restore_confirm_tree->father=backup_restore_tree;
		restore_confirm_tree->children[0]=NULL;
		restore_confirm_tree->show_page=restore_confirm_plane;
		restore_confirm_tree->button_nun=2;
		restore_confirm_tree->button_selected=1;
		backup_restore_tree->children[1]=restore_confirm_tree;
		backup_restore_tree->children[2]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gConfirm_restore_tips);
		dy=gr_get_height(gConfirm_restore_tips);
		add_widget_to_plane(restore_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gConfirm_restore_tips,NULL,NULL);
		x=0;
		y=y+gr_fb_height()*3*1125/10000;
		dx=gr_get_width(gCancel[0]);
		dy=gr_get_height(gCancel[0]);

		add_widget_to_plane(restore_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gCancel[0],gCancel[1],mg_return_previous_screen);
		x=0;
		y=y+gr_fb_height()*1125/10000;
		dx=gr_get_width(gConfirm_button_ture[0]);
		dy=gr_get_height(gConfirm_button_ture[0]);

		add_widget_to_plane(restore_confirm_plane,x,y,dx,dy,WIDGET_BUTTON,CENTRE_ALIGN,gConfirm_button_ture[0],gConfirm_button_ture[1],mg_restore_system_start);
		x=0;
		y=y+gr_fb_height()*2*1125/10000;
		dx=gr_get_width(gRecovery_main_menu_text_tip);
		dy=gr_get_height(gRecovery_main_menu_text_tip);
		add_widget_to_plane(restore_confirm_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gRecovery_main_menu_text_tip,NULL,NULL);
		

	}
	{
		Mg_Page_tree *restore_choose_box_tree;
		x=0;
		y=gr_fb_height() *11 /100;
		dx=gr_fb_width();
		dy=gr_fb_height() *79/100;
		Mg_Plane *restore_choose_box_plane = new_plane(x,y,dx,dy,CENTRE_ALIGN,NULL);

		restore_choose_box_tree= calloc(sizeof(Mg_Page_tree),1);
		restore_choose_box_tree->father=backup_restore_tree;
		restore_choose_box_tree->children[0]=NULL;
		restore_choose_box_tree->show_page=restore_choose_box_plane;
		restore_choose_box_tree->button_nun=BUTTON_BOX_NO_BUTTON;
		restore_choose_box_tree->button_selected=0;
		backup_restore_tree->children[2]=restore_choose_box_tree;
		backup_restore_tree->children[3]=NULL;

		x=0;
		y=0;
		dx=gr_get_width(gBackup_and_restore_tips);
		dy=gr_get_height(gBackup_and_restore_tips);
		add_widget_to_plane(restore_choose_box_plane,x,y,dx,dy,WIDGET_LABEL,CENTRE_ALIGN,gBackup_and_restore_tips,NULL,NULL);
	}
	creat_mgui_frame();
	mg_show_ui_plane(current_directory);
}