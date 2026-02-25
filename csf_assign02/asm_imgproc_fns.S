/*
 * x86-64 assembly language implementations of functions
 * CSF Assignment 2
 * Su Huang 
 * shuan148@jh.edu
 */

	.section .text

/* Offsets of struct Image fields */
#define IMAGE_WIDTH_OFFSET 		0
#define IMAGE_HEIGHT_OFFSET 	4
#define IMAGE_DATA_OFFSET		8

/* Offsets of struct PixelAverager fields */ 
#define PA_R_OFFSET			0
#define PA_G_OFFSET			4
#define PA_B_OFFSET 		8
#define PA_A_OFFSET 		12
#define PA_COUNT_OFFSET 	16 

/*
 * Gets the R value of the pixel 
 * C function: uint32_t get_r( uint32_t pixel )
 *
 * Parameters: 
 *	%edi - 32-bit pixel value 
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
 *	%edi - 32-bit pixel value 
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
 *	%edi - 32-bit pixel value 
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
 *	%edi - 32-bit pixel value 
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
 *	%edi - 32-bit R value 
 * 	%esi - 32-bit G value 
 *	%edx - 32-bit B value
 *  %ecx - 32-bit alpha value 
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
 *	%rdi - 64-bit Image pointer 
 * 	%esi - 32-bit row index 
 *	%edx - 32-bit col index 
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
 * Rotates the RGB colours of a pixel to BRG 
 * C function: uint32_t rot_colors( struct Image *img, int32_t index ) 
 * 
 * Parameters: 
 * 	%rdi - 64-bit Image pointer 
 * 	%esi - 32-bit index 
 * 
 * Returns: 
 *	%eax - 32-bit rotated pixel 
 * 
 * Register use: 
 * 	%r12d - pixel  
 * 	%r13d - output R 
 *	%r14d - output G 
 * 	%r15d - output B
 * 	%ebx - alpha 	
 * 
 * Memory use: 
 *	-8(%rbp) - %r12d (pixel)
 * 	-16(%rbp) - %r13d (output R)
 *	-24(%rbp) - %r14d (output B) 
 * 	-32(%rbp) - %r15d (output B) 
 *  -40(%rbp) - %ebx (alpha)
 * 	-48(%rbp) - %rsp stack alignment 
 */ 
	.globl rot_colors
rot_colors:
	/* prologue */ 
	pushq %rbp 			/* save previous value in %rbp */ 
	movq %rsp, %rbp 	/* save top of stack from %rsp to %rbp */ 
	pushq %r12			/* will store pixel */
	pushq %r13 			/* will store output R */ 
	pushq %r14			/* will store output G */ 
	pushq %r15 			/* will store output B */ 
	pushq %rbx 			/* will store alpha */ 
	subq $8, %rsp 		/* align %rsp to be a multiple of 16 */

	movq IMAGE_DATA_OFFSET(%rdi), %r8		/* dereference pointer to input image pixel and move from %rdi to %r8 */
	movl (%r8, %rsi, 4), %r12d 				/* move pixel at input image index from %r8 + (%esi * 4) to %r12d */ 

	/* extract RGB and alpha values: get_b(pixel), get_g(pixel), get_r(pixel), get_a(pixel), */
	movl %r12d, %edi 		/* move input image pixel from %r12d to %edi (1st arg for get_b) */ 
	call get_b 				/* call get_b and return in %eax */ 
	movl %eax, %r13d 		/* move output R from %eax to %r13d */ 

	movl %r12d, %edi 		/* move input image pixel from %r12d to %edi (1st arg for get_r) */ 
	call get_r 				/* call get_r and return in %eax */ 
	movl %eax, %r14d 		/* move output G from %eax to %r14d */ 

	movl %r12d, %edi 		/* move input image pixel from %r12d to %edi (1st arg for get_g) */ 
	call get_g				/* call get_g and return in %eax */ 
	movl %eax, %r15d 		/* move output B from %eax to %r15d */ 

	movl %r12d, %edi 		/* move input image pixel from %r12d to %edi (1st arg for get_a) */ 
	call get_a 				/* call get_a and return in %eax */ 
	movl %eax, %ebx 		/* move alpha from %eax to %ebx */ 

	/* make pixel from new values: make_pixel(new_r, new_g, new_b, a) */
	movl %r13d, %edi 		/* move output R from %r13d to %edi (1st arg for make_pixel) */ 
	movl %r14d, %esi 		/* move output G from %r14d to %esi (2nd arg for make_pixel) */ 
	movl %r15d, %edx 		/* move output B from %r15d to %edx (3rd arg for make_pixel) */ 
	movl %ebx, %ecx 		/* move alpha from %ebx to %ecx (4th arg for make_pixel) */ 
	call make_pixel			/* call make_pixel and return in %eax */ 

	/* epilogue */ 
	addq $8, %rsp	/* remove alignment */ 
	popq %rbx 		/* restore callee-saved registers */
	popq %r15 
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

    ret		/* return pixel in %eax */ 

/* 
 * Determines if the row and col indices provided are valid in the image
 * C function: int32_t valid_index( struct Image *img, int32_t row, int32_t col )
 * 
 * Parameters: 
 * 	%rdi - 64-bit Image pointer 
 * 	%esi - 32-bit row index 
 *	%edx - 32-bit col index 
 * 
 * Returns: 
 * 	%eax - 32-bit 0 (invalid) or 1 (valid)
 */ 
	.globl valid_index
