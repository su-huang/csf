/*
 * x86-64 assembly language implementations of functions
 */

	.section .text

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET   0
#define IMAGE_HEIGHT_OFFSET  4
#define IMAGE_DATA_OFFSET    8

/*
 * Gets the R value of the pixel 
 * C function: uint32_t get_r( uint32_t pixel )
 *
 * Parameters: 
 *	%edi - contains 32-bit pixel value 
 *
 * Returns: 
 *	%eax - 32-bit value representing R
 */
	.globl get_r
get_r: 
	movl %edi, %eax  	/* move pixel from %edi to %eax */
	shrl $24, %eax		/* right shift pixel in %eax by 24 bits */
	andl $0xff, %eax 	/* isolate R value */
	ret 				/* return R in %eax */ 

/*
 * Gets the G value of the pixel 
* C function: uint32_t get_g( uint32_t pixel )
 *
 * Parameters: 
 *	%edi - contains 32-bit pixel value 
 *
 * Returns: 
 *	%eax - 32-bit value representing G
 */
	.globl get_g
get_g: 
	movl %edi, %eax  	/* move pixel from %edi to %eax */
	shrl $16, %eax		/* right shift pixel in %eax by 16 bits */
	andl $0xff, %eax 	/* isolate G value */
	ret 				/* return G in %eax */ 

/*
 * Gets the B value of the pixel 
 * C function: uint32_t get_b( uint32_t pixel )
 *
 * Parameters: 
 *	%edi - contains 32-bit pixel value 
 *
 * Returns: 
 *	%eax - 32-bit value representing B
 */
	.globl get_b
get_b: 
	movl %edi, %eax  	/* move pixel from %edi to %eax */
	shrl $8, %eax		/* right shift pixel in %eax by 8 bits */
	andl $0xff, %eax 	/* isolate B value */
	ret 				/* return B in %eax */ 

/*
 * Gets the alpha value of the pixel 
 * C function: uint32_t get_a( uint32_t pixel )
 *
 * Parameters: 
 *	%edi - contains 32-bit pixel value 
 *
 * Returns: 
 *	%eax - 32-bit value representing alpha
 */
	.globl get_a
get_a: 
	movl %edi, %eax  	/* move pixel from %edi to %eax */
	andl $0xff, %eax 	/* isolate alpha value */
	ret 				/* return alpha in %eax */ 

/*
 * Concatenates the RGB and alpha values into a pixel representation 
 * C function: uint32_t make_pixel( uint32_t r, uint32_t g, uint32_t b, uint32_t a )
 *
 * Parameters: 
 *	%edi - contains 32-bit R value 
 * 	%esi - contains 32-bit G value 
 *	%edx - contains 32-bit B value
 *  %ecx - contains 32-bit alpha value 
 * 
 * Returns: 
 *	%eax - 32-bit pixel representing these values 
 */ 
	.globl make_pixel
make_pixel: 
	shll $24, %edi 		/* right shift R in %edi by 24 bits */
	shll $16, %esi 		/* right shift G in %esi by 16 bits */
	shll $8, %edx 		/* right shift B in %edx by 8 bits */
	orl %esi, %edi 		/* OR R with G to combine and store in %edi */
	orl %edx, %edi 		/* OR B with R and G to combine and store in %edi */ 
	orl %ecx, %edi 		/* OR alpha with R and G and B to combine and store in %edi */
	movl %edi, %eax 	/* move pixel from %edi to %eax */
	ret 				/* return pixel in %eax */ 	

/*
 * Computes the row-major index of the pixel expressed in 2D indexing 
 * C function: int32_t compute_index( struct Image *img, int32_t row, int32_t col )
 * 
 * Parameters: 
 *	%rdi - contains 64-bit Image pointer 
 * 	%esi - contains 32-bit row index 
 *	%edx - contains 32-bit col index 
 * 
 * Returns: 
 *	%eax - 32-bit row-major index of the pixel
 */ 
	.globl compute_index
