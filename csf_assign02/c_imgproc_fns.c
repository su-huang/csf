// C implementations of image processing functions

#include <stdlib.h>
#include <assert.h>
#include "imgproc.h"

// TODO: define your helper functions here

//! Transform the entire image by shrinking it down both 
//! horizontally and vertically (by potentially different
//! factors). This is equivalent to sampling the orignal image
//! for every pixel that is in certain rows and columns as 
//! specified in the function inputs.
//!
//! Take the image below where each letter corresponds to a pixel
//!
//!                 XAAAYBBB
//!                 AAAABBBB
//!                 ZCCCWDDD
//!                 CCCCDDDD
//!
//! If the user specified to shrink it horazontally by a factor 
//! of 4 and shrink it vertically by a factor of 2, you would 
//! sample pixel that had a row index such that 
//!
//!             row index % 2 = 0 
//!
//! and a column index such that
//!
//!             column index % 4 = 0
//!
//! in the above example, this would mean pixels that are in 
//! rows 0 and 2 with columns 0 and 4. 
//! The resultant image is:
//!
//!                 XY
//!                 ZW
//! 
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
//! @param xfac factor to downsize the image horizontally; guaranteed to be positive
//! @param yfac factor to downsize the image vertically; guaranteed to be positive
void imgproc_squash( struct Image *input_img, struct Image *output_img, int32_t xfac, int32_t yfac ) {
  // TODO: implement
  int status = img_init(output_img, input_img->width / xfac, input_img->height / yfac);
  if (status != IMG_SUCCESS) {
    return; 
  }

  for (int32_t row = 0; row < output_img->height; row++) {
    for (int32_t col = 0; col < output_img->width; col++) {
      int32_t input_index = compute_index(input_img, row * yfac, col * xfac); 
      int32_t output_index = compute_index(output_img, row, col); 
      output_img->data[output_index] = input_img->data[input_index]; 
    }
  }
}

//! Transform the color component values in each input pixel
//! by applying a rotation on the values of the color components
//! I.e. the old pixel's red component value will be used for
//! the new pixel's green component value, the old pixel's green
//! component value will be used new pixel's blue component value
//! and the old pixel's blue component value will be used new 
//! pixel's red component value. The alpha value should not change.
//! For instance, if a pixel had the hex value 0xAABBCCDD, the 
//! transformed pixel would become 0xCCAABBDD
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_color_rot( struct Image *input_img, struct Image *output_img) {
  // TODO: implement
  int status = img_init(output_img, input_img->width, input_img->height);
  if (status != IMG_SUCCESS) {
    return; 
  }

  for (int32_t row = 0; row < input_img->height; row++) {
    for (int32_t col = 0; col < input_img->width; col++) {
      int32_t index = compute_index(input_img, row, col);  
      output_img->data[index] = rot_colors(input_img, index); 
    }
  }
}

//! Transform the input image using a blur effect.
//!
//! Each pixel of the output image should have its color components
//! determined by taking the average of the color components of pixels
//! within blur_dist number of pixels horizontally and vertically from
//! the pixel's location in the original image. For example, if
//! blur_dist is 0, then only the original pixel is considered, and the
//! the output image should be identical to the input image. If blur_dist
//! is 1, then the original pixel and the 8 pixels immediately surrounding
//! it would be considered, etc.  Pixels positions not within the bounds of
//! the image should be ignored: i.e., their color components aren't
//! considered in the computation of the result pixels.
//!
//! The alpha value each output pixel should be identical to the
//! corresponding input pixel.
//!
//! Averages should be computed using purely integer arithmetic with
//! no rounding.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
//! @param blur_dist all pixels whose x/y coordinates are within
//!                  this many pixels of the x/y coordinates of the
//!                  original pixel should be included in the color
//!                  component averages used to determine the color
//!                  components of the output pixel
void imgproc_blur( struct Image *input_img, struct Image *output_img, int32_t blur_dist ) {
  // TODO: implement
  int status = img_init(output_img, input_img->width, input_img->height);
  if (status != IMG_SUCCESS) {
    return; 
  }

  for (int32_t row = 0; row < input_img->height; row++) {
    for (int32_t col = 0; col < input_img->width; col++) {
      uint32_t pixel = blur_pixel(input_img, row, col, blur_dist); 
      int32_t index = compute_index(output_img, row, col); 
      output_img->data[index] = pixel; 
    }
  }
}