valid_index:
	cmpl $0, %esi				/* compare row index to 0 */
	jl .Lvalid_index_false 		/* jump to invalid index if row index < 0 */

	cmpl $0, %edx 				/* compare col index to 0 */
	jl .Lvalid_index_false 		/* jump to invalid index if col index < 0 */

	movl IMAGE_HEIGHT_OFFSET(%rdi), %eax		/* dereference pointer to height (32-bit) in %rdi and move to %eax */
	cmpl %eax, %esi 							/* compare row index to image height */
	jge .Lvalid_index_false						/* jump to invalid index if row index >= image height */ 

	movl IMAGE_WIDTH_OFFSET(%rdi), %eax 		/* dereference pointer to width (32-bit) in %rdi and move to %eax */
	cmpl %eax, %edx 							/* compare col index to image width */
	jge .Lvalid_index_false						/* jump to invalid index if col index >= image width */ 

	movl $1, %eax 		/* move 1 to %eax */
    ret					/* return 1 in %eax */

.Lvalid_index_false: 
	movl $0, %eax 		/* move 0 to %eax */
	ret 				/* return 0 in %eax */ 

/* Applies blur to a single pixel by averaging neighbouring pixels' RGB values using PixelAverager struct 
 * C function: uint32_t blur_pixel( struct Image *img, int32_t row, int32_t col, int32_t blur_dist )
 *
 * Parameters: 
 *	%rdi - 64-bit Image pointer  
 * 	%esi - 32-bit row index 
 *	%edx - 32-bit col index 
 *  %ecx - 32-bit blur dist 
 * 
 * Returns: 
 *	%eax - 32-bit blurred pixel 
 *
 * Register use: 
 * 	%r12 - Image pointer 
 * 	%r13d - current row index
 *	%r14d - current col index 
 * 	%r15d - blur dist 
 *	%rbx - PixelAverager pointer  
 * 
 * Memory use: 
 *	-8(%rbp) - %r12 (Image pointer)
 * 	-16(%rbp) - %r13d (row index)
 * 	-24(%rbp) - %r14d (col index)
 *	-32(%rbp) - %r15d (blur dist)
 * 	-40(%rbp) - %rbx (PixelAverager pointer)
 *	-60(%rbp) - stack: PixelAverager struct 
 *	-64(%rbp) - stack: max row index 
 *	-68(%rbp) - stack: max col index 
 *	-72(%rbp) - stack: min col index 
 *	-76(%rbp) - stack: initial row index 
 *	-80(%rbp) - stack: initial col index 
 */  
	.globl blur_pixel
blur_pixel:
	/* prologue */ 
	pushq %rbp 				/* save previous value in %rbp */ 
	movq %rsp, %rbp 		/* save top of stack from %rsp to %rbp */ 
	pushq %r12				/* will store image pointer */
	pushq %r13				/* will store row index */ 
	pushq %r14				/* will store col index */
	pushq %r15 				/* will store blur dist */
	pushq %rbx				/* will store PixelAverager pointer */
	subq $40, %rsp 			/* align %rsp to -80 */
	movq %rdi, %r12			/* move image pointer from %rdi to %r12 */ 
	movl %esi, %r13d 		/* move row index from %esi to %r13d */ 
	movl %esi, -76(%rbp)	/* move row index from %esi to -76(%rbp) */
	movl %edx, %r14d		/* move col index from %edx to %r14d */
	movl %edx, -80(%rbp)	/* move col index from %edx to -80(%rbp) */
	movl %ecx, %r15d		/* move blur dist from %ecx to %r15 */
	leaq -60(%rbp), %rbx	/* %rbx points to PixelAverager struct */

	/* initialize PixelAverager struct: pa_init(pa) */
	movq %rbx, %rdi 		/* move PixelAverager pointer from %rbx to %rdi (1st arg for pa_init) */ 
	call pa_init			/* call pa_init */ 

	movl %r13d, -64(%rbp)	/* move row index from %r13d to -64(%rbp) */
	addl %r15d, -64(%rbp)	/* add row index with blur dist to get max row index and store in -64(%rbp) */ 
	movl %r14d, -68(%rbp)	/* move col index from %r14d to -68(%rbp) */
	addl %r15d, -68(%rbp)	/* add col index with blur dist to get max col index and store in -68(%rbp) */

	subl %r15d, %r13d 		/* subtract blur dist from row index to get starting index and store in %r13d */ 
	subl %r15d, %r14d		/* subtract blur dist from col index to get starting index and store in %r14d */
	movl %r14d, -72(%rbp)	/* move starting col index from %r14d to -72(%rbp) to store min col index */

.Lblur_pixel_loop_row: 
	cmpl -64(%rbp), %r13d		/* compare row index with max row index */
	jg .Lblur_pixel_average 	/* jump to averaging if row index > max row index */

	movl -72(%rbp), %r14d	/* move min col index from -72(%rbp) to %r14d */

