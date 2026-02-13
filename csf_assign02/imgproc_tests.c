/*
 * Tests for image manipulation and helper functions 
 * CSF Assignment 2
 * Su Huang 
 * shuan148@jh.edu
 */

 #include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tctest.h"
#include "imgproc.h"

// Maximum number of pixels in a test image
#define MAX_NUM_PIXELS 1500

// Test image data: used to represent small test
// images that can be directly embedded in the test
// source code
struct TestImageData {
  int32_t width, height;
  uint32_t pixels[MAX_NUM_PIXELS];
};

// Include test image data
#include "test_image_data.h"

// Data type for the test fixture object.
// This contains data (including Image objects) that
// can be accessed by test functions. This is useful
// because multiple test functions can access the same
// data (so you don't need to create/initialize that
// data multiple times in different test functions.)
typedef struct {
  // Test images (for basic image transform tests)
  struct Image smol, smol_squash_1_1, smol_squash_3_1, smol_squash_1_3,
               smol_color_rot, smol_blur_0, smol_blur_3, smol_expand;

} TestObjs;

// Functions to create and clean up a test fixture object
TestObjs *setup( void );
void cleanup( TestObjs *objs );

// Helper functions used by the test code
void init_image_from_testdata(struct Image *img, struct TestImageData *test_data);
struct Image *create_output_image( const struct Image *src_img );
bool images_equal( struct Image *a, struct Image *b );
void destroy_img( struct Image *img );

// Test functions
void test_squash_basic( TestObjs *objs );
void test_color_rot_basic( TestObjs *objs );
void test_blur_basic( TestObjs *objs );
void test_expand_basic( TestObjs *objs );

void test_get_rgba( TestObjs *objs ); 
void test_make_pixel( TestObjs *objs ); 
void test_valid_index( TestObjs *objs ); 
void test_compute_index( TestObjs *objs ); 
void test_blur_pixel( TestObjs *objs ); 
void test_rot_colors( TestObjs *objs ); 
void test_expand_even_even( TestObjs *objs ); 
void test_expand_even_odd( TestObjs *objs ); 
void test_expand_odd_even( TestObjs *objs ); 
void test_expand_odd_odd( TestObjs *objs ); 
void test_pa_init( TestObjs *objs ); 
void test_pa_update( TestObjs *objs ); 
void test_pa_update_from_img( TestObjs *objs ); 
void test_pa_avg_pixel( TestObjs *objs ); 

int main( int argc, char **argv ) {
  // allow the specific test to execute to be specified as the
  // first command line argument
  if ( argc > 1 )
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  // Run tests.
  // Make sure you add additional TEST() macro invocations
  // for any additional test functions you add.
  TEST( test_squash_basic );
  TEST( test_color_rot_basic );
  TEST( test_blur_basic );
  TEST( test_expand_basic );

  TEST( test_get_rgba ); 
  TEST( test_make_pixel ); 
  TEST( test_valid_index ); 
  TEST( test_compute_index ); 
  TEST( test_blur_pixel ); 
  TEST( test_rot_colors ); 
  TEST( test_expand_even_even );
  TEST( test_expand_even_odd ); 
  TEST( test_expand_odd_even ); 
  TEST( test_expand_odd_odd ); 
  TEST( test_pa_init ); 
  TEST( test_pa_update ); 
  TEST( test_pa_update_from_img ); 
  TEST( test_pa_avg_pixel ); 

  TEST_FINI();
}

////////////////////////////////////////////////////////////////////////
// Test fixture setup/cleanup functions
////////////////////////////////////////////////////////////////////////

TestObjs *setup( void ) {
  TestObjs *objs = (TestObjs *) malloc( sizeof(TestObjs) );

  // Initialize test Images from test image data
  init_image_from_testdata( &objs->smol, &smol );
  init_image_from_testdata( &objs->smol_squash_1_1, &smol_squash_1_1 );
  init_image_from_testdata( &objs->smol_squash_3_1, &smol_squash_3_1 );
  init_image_from_testdata( &objs->smol_squash_1_3, &smol_squash_1_3 );
  init_image_from_testdata( &objs->smol_color_rot, &smol_color_rot );
  init_image_from_testdata( &objs->smol_blur_0, &smol_blur_0 );
  init_image_from_testdata( &objs->smol_blur_3, &smol_blur_3 );
  init_image_from_testdata( &objs->smol_expand, &smol_expand );

  return objs;
}

void cleanup( TestObjs *objs ) {
  // Note that the test Images don't need to be cleaned
  // up, because their data isn't dynamically allocated

  free( objs );
}

