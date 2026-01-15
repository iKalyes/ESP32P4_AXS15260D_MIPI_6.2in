/**
 * @file lv_draw_ppa_img.c
 *
 */

#include "lv_draw_ppa_private.h"
#include "lv_draw_ppa.h"

#if LV_USE_PPA

void lv_draw_ppa_img(lv_draw_task_t * t, const lv_draw_image_dsc_t * dsc,
                     const lv_area_t * coords)
{
    if(dsc->opa <= (lv_opa_t)LV_OPA_MIN) {
        return;
    }

    lv_draw_ppa_unit_t * u = (lv_draw_ppa_unit_t *)t->draw_unit;
    lv_draw_buf_t * draw_buf = t->target_layer->draw_buf;
    const lv_image_dsc_t * img_dsc = dsc->src;
    int width  = lv_area_get_width(coords);
    int height = lv_area_get_height(coords);

    int32_t offset_x = coords->x1 - t->target_layer->buf_area.x1;
    int32_t offset_y = coords->y1 - t->target_layer->buf_area.y1;

    ppa_blend_oper_config_t cfg = {
        .in_bg = {
            .buffer          = (void *)draw_buf->data,
            .pic_w           = draw_buf->header.w,
            .pic_h           = draw_buf->header.h,
            .block_w         = width,
            .block_h         = height,
            .block_offset_x  = offset_x,
            .block_offset_y  = offset_y,
            .blend_cm        = lv_color_format_to_ppa_blend(draw_buf->header.cf),
        },
        .bg_rgb_swap           = false,
        .bg_byte_swap          = false,
        .bg_alpha_update_mode  = PPA_ALPHA_NO_CHANGE,
        .bg_alpha_fix_val      = 0,
        .bg_ck_en              = false,

        .in_fg = {
            .buffer          = (void *)img_dsc->data,
            .pic_w           = img_dsc->header.w,
            .pic_h           = img_dsc->header.h,
            .block_w         = width,
            .block_h         = height,
            .block_offset_x  = 0,
            .block_offset_y  = 0,
            .blend_cm        = lv_color_format_to_ppa_blend(dsc->header.cf),
        },
        .fg_rgb_swap           = false,
        .fg_byte_swap          = false,
        .fg_alpha_update_mode = PPA_ALPHA_NO_CHANGE,
        .fg_alpha_fix_val      = 0xFF,
        .fg_ck_en              = false,

        .out = {
            .buffer          = draw_buf->data,
            .buffer_size     = PPA_ALIGN_UP(draw_buf->data_size, CONFIG_CACHE_L1_CACHE_LINE_SIZE),
            .pic_w           = draw_buf->header.w,
            .pic_h           = draw_buf->header.h,
            .block_offset_x  = offset_x,
            .block_offset_y  = offset_y,
            .blend_cm        = lv_color_format_to_ppa_blend(draw_buf->header.cf),
        },

        .mode            = PPA_TRANS_MODE_NON_BLOCKING,
        .user_data       = u,
    };

    esp_err_t ret = ppa_do_blend(u->blend_client, &cfg);
    if(ret != ESP_OK) {
        LV_LOG_WARN("PPA draw_img blend failed: %d", ret);
    }
}

#endif /* LV_USE_PPA */