.Lblur_pixel_loop_col: 
	cmpl -68(%rbp), %r14d 		/* compare col index with max col index */ 
	jg .Lblur_pixel_next_row	/* jump to next row if col index > max col index */ 

	/* aggregate all pixels within distance range: pa_update_from_img(img, row_index, col_index, pa) */
	movq %r12, %rdi 			/* move image pointer from %r12 to %rdi (1st arg for pa_update_from_img) */
	movl %r13d, %esi			/* move row index from %r13d to %esi (2nd arg for pa_update_from_img) */
	movl %r14d, %edx			/* move col index from %r14d to %edx (3rd arg for pa_update_from_img) */
	movq %rbx, %rcx				/* move PixelAverager pointer from %rbx to %rcx (4th arg for pa_update_from_img) */
	call pa_update_from_img		/* call pa_update_from_img */ 

	incl %r14d						/* increment col index */
	jmp .Lblur_pixel_loop_col		/* repeat col loop */

.Lblur_pixel_next_row: 
	incl %r13d 						/* increment row index */ 
	jmp .Lblur_pixel_loop_row 		/* repeat row loop */ 

.Lblur_pixel_average: 
	/* get index of pixel to blur: compute_index(img, row, col) */
	movq %r12, %rdi			/* move image pointer from %r12 to %rdi (1st arg for compute_index) */
	movl -76(%rbp), %esi	/* move initial row index from -76(%rbp) to %esi (2nd arg for compute_index) */
	movl -80(%rbp), %edx	/* move initial col index from -80(%rbp) to %edx (3rd arg for compute_index) */
	call compute_index 		/* call compute_index and return pixel index in %eax */

	/* get alpha value fo initial pixel: get_a(pixel) */
	movq IMAGE_DATA_OFFSET(%r12), %r8 		/* move image data pointer from %r12 offset to %r8 */
	movl (%r8, %rax, 4), %edi 				/* move pixel at image index from %r8 + (%rax * 4) to %edi (1st arg for get_a) */ 
	call get_a 								/* call get_a and return alpha in %eax */

	/* calculate average pixel and set default alpha: pa_avg_pixel(pa, 1, a) */
	movq %rbx, %rdi			/* move PixelAverager pointer from %rbx to %rdi (1st arg for pa_avg_pixel) */
	movl $1, %esi			/* move 1 to %esi (2nd arg for pa_avg_pixel) */
	movl %eax, %edx			/* move alpha value from %eax to %edx (3rd arg for pa_avg_pixel) */
	call pa_avg_pixel 		/* call pa_avg_pixel and return averaged pixel in %eax*/

.Lblur_pixel_done: 
	/* epilogue */
	addq $40, %rsp 		/* remove alignment */ 
	popq %rbx 			/* restore callee-saved registers */
	popq %r15 
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

	ret 	/* return blurred pixel in %eax */

/* 	
 *	Assigns output pixel the input pixel at row i/2 and col j/2 for even row and col indices 
 * 
 *	C function: uint32_t expand_even_even (struct Image *img, uint32_t row, uint32_t col )
 *
 * Parameters: 
 *	%rdi - 64-bit Image pointer  
 * 	%esi - 32-bit row index 
 *	%edx - 32-bit col index 
 * 
 * Returns: 
 *	%eax - 32-bit average pixel 
 *
 * Register use: 
 * 	%r12 - Image pointer 
 * 	%r13d - row index
 *	%r14d - col index 
 * 	%r15 - PixelAverager pointer  
 * 
 * Memory use: 
 *	-8(%rbp) - %r12 (Image pointer)
 * 	-16(%rbp) - %r13d (row index)
 * 	-24(%rbp) - %r14d (col index)
 *	-32(%rbp) - %r15 (PixelAverager pointer)
 *	-52(%rbp) - stack: PixelAverager struct 
 * 	-64(%rbp) - %rsp stack alignment 
 */  
	.globl expand_even_even
