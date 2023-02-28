/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../../inc/MarlinConfigPre.h"

#if HAS_TFT_LVGL_UI

#include "draw_ui.h"
#include <lv_conf.h>
#include "draw_endstop.h"

#include "../../../gcode/queue.h"
#include "../../../module/temperature.h"
#include "../../../inc/MarlinConfig.h"

extern lv_group_t *g;

#ifndef USE_NEW_LVGL_CONF
static lv_obj_t *scr;
#endif

enum {
  ID_T_PRE_HEAT = 1,
  ID_T_EXTRUCT,
  ID_T_MOV,
  ID_T_HOME,
  ID_T_LEVELING,
  ID_T_FILAMENT,
  ID_T_MORE,
  ID_T_RETURN,
  ID_T_GCODE,
  ID_T_ABOUT,
  ID_T_LANGUAGE,
  ID_T_ENDSTOP,
  ID_T_COOL,
  ID_T_WIFI,
  ID_T_TEMPSETTING,
  ID_T_VOICE,
};

#if ENABLED(MKS_TEST)
  extern uint8_t current_disp_ui;
#endif

static void event_handler(lv_obj_t *obj, lv_event_t event) {
  if (event != LV_EVENT_RELEASED) return;
  // if (TERN1(AUTO_BED_LEVELING_BILINEAR, obj->mks_obj_id != ID_T_LEVELING))

  voice_button_on();
  _delay_ms(100);
  WRITE(BEEPER_PIN, LOW);

  // lv_clear_tool();
  switch (obj->mks_obj_id) {
    case ID_T_PRE_HEAT: lv_clear_tool();lv_draw_preHeat(); break;
    case ID_T_EXTRUCT:  lv_clear_tool();lv_draw_extrusion(); break;
    case ID_T_MOV:      lv_clear_tool();lv_draw_move_motor(); break;
    case ID_T_HOME:     lv_clear_tool();lv_draw_home(); break;
    case ID_T_LEVELING:
      #if ENABLED(AUTO_BED_LEVELING_BILINEAR)
        queue.inject_P(PSTR("G91"));
        queue.inject_P(PSTR("G1 Z5 F1000"));
        queue.inject_P(PSTR("G90"));
        lv_draw_dialog(DIALOG_TYPE_AUTO_LEVELING_TIPS);
        uiCfg.autoLeveling = 1;
        // get_gcode_command(AUTO_LEVELING_COMMAND_ADDR, (uint8_t *)public_buf_m);
        // public_buf_m[sizeof(public_buf_m) - 1] = 0;
        // queue.inject_P(PSTR(public_buf_m));
      #else
        lv_clear_tool();
        uiCfg.leveling_first_time = true;
        lv_draw_manualLevel();
      #endif
      break;
    case ID_T_FILAMENT:
      lv_clear_tool();
      uiCfg.hotendTargetTempBak = thermalManager.degTargetHotend(uiCfg.extruderIndex);
      lv_draw_filament_change();
      break;
    case ID_T_MORE:
      lv_clear_tool();
      lv_draw_more();
      break;
    case ID_T_COOL:
      // lv_draw_fan();
      //关闭加热
      uiCfg.curTempType = 0;
      thermalManager.setTargetHotend(0, uiCfg.extruderIndex);//設置溫度
      thermalManager.start_watching_hotend(uiCfg.extruderIndex);//檢測是否有降溫
      // disp_desire_temp();//刷新界面
      uiCfg.curTempType = 1;
      #if HAS_HEATED_BED
          //thermalManager.temp_bed.target = 0;
          thermalManager.setTargetBed(0);
          thermalManager.start_watching_bed();
      #endif
      
      #if 0
      //启动风扇
      thermalManager.set_fan_speed(0, map(100, 0, 100, 0, 255));
      // lv_draw_tool();
      #endif
      break;
    case ID_T_TEMPSETTING:
      lv_clear_tool();
      lv_draw_tempsetting();
      break;
    case ID_T_WIFI:
      lv_clear_tool();
      if (gCfgItems.wifi_mode_sel == STA_MODEL) {
          if (wifi_link_state == WIFI_CONNECTED) {
            last_disp_state = SET_UI;
            lv_draw_wifi();
          }
          else {
            if (uiCfg.command_send) {
              uint8_t cmd_wifi_list[] = { 0xA5, 0x07, 0x00, 0x00, 0xFC };
              raw_send_to_wifi(cmd_wifi_list, COUNT(cmd_wifi_list));
              last_disp_state = SET_UI;
              lv_draw_wifi_list();
            }
            else {
              last_disp_state = SET_UI;
              lv_draw_dialog(DIALOG_WIFI_ENABLE_TIPS);
            }
          }
        }
      else {
        last_disp_state = SET_UI;

        lv_draw_wifi();
      }
      break;
    case ID_T_ENDSTOP:
      lv_clear_tool();
      lv_draw_endstop();
      break;
    case ID_T_VOICE:
      lv_clear_tool();
      lv_draw_voice();
      break;
    case ID_T_RETURN:
      lv_clear_tool();
      TERN_(MKS_TEST, current_disp_ui = 1);
      lv_draw_ready_print();
      break;
    case ID_T_GCODE: 
      lv_clear_tool();
      // TERN(MULTI_VOLUME, lv_draw_media_select(), lv_draw_print_file()); 
      // lv_draw_gcade();
      keyboard_value = GCade;
      // keyboard_value = GCodeCommand;
      lv_draw_keyboard();
      break;
    case ID_T_ABOUT:
      lv_clear_tool();
      lv_draw_about();
      break;
    case ID_T_LANGUAGE:
      lv_clear_tool();
      lv_draw_language();
      break;
    default: 
      break;
  }
}