compute_index: 
	movl IMAGE_WIDTH_OFFSET(%rdi), %eax		/* dereference pointer to width (32-bit) in %rdi and move to %eax */ 
	imull %esi, %eax 						/* multiply row with width and store in %eax */ 
	addl %edx, %eax 						/* add column with row * width and store in %eax */ 
	ret 									/* return index in %eax */ 

/*
 * Definitions of image transformation functions
 */

/*
 *  Transform the entire image by shrinking it down both 
 *  horizontally and vertically (by potentially different
 *  factors). This is equivalent to sampling the orignal image
 *  for every pixel that is in certain rows and columns as 
 *  specified in the function inputs.
 * 
 *  Take the image below where each letter corresponds to a pixel
 * 
 *                  XAAAYBBB
 *                  AAAABBBB
 *                  ZCCCWDDD
 *                  CCCCDDDD
 * 
 *  If the user specified to shrink it horazontally by a factor 
 *  of 4 and shrink it vertically by a factor of 2, you would 
 *  sample pixel that had a row index such that 
 * 
 *              row index % 2 = 0 
 * 
 *  and a column index such that
 * 
 *              column index % 4 = 0
 * 
 *  in the above example, this would mean pixels that are in 
 *  rows 0 and 2 with columns 0 and 4. 
 *  The resultant image is:
 * 
 *                  XY
 *                  ZW
 *  
 *  @param input_img pointer to the input Image
 *  @param output_img pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 *  @param xfac factor to downsize the image horizontally; guaranteed to be positive
 *  @param yfac factor to downsize the image vertically; guaranteed to be positive
 */

 /* 
  * Modifies output image to shrink image horizontally and vertically 
  * C function: void imgproc_squash( struct Image *input_img, struct Image *output_img, int32_t xfac, int32_t yfac )
  * 
  * Parameters: 
  * 	%rdi - contains 64-bit input Image pointer 
  * 	%rsi - contains 64-bit output Image pointer 
  *		%edx - contains 32-bit xfac 
  *		%ecx - contains 32-bit yfac 
  *
  * Register use: 
  * 	%r12 - input Image pointer 
  * 	%r13 - output Image pointer 
  *		%r14d - xfac 
  * 	%r15d - yfac 
  *		%ebx - current output row index 	
  * 
  * Memory use: 
  *		-8(%rbp) - %r12 (input Image pointer)
  * 	-16(%rbp) - %r13 (output Image pointer)
  *		-24(%rbp) - %r14 (xfac) 
  * 	-32(%rbp) - %r15 (yfac) 
  *		-40(%rbp) - %rbx (current output row index) 
  * 	-48(%rbp) - from %rsp stack alignment 
  * 	
  */ 
	.globl imgproc_squash
imgproc_squash:
	/* TODO: implement */
	/* prologue */ 
	pushq %rbp 			/* save previous value in %rbp */ 
	movq %rsp, %rbp 	/* save top of stack from %rsp to %rbp */ 

	
	/* stores local variables on the stack */
	pushq %r12		/* will store input Image pointer */
	pushq %r13 		/* will store output Image pointer */ 
	pushq %r14		/* will store xfac */ 
	pushq %r15 		/* will store yfac */ 
	pushq %rbx 		/* will store current index */ 
	subq $8, %rsp 	/* align %rsp to be a multiple of 16 */

	movq %rdi, %r12 	/* move input Image pointer from %rdi to %r12 */ 
	movq %rsi, %r13 	/* move output Image pointer from %rsi to %r13 */ 
	movl %edx, %r14d	/* move xfac from %edx to %r14d */ 
	movl %ecx, %r15d 	/* move yfac from %ecx to %r15d */ 

	/* initializes output image dimensions: img_init(output_img, input_img->width / xfac, input_img->height / yfac) */
	movl IMAGE_WIDTH_OFFSET(%r12), %eax		/* dereference pointer to width (32-bit) in %r12 and move to %eax */ 
	cltd 									/* make 64-bit version spread across %edx:%eax for 64-bit division */
	idivl %r14d 							/* divide 64-bit width by 32-bit xfac and store quotient in %eax, remainder in %edx*/
	movl %eax, %esi 						/* move quotient (output image width) from %eax to %esi (2nd arg for img_init) */

	movl IMAGE_HEIGHT_OFFSET(%r12), %eax 	/* dereference pointer to height (32-bit) in %r12 and move to %eax */ 
	cltd 									/* make 64-bit version spread across %edx:%eax for 64-bit division */
	idivl %r15d 							/* divide 64-bit height by 32-bit yfac and store quotient in %eax, remainder in %edx*/
	movl %eax, %edx  						/* move quotient (output image height) from %eax to %edx (3rd arg for img_init) */

	movq %r13, %rdi 						/* move ouput image pointer from r13 to %rdi (1st arg for img_init) */ 
	call img_init							/* call img_init and return in %eax */ 
	
	cmpl $0, %eax		/* compare 0 with return from img_init */ 	
	jne .Lsquash_done	/* jump to end if return value != 0 */

	movl $0, %ebx 		/* move 0 to %ebx (row index) */ 