expand_even_even:
	/* prologue */
	pushq %rbp 				/* save previous value in %rbp */ 
	movq %rsp, %rbp 		/* save top of stack from %rsp to %rbp */ 
	pushq %r12				/* will store image pointer */
	pushq %r13				/* will store row index */ 
	pushq %r14 				/* will store col index */
	pushq %r15 				/* will store PixelAverager pointer */
	subq $32, %rsp 			/* align %rsp to -64 */
	movq %rdi, %r12			/* move image pointer from %rdi to %r12 */ 
	leaq -52(%rbp), %r15	/* %r15 points to PixelAverager struct */

	movl %esi, %r8d 		/* move row index from %esi to %r8d */ 
	movl %edx, %r9d			/* move col index from %edx to %r9d */

	movl %r8d, %eax			/* move row index (dividend) from %r8d to %eax */
    movl $0, %edx			/* move 0 to %edx to clear the high bits for 32-bit dividend */  
	movl $2, %r8d			/* move 2 to %r8d (divl requires register) */
    divl %r8d				/* divide row index in %edx:%eax by %r8d and return quotient in %eax */
    movl %eax, %r13d       	/* move new row index from %eax to %r13d */

	movl %r9d, %eax			/* move col index (dividend) from %r9d to %eax */
    movl $0, %edx			/* move 0 to %edx to clear the high bits for 32-bit dividend */  
    divl %r8d				/* divide col index in %edx:%eax by %r8d and return quotient in %eax */
    movl %eax, %r14d       	/* move new col index from %eax to %r14d */

	/* initialize PixelAverager struct: pa_init(pa) */
	movq %r15, %rdi 		/* move PixelAverager pointer from %rbx to %rdi (1st arg for pa_init) */ 
	call pa_init			/* call pa_init */ 

	/* aggregate 2 horizontal pixels: pa_update_from_img(img, new_row, new_col, pa), pa_update_from_img(img, new_row + 1, new_col, pa) */
	movq %r12, %rdi 			/* move image pointer from %r12 to %rdi (1st arg for pa_update_from_img) */
	movl %r13d, %esi 			/* move row index from %r13d to %esi (2nd arg for pa_update_from_img) */
	movl %r14d, %edx 			/* move col index from %r14d to %edx (3rd arg for pa_update_from_img) */
	movq %r15, %rcx 			/* move PixelAverager pointer from %r15 to %rcx (4th arg for pa_update_from_img) */
	call pa_update_from_img 	/* call pa_update_from_img to average PixelAverager */

	movq %r12, %rdi 			/* move image pointer from %r12 to %rdi (1st arg for pa_update_from_img) */
	movl %r13d, %esi  			/* move row index from %r13d to %esi (2nd arg for pa_update_from_img) */
	movl %r14d, %edx 			/* move col index from %r14d to %edx (3rd arg for pa_update_from_img) */
	movq %r15, %rcx 			/* move PixelAverager pointer from %r15 to %rcx (4th arg for pa_update_from_img) */
	call pa_update_from_img 	/* call pa_update_from_img to average PixelAverager */
	
	/* average 2 horizontal pixels including alpha: pa_avg_pixel(pa, 0, 0) */
	movq %r15, %rdi 		/* move PixelAverager pointer form %r15 to %rdi (1st arg for pa_avg_pixel) */
	movl $0, %esi 			/* move 0 to %esi (2nd arg for pa_avg_pixel, indicates we use the average alpha value) */
	movl $0, %edx 			/* move 0 to %edx (3rd arg for pa_avg_pixel, value won't be use) */
	call pa_avg_pixel		/* call pa_avg_pixel and return averaged pixel in %eax*/

	/* epilogue */
	addq $32, %rsp 		/* remove alignment */ 
	popq %r15 			/* restore callee-saved registers */
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

    ret		/* return pixel in %eax */

	.globl expand_even_odd
expand_even_odd:
    ret

	.globl expand_odd_even
expand_odd_even:
    ret

	.globl expand_odd_odd
expand_odd_odd:
    ret

/* 
 * Sets all colour accumulations and pixel count to zero for PixelAverager struct 
 * C function: void pa_init( struct PixelAverager *pa ) 
 * 
 * Parameters: 
 * 	%rdi - 64-bit PixelAverager pointer 
 * 
 */ 
	.globl pa_init
pa_init:
	movl $0, PA_R_OFFSET(%rdi)			/* move 0 to R in PixelAverager */  
	movl $0, PA_G_OFFSET(%rdi) 			/* move 0 to G in PixelAverager */  
	movl $0, PA_B_OFFSET(%rdi) 			/* move 0 to B in PixelAverager */  
	movl $0, PA_A_OFFSET(%rdi) 			/* move 0 to alpha in PixelAverager */  
	movl $0, PA_COUNT_OFFSET(%rdi) 		/* move 0 to count in PixelAverager */  

    ret

/* 
 * Adds the RGB and alpha values of the pixel to the running total for PixelAverager struct 
 * C function: void pa_update( struct PixelAverager *pa, uint32_t pixel )
 * 
 * Parameters: 
 * 	%rdi - 64-bit PixelAverager pointer 
 * 	%esi - 32-bit pixel value 
 *
 * Register use: 
 * 	%r12 - PixelAverager pointer 
 * 	%r13d - pixel 
 * 
 * Memory use: 
 *	-8(%rbp) - %r12 (PixelAverager pointer)
 * 	-16(%rbp) - %r13d (pixel)
 */ 
	.globl pa_update
pa_update:
	/* prologue */ 
	pushq %rbp 			/* save previous value in %rbp */ 
	movq %rsp, %rbp 	/* save top of stack from %rsp to %rbp */ 
	pushq %r12			/* will store PixelAverager pointer */
	pushq %r13			/* will store pixel */ 
	movq %rdi, %r12		/* move PixelAverager pointer from %rdi to %r12 */ 
	movl %esi, %r13d 	/* move pixel from %esi to %r13d */ 

	/* extract and add RGB and alpha values: get_r(pixel), get_g(pixel), get_b(pixel), get_a(pixel) */
	movl %r13d, %edi 				/* move pixel from %r13d to %edi (1st arg for get_r) */
	call get_r						/* call get_r and return in %eax */ 
	addl %eax, PA_R_OFFSET(%r12)	/* add pixel R value with PixelAverager R value and store in offset %r12 */ 

	movl %r13d, %edi				/* move pixel from %r13d to %edi (1st arg for get_g) */
	call get_g 						/* call get_g and return in %eax */ 
	addl %eax, PA_G_OFFSET(%r12)	/* add pixel G value with PixelAverager G value and store in offset %r12 */ 

	movl %r13d, %edi				/* move pixel from %r13d to %edi (1st arg for get_b) */
	call get_b						/* call get_b and return in %eax */ 
	addl %eax, PA_B_OFFSET(%r12)	/* add pixel G value with PixelAverager G value and store in offset %r12 */ 

	movl %r13d, %edi				/* move pixel from %r13d to %edi (1st arg for get_a) */
	call get_a						/* call get_a and return in %eax */ 
	addl %eax, PA_A_OFFSET(%r12)	/* add pixel alpha value with PixelAverager alpha value and store in offset %r12 */ 

	incl PA_COUNT_OFFSET(%r12)		/* increment PixelAverager count */ 

	/* epilogue */ 
	popq %r13 		/* restore callee-saved registers */
	popq %r12 
	popq %rbp 	

    ret