//! The `expand` transformation doubles the width and height of the image.
//! 
//! Let's say that there are n rows and m columns of pixels in the
//! input image, so there are 2n rows and 2m columns in the output
//! image.  The pixel color and alpha value of the output pixel at row i and column
//! j should be computed as follows.
//! 
//! If both i and j are even, then the color and alpha value of the output
//! pixel are exactly the same as the input pixel at row i/2 and column j/2.
//! 
//! If i is even but j is odd, then the color components and alpha value
//! of the output pixel are computed as the average of those in the input pixels
//! in row i/2 at columns floor(j/2) and floor(j/2) + 1.
//! 
//! If i is odd and j is even, then the color components and alpha value
//! of the output pixel are computed as the average of those in the input pixels
//! in column j/2 at rows floor(i/2) and  floor(i/2) + 1.
//! 
//! If both i and j are odd then the color components and alpha value
//! of the output pixel are computed as the average of the input pixels
//! 
//! 1. At row floor(i/2) and column floor(j/2)
//! 2. At row floor(i/2) and column floor(j/2) + 1
//! 3. At row floor(i/2) + 1 and column floor(j/2)
//! 4. At row floor(i/2) + 1 and column floor(j/2) + 1
//! 
//! Note that in the cases where either i or j is odd, it is not
//! necessarily the case that either row floor(i/2) + 1 or
//! column floor(j/2) + 1 are in bounds in the input image.
//! Only input pixels that are properly in bounds should be incorporated into
//! the averages used to determine the color components and alpha value
//! of the output pixel.
//! 
//! Averages should be computed using purely integer arithmetic with
//! no rounding.
//!
//! @param input_img pointer to the input Image
//! @param output_img pointer to the output Image (in which the
//!                   transformed pixels should be stored)
void imgproc_expand( struct Image *input_img, struct Image *output_img) {
  // TODO: implement
  int status = img_init(output_img, input_img->width * 2, input_img->height * 2);
  if (status != IMG_SUCCESS) {
    return; 
  }

  for (int32_t row = 0; row < output_img->height; row++) {
    for (int32_t col = 0; col < output_img->width; col++) {
      uint32_t pixel; 
      if (row % 2 == 0 && col % 2 == 0) {
        pixel = expand_even_even(input_img, row, col); 
      } else if (row % 2 == 0 && col % 2 != 0) {
        pixel = expand_even_odd(input_img, row, col); 
      } else if (row % 2 != 0 && col % 2 == 0) {
        pixel = expand_odd_even(input_img, row, col); 
      } else {
        pixel = expand_odd_odd(input_img, row, col); 
      }

      int32_t index = compute_index(output_img, row, col); 
      output_img->data[index] = pixel; 
    }
  }
}

uint32_t get_r( uint32_t pixel ) {
  return (pixel >> 24) & 0xff; 
}

uint32_t get_g( uint32_t pixel ) {
  return (pixel >> 16) & 0xff;
}

uint32_t get_b( uint32_t pixel ) {
  return (pixel >> 8) & 0xff;
}

uint32_t get_a( uint32_t pixel ) {
  return pixel & 0xff;
}

uint32_t make_pixel( uint32_t r, uint32_t g, uint32_t b, uint32_t a ) {
  return (r << 24) | (g << 16) | (b << 8) | a;
}

int32_t valid_index( struct Image *img, int32_t row, int32_t col ) {
  if (row < 0 || row >= img->height || col < 0 || col >= img->width) {
    return 0; 
  }
  return 1; 
}

int32_t compute_index( struct Image *img, int32_t row, int32_t col ) {
  return row * img->width + col; 
}