.Lsquash_loop_row: 
	cmpl IMAGE_HEIGHT_OFFSET(%r13), %ebx	/* compare output image height with row index */
	jge .Lsquash_done 						/* jump to end if row index >= output image height */

	movl $0, %r10d 		/* move 0 to %r10d (col index) */ 

.Lsquash_loop_col: 
	cmpl IMAGE_WIDTH_OFFSET(%r13), %r10d 	/* compare output image width with col index */ 
	jge .Lsquash_next_row					/* jump to end if col index >= output image width */ 

	/* find input index: compute_index(input_img, row * yfac, col * xfac) */ 
	movq %r12, %rdi 		/* move input image pointer from %r12 to %rdi (1st arg for compute_index) */ 
	movl %ebx, %esi 		/* move output row index from %ebx to %esi */ 
	imull %r15d, %esi 		/* multiply yfac with output image row and store result in %esi (2nd arg for compute_index) */ 
	movl %r10d, %edx 		/* move output col index from %r10d to %edx */ 
	imull %r14d, %edx 		/* multiply xfac with output image col and store result in %edx (3rd arg for compute_index) */ 
	call compute_index 		/* call compute_index and return input image index in %eax */ 
	movl %eax, %r11d 		/* move input image index from %eax to %r11d */ 

	/* find output index: compute_index(output_img, row, col) */ 
	movq %r13, %rdi 		/* move output image pointer from %r13 to %rdi (1st arg for compute_index) */ 
	movl %ebx, %esi 		/* move output row index from %ebx to %esi (2nd arg for compute_index) */  
	movl %r10d, %edx 		/* move output col index from %r10d to %edx (3rd arg for compute_index) */ 
	call compute_index		/* call compute_index and return output image index in %eax */ 

	/* assign input image pixel to output image pixel */ 
	movq IMAGE_DATA_OFFSET(%r12), %r8		/* dereference pointer to input image pixel and move from %r12 to %r8 */
	movl (%r8, %r11, 4), %r9d				/* move pixel at input image index from %r8 + (%r11 * 4) to %r9d */ 
	movq IMAGE_DATA_OFFSET(%r13), %r8		/* dereference pointer to output image pixel and move from %r13 to %r8 */
	movl %r9d, (%r8, %rax, 4)				/* move pixel of input image index from %r9d to pixel at output image index at %r8 + (%rax * 4) */ 

	incl %r10d 					/* increment column index */
	jmp .Lsquash_loop_col		/* repeat column loop */ 

.Lsquash_next_row: 
	incl %ebx 					/* increement row index */ 
	jmp .Lsquash_loop_row 		/* repeat row loop */ 

.Lsquash_done: 
	/* analogue */ 
	addq $8, %rsp	/* remove alignment */ 
	popq %rbx 		/* restore callee-saved registers*/
	popq %r15 
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

	ret