/* 
 * Adds the RGB and alpha values of the pixel from the image to the running total for PixelAverager struct 
 * C function: void pa_update_from_img( struct Image *img, int32_t row, int32_t col, struct PixelAverager *pa )
 * 
 * Parameters: 
 * 	%rdi - 64-bit Image pointer 
 * 	%esi - 32-bit row index 
 *	%edx - 32-bit col index 
 * 	%rcx - 64-bit PixelAverager pointer 
 *
 * Register use: 
 * 	%r12 - Image pointer 
 * 	%r13d - row index 
 * 	%r14d - col index 
 * 	%r15 - PixelAverager pointer 
 * 
 * Memory use: 
 *	-8(%rbp) - %r12 (Image pointer)
 * 	-16(%rbp) - %r13d (row index)
 * 	-24(%rbp) - %r14d (col index)
 * 	-32(%rbp) - %r15 (PixelAverager pointer)
 */ 
	.globl pa_update_from_img
pa_update_from_img:
	/* prologue */ 
	pushq %rbp 				/* save previous value in %rbp */ 
	movq %rsp, %rbp 		/* save top of stack from %rsp to %rbp */ 
	pushq %r12				/* will store image pointer */
	pushq %r13				/* will store row index */ 
	pushq %r14 				/* will store col index */
	pushq %r15 				/* will store PixelAverager pointer */
	movq %rdi, %r12			/* move image pointer from %rdi to %r12 */ 
	movl %esi, %r13d 		/* move row index from %esi to %r13d */ 
	movl %edx, %r14d		/* move col index from %edx to %r14d */
	movq %rcx, %r15			/* move PixelAverage pointer from %rcx to %r15 */
	
	/* checks for valid index: valid_index(img, row, col) */
	call valid_index						/* call valid_index and return in %eax (parameters already in correct registers) */ 
	cmpl $0, %eax 							/* compare 0 with return from valid_index */
	je .Lpa_update_from_img_done 			/* jump to end if return value == 0 (invalid index) */

	/* find pixel index: compute_index(img, row, col) */ 
	movq %r12, %rdi 		/* move image pointer from %r12 to %rdi (1st arg for compute_index) */ 
	movl %r13d, %esi 		/* move output row index from %r13d to %esi (2nd arg for compute_index) */  
	movl %r14d, %edx 		/* move output col index from %r14d to %edx (3rd arg for compute_index) */ 
	call compute_index		/* call compute_index and return pixel index in %eax */ 

    /* updates PixelAverager from image: pa_update(pa, pixel) */
	movq %r15, %rdi 						/* move PixelAverager pointer from %r15 to %rdi (1st arg for pa_updpate) */
	movq IMAGE_DATA_OFFSET(%r12), %r8 		/* move image data pointer from %r12 offset to %r8 */
	movl (%r8, %rax, 4), %esi 				/* move pixel at image index from %r8 + (%rax * 4) to %esi (2nd arg for pa_update) */ 
	call pa_update 							/* call pa_update to update PixelAverager pointer */

.Lpa_update_from_img_done: 
	/* epilogue */
	popq %r15 		/* restore callee-saved registers */
	popq %r14 
	popq %r13 
	popq %r12 
	popq %rbp 	

	ret 			

/* 
 * Computes the average of each accumulation from PixelAverager struct and returns a pixel 
 * C function: uint32_t pa_avg_pixel( struct PixelAverager *pa, uint32_t default_alpha, uint32_t img_alpha )
 * 
 * Parameters: 
 * 	%rdi - 64-bit PixelAverager pointer 
 * 	%esi - 32-bit alpha indicator (1 to set a default value, 0 otherwise )
 *	%edx - 32-bit default alpha value 
 *
 * Register use: 
 * 	%r12 - PixelAverager pointer 
 * 	%r13d - alpha indicator
 * 	%r14d - default alpha value 
 * 	%r15d - count 
 * 	%ebx - average R
 * 
 * Memory use: 
 *	-8(%rbp) - %r12 (PixelAverager pointer)
 * 	-16(%rbp) - %r13d (alpha indicator)
 * 	-24(%rbp) - %r14d (default alpha value)
 * 	-32(%rbp) - %r15d count
 * 	-40(%rbp) - %ebx (average R)
 * 	-44(%rbp) - stack: average G 
 *	-48(%rbp) - stack: average B
 * 	-52(%rbp) - stack: average alpha 
 * 	-64(%rbp) - %rsp stack alignment  
 */ 
	.globl pa_avg_pixel