////////////////////////////////////////////////////////////////////////
// Test code helper functions
////////////////////////////////////////////////////////////////////////

// Helper function to initialize an Image from
// a TestImageData instance. Note that the Image will
// point directly to the TestImageData's pixel data,
// so don't call img_cleanup() on the resulting Image.
void init_image_from_testdata(struct Image *img, struct TestImageData *test_data) {
  img->width = test_data->width;
  img->height = test_data->height;
  img->data = test_data->pixels;
}

// Helper function to create a temporary output Image
// the same size as a given one
struct Image *create_output_image( const struct Image *src_img ) {
  struct Image *img;
  img = malloc( sizeof( struct Image ) );
  img_init( img, src_img->width, src_img->height );
  return img;
}

// Returns true IFF both Image objects are identical
bool images_equal( struct Image *a, struct Image *b ) {
  if ( a->width != b->width || a->height != b->height )
    return false;

  for ( int i = 0; i < a->height; ++i )
    for ( int j = 0; j < a->width; ++j ) {
      int index = i*a->width + j;
      if ( a->data[index] != b->data[index] )
        return false;
    }

  return true;
}

void destroy_img( struct Image *img ) {
  if ( img != NULL )
    img_cleanup( img );
  free( img );
}

////////////////////////////////////////////////////////////////////////
// Test functions
////////////////////////////////////////////////////////////////////////