/*
 *  Transform the color component values in each input pixel
 *  by applying a rotation on the values of the color components
 *  I.e. the old pixel's red component value will be used for
 *  the new pixel's green component value, the old pixel's green
 *  component value will be used new pixel's blue component value
 *  and the old pixel's blue component value will be used new 
 *  pixel's red component value. The alpha value should not change.
 *  For instance, if a pixel had the hex value 0xAABBCCDD, the 
 *  transformed pixel would become 0xCCAABBDD
 * 
 *  @param input_img pointer to the input Image
 *  @param output_img pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 */
	.globl imgproc_color_rot
imgproc_color_rot:
	/* TODO: implement */
	ret


/*
 *  Transform the input image using a blur effect.
 * 
 *  Each pixel of the output image should have its color components
 *  determined by taking the average of the color components of pixels
 *  within blur_dist number of pixels horizontally and vertically from
 *  the pixel's location in the original image. For example, if
 *  blur_dist is 0, then only the original pixel is considered, and the
 *  the output image should be identical to the input image. If blur_dist
 *  is 1, then the original pixel and the 8 pixels immediately surrounding
 *  it would be considered, etc.  Pixels positions not within the bounds of
 *  the image should be ignored: i.e., their color components aren't
 *  considered in the computation of the result pixels.
 * 
 *  The alpha value each output pixel should be identical to the
 *  corresponding input pixel.
 * 
 *  Averages should be computed using purely integer arithmetic with
 *  no rounding.
 * 
 *  @param input_img pointer to the input Image
 *  @param output_img pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 *  @param blur_dist all pixels whose x/y coordinates are within
 *                   this many pixels of the x/y coordinates of the
 *                   original pixel should be included in the color
 *                   component averages used to determine the color
 *                   components of the output pixel
 */
	.globl imgproc_blur
imgproc_blur:
	/* TODO: implement */
	ret

/*
 *  The `expand` transformation doubles the width and height of the image.
 *  
 *  Let's say that there are n rows and m columns of pixels in the
 *  input image, so there are 2n rows and 2m columns in the output
 *  image.  The pixel color and alpha value of the output pixel at row i and column
 *  j should be computed as follows.
 *  
 *  If both i and j are even, then the color and alpha value of the output
 *  pixel are exactly the same as the input pixel at row i/2 and column j/2.
 *  
 *  If i is even but j is odd, then the color components and alpha value
 *  of the output pixel are computed as the average of those in the input pixels
 *  in row i/2 at columns floor(j/2) and floor(j/2) + 1.
 *  
 *  If i is odd and j is even, then the color components and alpha value
 *  of the output pixel are computed as the average of those in the input pixels
 *  in column j/2 at rows floor(i/2) and  floor(i/2) + 1.
 *  
 *  If both i and j are odd then the color components and alpha value
 *  of the output pixel are computed as the average of the input pixels
 *  
 *  1. At row floor(i/2) and column floor(j/2)
 *  2. At row floor(i/2) and column floor(j/2) + 1
 *  3. At row floor(i/2) + 1 and column floor(j/2)
 *  4. At row floor(i/2) + 1 and column floor(j/2) + 1
 *  
 *  Note that in the cases where either i or j is odd, it is not
 *  necessarily the case that either row floor(i/2) + 1 or
 *  column floor(j/2) + 1 are in bounds in the input image.
 *  Only input pixels that are properly in bounds should be incorporated into
 *  the averages used to determine the color components and alpha value
 *  of the output pixel.
 *  
 *  Averages should be computed using purely integer arithmetic with
 *  no rounding.
 * 
 *  @param input_img pointer to the input Image
 *  @param output_img pointer to the output Image (in which the
 *                    transformed pixels should be stored)
 */
	.globl imgproc_expand
imgproc_expand:
	/* TODO: implement */
	ret

/*
vim:ft=gas:
*/