uint32_t blur_pixel( struct Image *img, int32_t row, int32_t col, int32_t blur_dist ) {
  struct PixelAverager pa; 
  pa_init(&pa); 

  for (int32_t row_index = row - blur_dist; row_index <= row + blur_dist; row_index++) {
    for (int32_t col_index = col - blur_dist; col_index <= col + blur_dist; col_index++) {
      pa_update_from_img(img, row_index, col_index, &pa); 
    }
  }

  int32_t index = compute_index(img, row, col); 
  uint32_t a = get_a(img->data[index]); 

  return pa_avg_pixel(&pa, 1, a); 
}

uint32_t rot_colors( struct Image *img, int32_t index ) {
  uint32_t pixel = img->data[index];

  uint8_t a = get_a(pixel); 
  uint8_t new_r = get_b(pixel); 
  uint8_t new_b = get_g(pixel); 
  uint8_t new_g = get_r(pixel); 

  return make_pixel(new_r, new_g, new_b, a); 
}

uint32_t expand_even_even (struct Image *img, int32_t row, int32_t col ) {
  int32_t new_row = row / 2; 
  int32_t new_col = col / 2; 
  int32_t index = compute_index(img, new_row, new_col); 
  
  return (img->data[index]); 
}

uint32_t expand_even_odd (struct Image *img, int32_t row, int32_t col ) {
  int32_t new_row = row / 2; 
  int32_t new_col = col / 2; 

  struct PixelAverager pa; 
  pa_init(&pa); 
  pa_update_from_img(img, new_row, new_col, &pa); 
  pa_update_from_img(img, new_row, new_col + 1, &pa); 

  return pa_avg_pixel(&pa, 0, 0); 
}

uint32_t expand_odd_even (struct Image *img, int32_t row, int32_t col ) {
  int32_t new_row = row / 2; 
  int32_t new_col = col / 2; 

  struct PixelAverager pa; 
  pa_init(&pa); 
  pa_update_from_img(img, new_row, new_col, &pa); 
  pa_update_from_img(img, new_row + 1, new_col, &pa); 

  return pa_avg_pixel(&pa, 0, 0); 
}

uint32_t expand_odd_odd (struct Image *img, int32_t row, int32_t col ) {
  int32_t new_row = row / 2; 
  int32_t new_col = col / 2; 

  struct PixelAverager pa; 
  pa_init(&pa); 
  pa_update_from_img(img, new_row, new_col, &pa); 
  pa_update_from_img(img, new_row, new_col + 1, &pa); 
  pa_update_from_img(img, new_row + 1, new_col, &pa); 
  pa_update_from_img(img, new_row + 1, new_col + 1, &pa); 

  return pa_avg_pixel(&pa, 0, 0); 
}

void pa_init( struct PixelAverager *pa ) {
  pa->r = 0; 
  pa->g = 0; 
  pa->b = 0; 
  pa->a = 0; 
  pa->count = 0; 
}

void pa_update( struct PixelAverager *pa, uint32_t pixel ) {
  pa->r += get_r(pixel); 
  pa->g += get_g(pixel); 
  pa->b += get_b(pixel); 
  pa->a += get_a(pixel); 
  pa->count++; 
}

void pa_update_from_img( struct Image *img, int32_t row, int32_t col, struct PixelAverager *pa ) {
  if (valid_index(img, row, col)) {
    int32_t ind = compute_index(img, row, col); 
    pa_update(pa, img->data[ind]); 
  }
}

uint32_t pa_avg_pixel( struct PixelAverager *pa, uint32_t default_alpha, uint32_t img_alpha ) {
  if (pa->count == 0) {
    return make_pixel(pa->r, pa->g, pa->b, pa->a); 
  }

  uint32_t avg_r = pa->r / pa->count;
  uint32_t avg_g = pa->g / pa->count;
  uint32_t avg_b = pa->b / pa->count;
  uint32_t avg_a = pa->a / pa->count;

  if (default_alpha) {
    avg_a = img_alpha; 
  }
  
  return make_pixel(avg_r, avg_g, avg_b, avg_a);
}
