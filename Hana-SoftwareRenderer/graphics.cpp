#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "graphics.h"
#include "macro.h"
#include "maths.h"

/* framebuffer management */

framebuffer *framebuffer_create(int width, int height) {
    int color_buffer_size = width * height * 4;
    int depth_buffer_size = sizeof(float) * width * height;
    vec4_t default_color = {0, 0, 0, 1};
    float default_depth = 1;
    framebuffer* fb = new framebuffer();

    assert(width > 0 && height > 0);

    fb->width = width;
    fb->height = height;
    fb->color_buffer = (unsigned char*)malloc(color_buffer_size);
    fb->depth_buffer = (float*)malloc(depth_buffer_size);

    framebuffer_clear_color(fb, default_color);
    framebuffer_clear_depth(fb, default_depth);

    return fb;
}

void framebuffer_release(framebuffer *framebuffer) {
    free(framebuffer->color_buffer);
    free(framebuffer->depth_buffer);
    free(framebuffer);
}

void framebuffer_clear_color(framebuffer *framebuffer, vec4_t color) {
    int num_pixels = framebuffer->width * framebuffer->height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        framebuffer->color_buffer[i * 4 + 0] = float_to_uchar(color.x);
        framebuffer->color_buffer[i * 4 + 1] = float_to_uchar(color.y);
        framebuffer->color_buffer[i * 4 + 2] = float_to_uchar(color.z);
        framebuffer->color_buffer[i * 4 + 3] = float_to_uchar(color.w);
    }
}

void framebuffer_clear_depth(framebuffer *framebuffer, float depth) {
    int num_pixels = framebuffer->width * framebuffer->height;
    int i;
    for (i = 0; i < num_pixels; i++) {
        framebuffer->depth_buffer[i] = depth;
    }
}



void graphics_draw_triangle(framebuffer* framebuffer) {
    //int num_vertices;
    //int i;

    ///* execute vertex shader */
    //for (i = 0; i < 3; i++) {
    //    vec4_t clip_coord = program->vertex_shader(program->shader_attribs[i],
    //        program->in_varyings[i],
    //        program->shader_uniforms);
    //    program->in_coords[i] = clip_coord;
    //}

    ///* triangle clipping */
    //num_vertices = clip_triangle(program->sizeof_varyings,
    //    program->in_coords, program->in_varyings,
    //    program->out_coords, program->out_varyings);

    ///* triangle assembly */
    //for (i = 0; i < num_vertices - 2; i++) {
    //    int index0 = 0;
    //    int index1 = i + 1;
    //    int index2 = i + 2;
    //    vec4_t clip_coords[3];
    //    void* varyings[3];
    //    int is_culled;

    //    clip_coords[0] = program->out_coords[index0];
    //    clip_coords[1] = program->out_coords[index1];
    //    clip_coords[2] = program->out_coords[index2];
    //    varyings[0] = program->out_varyings[index0];
    //    varyings[1] = program->out_varyings[index1];
    //    varyings[2] = program->out_varyings[index2];

    //    is_culled = rasterize_triangle(framebuffer, program,
    //        clip_coords, varyings);
    //    if (is_culled) {
    //        break;
    //    }
    //}
}