pa_avg_pixel:
	/* prologue */
	pushq %rbp 							/* save previous value in %rbp */ 
	movq %rsp, %rbp 					/* save top of stack from %rsp to %rbp */ 
	pushq %r12							/* will store PixelAverager pointer */
	pushq %r13							/* will store alpha indicator */ 
	pushq %r14 							/* will store default alpha value */
	pushq %r15 							/* will store count */
	pushq %rbx 							/* will store average R */
	subq $24, %rsp 						/* align %rsp to -64 */
	movq %rdi, %r12						/* move PixelAverager pointer from %rdi to %r12 */ 
	movl %esi, %r13d 					/* move alpha indicator from %esi to %r13d */ 
	movl %edx, %r14d					/* move default alpha value from %edx to %r14d */
	movl PA_COUNT_OFFSET(%r12), %r15d	/* move count from %r12 offset to %r15d */

	cmpl $0, %r15d 				/* compare 0 with count*/
	je .Lpa_avg_pixel_count0	/* jump to count of 0 if return value == 0 */

	movl PA_R_OFFSET(%r12), %eax	/* move R sum (dividend) from %r12 offset to %eax */
    movl $0, %edx					/* move 0 to %edx to clear the high bits for 32-bit dividend */  
    divl %r15d						/* divide R sum in %edx:%eax by count in %r15d and return quotient in %eax */
    movl %eax, %ebx       			/* move average R from %eax to %ebx */

	movl PA_G_OFFSET(%r12), %eax 	/* move G sum (dividend) from %r12 offset to %eax */
	movl $0, %edx					/* move 0 to %edx to clear the high bits for 32-bit dividend */  
	divl %r15d  					/* divide G sum in %edx:%eax by count in %r15d and return quotient in %eax */
	movl %eax, -44(%rbp) 			/* move average G from %eax to -44(%rbp) */

	movl PA_B_OFFSET(%r12), %eax 	/* move B sum (dividend) from %r12 offset to %eax */
	movl $0, %edx					/* move 0 to %edx to clear the high bits for 32-bit dividend */  
	divl %r15d  					/* divide B sum in %edx:%eax by count in %r15d and return quotient in %eax */
	movl %eax, -48(%rbp) 			/* move average B from %eax to -48(%rbp) */

	cmpl $1, %r13d 						/* compare 1 with alpha indicator */
	je .Lpa_avg_pixel_default_alpha		/* jump to default alpha if return value == 1 (set default alpha) */

	movl PA_A_OFFSET(%r12), %eax 	/* move alpha sum (dividend) from %r12 offset to %eax */
	movl $0, %edx					/* move 0 to %edx to clear the high bits for 32-bit dividend */  
	divl %r15d  					/* divide alpha sum in %edx:%eax by count in %r15d and return quotient in %eax */
	movl %eax, -52(%rbp) 			/* move average alpha from %eax to -52(%rbp) */

	jmp .Lpa_avg_pixel_make	/* jump to make pixel */

.Lpa_avg_pixel_count0: 
	movl PA_R_OFFSET(%r12), %ebx 		/* move R from %r12 offset to %ebx */
	movl PA_G_OFFSET(%r12), %eax 		/* move G from %r12 offset to %eax (can't move between memory locations directly) */
	movl %eax, -44(%rbp) 				/* move G from %eax to -44(%rbp) */
	movl PA_B_OFFSET(%r12), %eax		/* move B from %r12 offset to %eax (can't move between memory locations directly) */
	movl %eax, -48(%rbp) 				/* move B from %eax to -48(%rbp) */
	movl PA_A_OFFSET(%r12), %eax		/* move alpha from %r12 offset to %eax (can't move between memory locations directly) */
	movl %eax, -52(%rbp)				/* move alpha from %eax to -52(%rbp) */

	jmp .Lpa_avg_pixel_make		/* jump to make pixel */

.Lpa_avg_pixel_default_alpha: 
	movl %r14d, -52(%rbp) 		/* move default alpha from %r14d to -52(%rbp) */

.Lpa_avg_pixel_make: 
	/* make pixel from average values: make_pixel(r, g, b, a) */
	movl %ebx, %edi 			/* move output R from %ebx to %edi (1st arg for make_pixel) */ 
	movl -44(%rbp), %esi 		/* move output G from -44(%rbp) to %esi (2nd arg for make_pixel) */ 
	movl -48(%rbp), %edx 		/* move output B from -48(%rbp) to %edx (3rd arg for make_pixel) */ 
	movl -52(%rbp), %ecx 		/* move alpha from -52(%rbp) to %ecx (4th arg for make_pixel) */ 
	call make_pixel				/* call make_pixel and return in %eax */ 

.Lpa_avg_pixel_done: 
	/* epilogue */
	addq $24, %rsp	/* remove alignment */ 
	popq %rbx		/* restore callee-saved registers */
	popq %r15
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

    ret		/* return pixel in %eax */

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
  * 	%rdi - 64-bit input Image pointer 
  * 	%rsi - 64-bit output Image pointer 
  *		%edx - 32-bit xfac 
  *		%ecx - 32-bit yfac 
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
  *		-24(%rbp) - %r14d (xfac) 
  * 	-32(%rbp) - %r15d (yfac) 
  *		-40(%rbp) - %ebx (current output row index) 
  * 	-48(%rbp) - %rsp stack alignment 
  * 	
  */ 
	.globl imgproc_squash
imgproc_squash:
	/* prologue */ 
	pushq %rbp 			/* save previous value in %rbp */ 
	movq %rsp, %rbp 	/* save top of stack from %rsp to %rbp */ 
	pushq %r12			/* will store input image pointer */
	pushq %r13 			/* will store output image pointer */ 
	pushq %r14			/* will store xfac */ 
	pushq %r15 			/* will store yfac */ 
	pushq %rbx 			/* will store current index */ 
	subq $8, %rsp 		/* align %rsp to be a multiple of 16 */
	movq %rdi, %r12 	/* move input image pointer from %rdi to %r12 */ 
	movq %rsi, %r13 	/* move output image pointer from %rsi to %r13 */ 
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

	movq %r13, %rdi 						/* move ouput image pointer from %r13 to %rdi (1st arg for img_init) */ 
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
	incl %ebx 					/* increment row index */ 
	jmp .Lsquash_loop_row 		/* repeat row loop */ 