void lv_draw_tool() {
#ifdef USE_NEW_LVGL_CONF
  mks_ui.src_main = lv_set_scr_id_title(mks_ui.src_main, TOOL_UI, "");
#else
  scr = lv_screen_create(TOOL_UI);
#endif
#ifdef USE_NEW_LVGL_CONF
  lv_big_button_create(mks_ui.src_main, "F:/bmp_preHeat.bin", tool_menu.preheat, INTERVAL_V, titleHeight, event_handler, ID_T_PRE_HEAT);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_extruct.bin", tool_menu.extrude, BTN_X_PIXEL + INTERVAL_V * 2, titleHeight, event_handler, ID_T_EXTRUCT);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_mov.bin", tool_menu.move, BTN_X_PIXEL * 2 + INTERVAL_V * 3, titleHeight, event_handler, ID_T_MOV);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_zero.bin", tool_menu.home, BTN_X_PIXEL * 3 + INTERVAL_V * 4, titleHeight, event_handler, ID_T_HOME);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_leveling.bin", tool_menu.TERN(AUTO_BED_LEVELING_BILINEAR, autoleveling, leveling), INTERVAL_V, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_T_LEVELING);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_filamentchange.bin", tool_menu.filament, BTN_X_PIXEL + INTERVAL_V * 2, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_T_FILAMENT);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_more.bin", tool_menu.more, BTN_X_PIXEL * 2 + INTERVAL_V * 3, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_T_MORE);
  lv_big_button_create(mks_ui.src_main, "F:/bmp_return.bin", common_menu.text_back, BTN_X_PIXEL * 3 + INTERVAL_V * 4, BTN_Y_PIXEL + INTERVAL_H + titleHeight, event_handler, ID_T_RETURN);