#define SQUASH_TEST( xfac, yfac ) \
do { \
  struct Image *out_img = create_output_image( &objs->smol_squash_##xfac##_##yfac ); \
  imgproc_squash( &objs->smol, out_img, xfac, yfac ); \
  ASSERT( images_equal( out_img, &objs->smol_squash_##xfac##_##yfac ) ); \
  destroy_img( out_img ); \
} while (0)

#define XFORM_TEST( xform ) \
do { \
  struct Image *out_img = create_output_image( &objs->smol_##xform ); \
  imgproc_##xform( &objs->smol, out_img ); \
  ASSERT( images_equal( out_img, &objs->smol_##xform ) ); \
  destroy_img( out_img ); \
} while (0)

#define BLUR_TEST( blur_dist) \
do { \
  struct Image *out_img = create_output_image( &objs->smol_blur_##blur_dist ); \
  imgproc_blur( &objs->smol, out_img, blur_dist ); \
  ASSERT( images_equal( out_img, &objs->smol_blur_##blur_dist ) ); \
  destroy_img( out_img ); \
} while (0)

void test_squash_basic( TestObjs *objs ) {
  SQUASH_TEST( 1, 1 );
  SQUASH_TEST( 3, 1 );
  SQUASH_TEST( 1, 3 );
}

void test_color_rot_basic( TestObjs *objs ) {
  XFORM_TEST( color_rot );
}

void test_blur_basic( TestObjs *objs ) {
  BLUR_TEST( 0 );
  BLUR_TEST( 3 );
}

void test_expand_basic( TestObjs *objs ) {
  XFORM_TEST( expand );
}

//! Test extraction of R, G, B, and A components from a pixel
void test_get_rgba( TestObjs *objs ) {
  // Basic test 
  uint32_t p1 = 0x12345678;
  ASSERT(get_r(p1) == 0x12);
  ASSERT(get_g(p1) == 0x34);
  ASSERT(get_b(p1) == 0x56);
  ASSERT(get_a(p1) == 0x78);

  // Test with higher bits 
  uint32_t p2 = 0x80F0ABFF; 
  ASSERT(get_r(p2) == 0x80);
  ASSERT(get_g(p2) == 0xF0);
  ASSERT(get_b(p2) == 0xAB);
  ASSERT(get_a(p2) == 0xFF);

  // Test with all 0s 
  uint32_t p3 = 0x00; 
  ASSERT(get_r(p3) == 0x00); 
  ASSERT(get_b(p3) == 0x00); 
  ASSERT(get_g(p3) == 0x00); 
  ASSERT(get_a(p3) == 0x00); 
}

//! Test concatenation of R, G, B, and A components into a pixel
void test_make_pixel( TestObjs *objs ) {
  // Basic test 
  ASSERT(make_pixel(0x12, 0x34, 0x56, 0x78) == 0x12345678);

  // Test with all 0s 
  ASSERT(make_pixel(0, 0, 0, 0) == 0x00000000);

  // Test with max bits 
  ASSERT(make_pixel(255, 255, 255, 255) == 0xFFFFFFFF);
} 

//! Test index validation from based on image dimensions 
void test_valid_index( TestObjs *objs ) {
  // Test corners 
  ASSERT(valid_index(&objs->smol, 0, 0) == 1); 
  ASSERT(valid_index(&objs->smol, objs->smol.height - 1, objs->smol.width - 1) == 1);

  // Test pixels just out of bounds 
  ASSERT(valid_index(&objs->smol, objs->smol.height, 0) == 0);
  ASSERT(valid_index(&objs->smol, 0, objs->smol.width) == 0);

  // Test Negative values
  ASSERT(valid_index(&objs->smol, -1, 0) == 0);
  ASSERT(valid_index(&objs->smol, 0, -1) == 0);
}

//! Test computation of row-major order index 
void test_compute_index( TestObjs *objs ) {
  // Basic tests
  ASSERT(compute_index(&objs->smol, 0, 0) == 0); 
  ASSERT(compute_index(&objs->smol, 0, 1) == 1); 
  ASSERT(compute_index(&objs->smol, 1, 1) == objs->smol.width + 1); 
  ASSERT(compute_index(&objs->smol, objs->smol.height - 1, objs->smol.width - 1) == (objs->smol.height - 1) * objs->smol.width + objs->smol.width - 1); 
}

//! Test neighbour averaging for blur effect 
void test_blur_pixel( TestObjs *objs ) {
  // Dist 0 test 
  uint32_t og1 = objs->smol.data[compute_index(&objs->smol, 1, 1)];
  uint32_t blur1 = blur_pixel(&objs->smol, 1, 1, 0);
  ASSERT(blur1 == og1);

  // Dist 3 tests 
  // Basic test 
  uint32_t blur2 = blur_pixel(&objs->smol, 5, 5, 3); 
  ASSERT(blur2 == objs->smol_blur_3.data[compute_index(&objs->smol, 5, 5)]); 

  // Edge tests 
  uint32_t blur3 = blur_pixel(&objs->smol, objs->smol.height - 1, 6, 3); 
  ASSERT(blur3 == objs->smol_blur_3.data[compute_index(&objs->smol, objs->smol.height - 1, 6)]); 

  // Corner test
  uint32_t blur4 = blur_pixel(&objs->smol, 0, objs->smol.width - 1, 3); 
  ASSERT(blur4 == objs->smol_blur_3.data[compute_index(&objs->smol, 0, objs->smol.width - 1)]); 
}

//! Test rotation of colour channels 
void test_rot_colors( TestObjs *objs ) {
  // Basic test 
  uint32_t p1 = 0x12345678; 
  struct Image img1 = { 1, 1, &p1 };
  ASSERT(rot_colors(&img1, 0) == 0x56123478);

  // Test with all 0s 
  uint32_t p2 = 0x00; 
  struct Image img2 = { 1, 1, &p2 };
  ASSERT(rot_colors(&img2, 0) == 0x00);

  // Test with higher bits 
  uint32_t p3 = 0x80F0ABFF; 
  struct Image img3 = { 1, 1, &p3 }; 
  ASSERT(rot_colors(&img3, 0) == 0xAB80F0FF); 
} 

//! Test expansion where output pixels map exactly to an existing input pixel
void test_expand_even_even( TestObjs *objs ) {
  // Basic test 
  uint32_t p1 = expand_even_even(&objs->smol, 2, 2);
  ASSERT(p1 == objs->smol_expand.data[compute_index(&objs->smol_expand, 2, 2)]);

  // Corner pixel test 
  uint32_t p2 = expand_even_even(&objs->smol, 0, 0); 
  ASSERT(p2 == objs->smol_expand.data[0]); 

  // Edge pixel test 
  uint32_t p3 = expand_even_even(&objs->smol, 0, 4);
  ASSERT(p3 == objs->smol_expand.data[compute_index(&objs->smol_expand, 0, 4)]); 
} 

//! Test expansion case where output pixels are a horizontal average of two input pixels
void test_expand_even_odd( TestObjs *objs ) {
  // Basic test 
  uint32_t p1 = expand_even_odd(&objs->smol, 2, 5); 
  ASSERT(p1 == objs->smol_expand.data[compute_index(&objs->smol_expand, 2, 5)]); 

  // Edge pixel test
  uint32_t p2 = expand_even_odd(&objs->smol, 0, 5); 
  ASSERT(p2 == objs->smol_expand.data[5]); 
} 

//! Test expansion case where output pixels are a vertical average of two input pixels
void test_expand_odd_even( TestObjs *objs ) {
  // Basic test 
  uint32_t p1 = expand_odd_even(&objs->smol, 17, 4); 
  ASSERT(p1 == objs->smol_expand.data[compute_index(&objs->smol_expand, 17, 4)]); 

  // Edge pixel test 
  uint32_t p2 = expand_odd_even(&objs->smol, 19, 14); 
  ASSERT(p2 == objs->smol_expand.data[compute_index(&objs->smol_expand, 19, 14)]); 
} 

//! Test expansion case where output pixels are an average of four input pixels
void test_expand_odd_odd( TestObjs *objs ) {
  // Basic tests 
  uint32_t p1 = expand_odd_odd(&objs->smol, 9, 9); 
  ASSERT(p1 == objs->smol_expand.data[compute_index(&objs->smol_expand, 9, 9)]); 

  uint32_t p2 = expand_odd_odd(&objs->smol, 1, 3); 
  ASSERT(p2 == objs->smol_expand.data[compute_index(&objs->smol_expand, 1, 3)]); 
} 

//! Test initialization of PixelAverager struct
void test_pa_init( TestObjs *objs ) {
  // Basic test 
  struct PixelAverager pa;
  pa_init(&pa);
  ASSERT(pa.r == 0);
  ASSERT(pa.g == 0);
  ASSERT(pa.b == 0);
  ASSERT(pa.a == 0);
  ASSERT(pa.count == 0);

  // Test re-initialize with existing values 
  pa.r = 10; 
  pa.g = 10; 
  pa.b = 10; 
  pa.a = 10; 
  pa.count++; 
  pa_init(&pa); 
  ASSERT(pa.r == 0);
  ASSERT(pa.g == 0);
  ASSERT(pa.b == 0);
  ASSERT(pa.a == 0);
  ASSERT(pa.count == 0);
} 

//! Test accumulation of pixel components into PixelAverager
void test_pa_update( TestObjs *objs ) {
  // Basic test
  struct PixelAverager pa;
  pa_init(&pa);
  pa_update(&pa, make_pixel(10, 20, 30, 40)); 
  ASSERT(pa.r == 10); 
  ASSERT(pa.g == 20); 
  ASSERT(pa.b == 30); 
  ASSERT(pa.a == 40); 
  ASSERT(pa.count == 1); 

  // Test more accumulation 
  pa_update(&pa, make_pixel(1, 2, 3, 4)); 
  ASSERT(pa.r == 11); 
  ASSERT(pa.g == 22); 
  ASSERT(pa.b == 33); 
  ASSERT(pa.a == 44); 
  ASSERT(pa.count == 2); 
} 

//! Test retrieval and accumulation of pixel data from an image into PixelAverager
void test_pa_update_from_img( TestObjs *objs ) {
  // Basic test
  struct PixelAverager pa; 
  pa_init(&pa); 
  pa_update_from_img(&objs->smol, 0, 0, &pa); 
  ASSERT(pa.r == get_r(objs->smol.data[0])); 
  ASSERT(pa.g == get_g(objs->smol.data[0])); 
  ASSERT(pa.b == get_b(objs->smol.data[0])); 
  ASSERT(pa.count == 1); 

  // Test more accumulation 
  pa_update_from_img(&objs->smol, 4, 9, &pa); 
  ASSERT(pa.r == get_r(objs->smol.data[0]) + get_r(objs->smol.data[compute_index(&objs->smol, 4, 9)])); 
  ASSERT(pa.g == get_g(objs->smol.data[0]) + get_g(objs->smol.data[compute_index(&objs->smol, 4, 9)])); 
  ASSERT(pa.b == get_b(objs->smol.data[0]) + get_b(objs->smol.data[compute_index(&objs->smol, 4, 9)])); 
  ASSERT(pa.count == 2); 
} 

//! Test final calculation of average colour values and alpha override 
void test_pa_avg_pixel( TestObjs *objs ) {
  // Test when count = 0 
  struct PixelAverager pa; 
  pa_init(&pa); 
  ASSERT(pa_avg_pixel(&pa, 0, 0) == make_pixel(0, 0, 0, 0)); 
  
  // Basic test
  pa_update(&pa, make_pixel(10, 20, 30, 40)); 
  ASSERT(pa_avg_pixel(&pa, 0, 0) == make_pixel(10, 20, 30, 40)); 

  // Test more accumulation 
  pa_update(&pa, make_pixel(2, 1, 0, 10)); 
  ASSERT(pa_avg_pixel(&pa, 0, 0) == make_pixel(6, 10, 15, 25)); 

  // Test alpha override 
  ASSERT(pa_avg_pixel(&pa, 1, 9) == make_pixel(6, 10, 15, 9)); 
} 