.Lsquash_done: 
	/* epilogue */ 
	addq $8, %rsp	/* remove alignment */ 
	popq %rbx 		/* restore callee-saved registers */
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

 /* 
  * Applies a rotation to the colour component vaues in each input pixel 
  * C function: void imgproc_color_rot( struct Image *input_img, struct Image *output_img )
  * 
  * Parameters: 
  * 	%rdi - 64-bit input Image pointer 
  * 	%rsi - 64-bit output Image pointer 
  *
  * Register use: 
  * 	%r12 - input Image pointer 
  * 	%r13 - output Image pointer 
  * 	%r14d - current output row index 
  *		%r15d - current output col index 
  * 	%ebx - current index 
  * 
  * Memory use: 
  *		-8(%rbp) - %r12 (input Image pointer)
  * 	-16(%rbp) - %r13 (output Image pointer)
  * 	-24(%rbp) - %r14d (current output row index) 
  * 	-32(%rbp) - %r15d (current output col index)
  * 	-40(%rbp) - %ebx (current index)
  *		-48(%rbp) - %rsp stack alignment 
  * 	
  */ 
	.globl imgproc_color_rot
imgproc_color_rot:
	/* prologue */ 
	pushq %rbp 			/* save previous value in %rbp */ 
	movq %rsp, %rbp 	/* save top of stack from %rsp to %rbp */ 
	pushq %r12			/* will store input image pointer */
	pushq %r13 			/* will store output image pointer */ 
	pushq %r14			/* will store current output row index */ 
	pushq %r15 			/* will store current output col index */ 
	pushq %rbx			/* will store current index */ 
	subq $8, %rsp 		/* align %rsp to be a multiple of 16 */
	movq %rdi, %r12 	/* move input image pointer from %rdi to %r12 */ 
	movq %rsi, %r13 	/* move output image pointer from %rsi to %r13 */ 

	/* initializes output image dimensions: img_init(output_img, input_img->width, input_img->height) */ 
	movq %r13, %rdi								/* move output image pointer from %r13 to %rdi (1st arg for img_init) */ 
	movl IMAGE_WIDTH_OFFSET(%r12), %esi 		/* dereference pointer to width (32-bit) in %r12 and move to %esi (2nd arg for img_init) */ 
	movl IMAGE_HEIGHT_OFFSET(%r12), %edx 		/* dereference pointer to height (32-bit) in %r12 and move to %edx (3rd arg for img_init) */ 
	call img_init								/* call img_init and return in %eax */ 

	cmpl $0, %eax 		/* compare 0 with return from img_init */
	jne .Lrot_done		/* jump to end if return value != 0 */

	movl $0, %r14d 		/* move 0 to %r14 (row index) */ 

.Lrot_loop_row: 
	cmpl IMAGE_HEIGHT_OFFSET(%r13), %r14d	/* compare output image height with row index */
	jge .Lrot_done 							/* jump to end if row index >= output image height */

	movl $0, %r15d 		/* move 0 to %r15d (col index) */ 

.Lrot_loop_col: 
	cmpl IMAGE_WIDTH_OFFSET(%r13), %r15d 	/* compare output image width with col index */ 
	jge .Lrot_next_row						/* jump to end if col index >= output image width */ 

	/* find output index: compute_index(input_img, row, col) */ 
	movq %r12, %rdi			/* move input image pointer from %r12 to %rdi (1st arg for compute_index) */ 
	movl %r14d, %esi 		/* move row index from %r14d to %esi (2nd arg for compute_index) */ 
	movl %r15d, %edx 		/* move col index from %r15d to %edx (3rd arg for compute_index) */ 
	call compute_index 		/* call compute_index and return index in %eax */ 
	movl %eax, %ebx 		/* move index from %eax to %ebx */ 

	/* get output pixel: rot_colors(input_img, index) */ 
	movq %r12, %rdi 		/* move input image pointer from %r12 to %rdi (1st arg for compute_index) */ 
	movl %ebx, %esi 		/* move index from %ebx to %esi */ 
	call rot_colors 		/* call rot_colors and return new pixel in %eax */ 

	/* assign new pixel to output image pixel */ 
	movq IMAGE_DATA_OFFSET(%r13), %r8		/* dereference pointer to output image pixel and move from %r13 to %r8 */
	movl %eax, (%r8, %rbx, 4)				/* move new pixel from %eax to pixel at output image index at %r8 + (%rax * 4) */ 

	incl %r15d 				/* increment col index */
	jmp .Lrot_loop_col		/* repeat col loop */ 

.Lrot_next_row: 
	incl %r14d				/* increment row index */ 
	jmp .Lrot_loop_row		/* repeat row loop */ 

.Lrot_done: 
	/* epilogue */ 
	addq $8, %rsp	/* remove alignment */ 
	popq %rbx 		/* restore callee-saved registers */
	popq %r15 
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

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