#else
  //冷却
  lv_obj_t *btnCool = lv_imgbtn_create(scr, "F:/bmp_tool_cool.bin", event_handler, ID_T_COOL);
  lv_obj_set_pos(btnCool, 241, 9);
  lv_obj_t *labelCool = lv_label_create_empty(btnCool);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelCool, tool_menu.cool);
    lv_label_set_style(labelCool,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelCool, btnCool, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }
  
  //WIFI
  lv_obj_t *btnWifi = lv_imgbtn_create(scr, "F:/bmp_wifi.bin", event_handler, ID_T_WIFI);
  lv_obj_set_pos(btnWifi, 357, 9);
  lv_obj_t *labelWifi = lv_label_create_empty(btnWifi);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelWifi, "WiFi");
    lv_label_set_style(labelWifi,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelWifi, btnWifi, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //温度设置
  lv_obj_t *btnTempsetting = lv_imgbtn_create(scr, "F:/bmp_tool_tempset.bin", event_handler, ID_T_TEMPSETTING);
  lv_obj_set_pos(btnTempsetting, 9, 127);
  lv_obj_t *labelTempsetting = lv_label_create_empty(btnTempsetting);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelTempsetting, tool_menu.tempsetting);
    lv_label_set_style(labelTempsetting,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelTempsetting, btnTempsetting, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //voice
  lv_obj_t *btnVoice = lv_imgbtn_create(scr, "F:/bmp_tool_voice.bin", event_handler, ID_T_VOICE);
  lv_obj_set_pos(btnVoice, 125, 127);
  lv_obj_t *labelVoice = lv_label_create_empty(btnVoice);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelVoice, tool_menu.voice);
    lv_label_set_style(labelVoice,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelVoice, btnVoice, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //语言设置
  lv_obj_t *btnLanguage = lv_imgbtn_create(scr, "F:/bmp_language.bin", event_handler, ID_T_LANGUAGE);
  lv_obj_set_pos(btnLanguage, 241, 127);
  lv_obj_t *labelLanguage = lv_label_create_empty(btnLanguage);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelLanguage, set_menu.language);
    lv_label_set_style(labelLanguage,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelLanguage, btnLanguage, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  lv_obj_t *btnBack = lv_imgbtn_create(scr, "F:/bmp_tool_back.bin", event_handler, ID_T_RETURN);
  lv_obj_set_pos(btnBack, 357, 127);
  lv_obj_t *labelBack = lv_label_create_empty(btnBack);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelBack, common_menu.text_back);
    lv_label_set_style(labelBack,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelBack, btnBack, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }
  
  //Gcode
  lv_obj_t *btnGcode = lv_imgbtn_create(scr, "F:/bmp_tool_gcode.bin", event_handler, ID_T_GCODE);
  lv_obj_set_pos(btnGcode, 9, 245);
  lv_obj_t *labelGcode = lv_label_create_empty(btnGcode);
  lv_label_set_text(labelGcode, more_menu.gcode);
    lv_label_set_style(labelGcode,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelGcode, btnGcode, 0, 35, 0);

  //about
  lv_obj_t *btnAbout = lv_imgbtn_create(scr, "F:/bmp_tool_about.bin", event_handler, ID_T_ABOUT);
  lv_obj_set_pos(btnAbout, 241, 245);
  lv_obj_t *labelAbout = lv_label_create_empty(btnAbout);
  lv_label_set_text(labelAbout, set_menu.about);
    lv_label_set_style(labelAbout,LV_LABEL_STYLE_MAIN,&label_dialog_white);
  lv_obj_align(labelAbout, btnAbout, 0, 20, 0);
  lv_refr_now(lv_refr_get_disp_refreshing());
  
  //调平
  lv_obj_t *btnLeveling = lv_imgbtn_create(scr, "F:/bmp_leveling.bin", event_handler, ID_T_LEVELING);
  lv_obj_set_pos(btnLeveling, 9, 9);
  lv_obj_t *labelLeveling = lv_label_create_empty(btnLeveling);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelLeveling, tool_menu.leveling);
    lv_label_set_style(labelLeveling,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelLeveling, btnLeveling, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }

  //限位
  lv_obj_t *btnEndStop = lv_imgbtn_create(scr, "F:/bmp_tool_endstop.bin", event_handler, ID_T_ENDSTOP);
  lv_obj_set_pos(btnEndStop, 125, 9);
  lv_obj_t *labelEndStop = lv_label_create_empty(btnEndStop);
  if (gCfgItems.multiple_language) {
    lv_label_set_text(labelEndStop, tool_menu.endstop);
    lv_label_set_style(labelEndStop,LV_LABEL_STYLE_MAIN,&label_dialog_white);
    lv_obj_align(labelEndStop, btnEndStop, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  }
#endif
}

void lv_clear_tool() {
  #if HAS_ROTARY_ENCODER
    if (gCfgItems.encoder_enable) lv_group_remove_all_objs(g);
  #endif
#ifdef USE_NEW_LVGL_CONF
  lv_obj_clean(mks_ui.src_main);
#else
  lv_obj_del(scr);
#endif
}

#endif // HAS_TFT_LVGL_UI