/*
 * Transforms the input image using a blur effect 
 * C function: void imgproc_blur( struct Image *input_img, struct Image *output_img, int32_t blur_dist )
 * 
 * Parameters: 
 * 	%rdi - 64-bit input Image pointer 
 * 	%rsi - 64-bit output Image pointer 
 *	%edx - 32-bit blur dist  
 *
 * Register use: 
 * 	%r12 - input Image pointer 
 * 	%r13 - output Image pointer 
 *	%r14d - blur dist 
 * 	%r15d - current row index 
 *	%ebx - current col index 
 * 
 * Memory use: 
 *	-8(%rbp) - %r12 (input Image pointer)
 * 	-16(%rbp) - %r13 (output Image pointer)
 *	-24(%rbp) - %r14d (blur dist) 
 * 	-32(%rbp) - %r15d (current row index) 
 *	-40(%rbp) - %ebx (current col index) 
 *	-44(%rbp) - stack: blurred pixel 
 * 	-48(%rbp) - %rsp stack alignment 
 * 	
 */ 
	.globl imgproc_blur
imgproc_blur:
	/* TODO: implement */
	/* prologue */ 
	pushq %rbp 			/* save previous value in %rbp */ 
	movq %rsp, %rbp 	/* save top of stack from %rsp to %rbp */ 
	pushq %r12			/* will store input image pointer */
	pushq %r13 			/* will store output image pointer */ 
	pushq %r14			/* will store blur dist */ 
	pushq %r15 			/* will store current row index */ 
	pushq %rbx			/* will store current col index */ 
	subq $8, %rsp 		/* align %rsp to -48 */
	movq %rdi, %r12 	/* move input image pointer from %rdi to %r12 */ 
	movq %rsi, %r13 	/* move output image pointer from %rsi to %r13 */ 
	movl %edx, %r14d

	/* initializes output image dimensions: img_init(output_img, input_img->width, input_img->height) */ 
	movq %r13, %rdi								/* move output image pointer from %r13 to %rdi (1st arg for img_init) */ 
	movl IMAGE_WIDTH_OFFSET(%r12), %esi 		/* dereference pointer to width (32-bit) in %r12 and move to %esi (2nd arg for img_init) */ 
	movl IMAGE_HEIGHT_OFFSET(%r12), %edx 		/* dereference pointer to height (32-bit) in %r12 and move to %edx (3rd arg for img_init) */ 
	call img_init								/* call img_init and return in %eax */ 

	cmpl $0, %eax 					/* compare 0 with return from img_init */
	jne .Limgproc_blur_done			/* jump to end if return value != 0 */

	movl $0, %r15d 		/* move 0 to %r15d (row index) */ 

.Limgproc_blur_loop_row: 
	cmpl IMAGE_HEIGHT_OFFSET(%r13), %r15d		/* compare output image height with row index */
	jge .Limgproc_blur_done						/* jump to end if row index >= image height */

	movl $0, %ebx 	/* move 0 to %ebx (col index) */ 

.Limgproc_blur_loop_col: 
	cmpl IMAGE_WIDTH_OFFSET(%r13), %ebx 	/* compare output image width with col index */ 
	jge .Limgproc_blur_next_row				/* jump to end if col index >= output image width */ 

	/* get blurred pixel and index: blur_pixel(input_img, row, col, blur_dist), compute_index(input_img, row, col) */
	movq %r12, %rdi 		/* move input image pointer from %r12 to %rdi (1st arg for blur_pixel) */ 
	movl %r15d, %esi 		/* move row index from %r15d to %esi (2nd arg for blur_pixel) */
	movl %ebx, %edx 		/* move col index from %ebx to %edx (3rd arg for blur_pixel) */
	movl %r14d, %ecx 		/* move blur dist from %r14d to %ecx (4th arg for blur_pixel) */
	call blur_pixel 		/* call blur_pixel and return in %eax */
	movl %eax, -44(%rbp)	/* move blurred pixel from %eax to -44(%rbp) */

	/* get index to add blurred pixel: compute_index(output_img, row, col)*/
	movq %r13, %rdi 		/* move output image pointer from %r13 to %rdi (1st arg for blur_pixel) */ 
	movl %r15d, %esi 		/* move row index from %r15d to %esi (2nd arg for blur_pixel) */
	movl %ebx, %edx 		/* move col index from %ebx to %edx (3rd arg for blur_pixel) */
	call compute_index		/* call compute_index and return in %eax (arguments align with blur_pixel) */

	/* assign new pixel to output image pixel */ 
	movq IMAGE_DATA_OFFSET(%r13), %r8		/* dereference pointer to output image pixel and move from %r13 to %r8 */
	movl -44(%rbp), %r9d					/* move blurred pixel from -44(%rbp) to %r9d (can't move between memory locations directly)*/
	movl %r9d, (%r8, %rax, 4)				/* move new pixel from %r9d to pixel at output image index at %r8 + (%rax * 4) */ 

	incl %ebx 						/* increment col index */
	jmp .Limgproc_blur_loop_col		/* repeat col loop */ 

.Limgproc_blur_next_row: 
	incl %r15d 						/* increment row index */
	jmp .Limgproc_blur_loop_row		/* repeat row loop */

.Limgproc_blur_done: 
	/* epilogue */
	addq $8, %rsp	/* remove alignment */ 
	popq %rbx 		/* restore callee-saved registers */
	popq %r15 
	popq %r14
	popq %r13
	popq %r12 
	popq %rbp 

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
