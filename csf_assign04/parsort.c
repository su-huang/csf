/*
 * C implementations of parallel quicksort 
 * CSF Assignment 4 
 * Su Huang 
 * shuan148@jh.edu
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int compare_i64(const void *left_, const void *right_) {
  int64_t left = *(int64_t *)left_;
  int64_t right = *(int64_t *)right_;
  if (left < right) return -1;
  if (left > right) return 1;
  return 0;
}

void seq_sort(int64_t *arr, size_t begin, size_t end) {
  size_t num_elements = end - begin;
  qsort(arr + begin, num_elements, sizeof(int64_t), compare_i64);
}

// Merge the elements in the sorted ranges [begin, mid) and [mid, end),
// copying the result into temparr.
void merge(int64_t *arr, size_t begin, size_t mid, size_t end, int64_t *temparr) {
  int64_t *endl = arr + mid;
  int64_t *endr = arr + end;
  int64_t *left = arr + begin, *right = arr + mid, *dst = temparr;

  for (;;) {
    int at_end_l = left >= endl;
    int at_end_r = right >= endr;

    if (at_end_l && at_end_r) break;

    if (at_end_l)
      *dst++ = *right++;
    else if (at_end_r)
      *dst++ = *left++;
    else {
      int cmp = compare_i64(left, right);
      if (cmp <= 0)
        *dst++ = *left++;
      else
        *dst++ = *right++;
    }
  }
}

void fatal(const char *msg) __attribute__ ((noreturn));

void fatal(const char *msg) {
  fprintf(stderr, "Error: %s\n", msg);
  exit(1);
}

// child data type 
typedef struct {
  int created;    // 1 if fork succeeded, 0 otherwise
  pid_t pid;      // pid of child 
  int waited;     // 1 if waitpid has been called
  int exit_code;  // exit code of child 
  int exited;     // 1 if child exited normally 
} Child;

// pre-declare merge_sort 
void merge_sort(int64_t *arr, size_t begin, size_t end, size_t threshold);

// forks a portion of the array and returns its child struct 
Child quicksort_subproc(int64_t *arr, size_t begin, size_t end, size_t threshold) { 
  Child c = {0}; 
 
  pid_t child_pid = fork();
  if (child_pid == 0) {
    // executing in the child
    merge_sort(arr, begin, end, threshold);
    exit(0);
  } else if (child_pid < 0) {
    // fork failed
    fprintf(stderr, "error: fork failed\n");
    c.created = 0;
  } 
  
  // in parent
  c.created = 1;
  c.pid = child_pid;
  return c;
}

// waits for a child process to finish and stores the result in its child struct 
void quicksort_wait(Child *c) {
  if (!c->created) {
    // child wasn't created 
    return;
  }
 
  int rc, wstatus;
  rc = waitpid(c->pid, &wstatus, 0);
  c->waited = 1;
 
  if (rc < 0) {
    // waitpid failed
    fprintf(stderr, "error: waitpid failed\n");
    c->exited = 0;
    return;
  }
 
  if (!WIFEXITED(wstatus)) {
    // child did not exit normally
    fprintf(stderr, "error: child did not exit normally\n");
    c->exited = 0;
    return;
  }
 
  c->exited = 1;
  c->exit_code = WEXITSTATUS(wstatus);
}

// returns 1 if the child was created, waited on, exited normally, exited with code 0
// otherwise returns 0 
int quicksort_check_success(const Child *c) {
  if (!c->created) return 0;        // child wasn't created 
  if (!c->waited) return 0;         // waitpid wasn't called 
  if (!c->exited) return 0;         // child didn't exit normally 
  if (c->exit_code != 0) return 0;  // child exited with a non-zero exit code
  return 1;
}

void merge_sort(int64_t *arr, size_t begin, size_t end, size_t threshold) {
  assert(end >= begin);
  size_t size = end - begin;

  if (size <= threshold) {
    seq_sort(arr, begin, end);
    return;
  }

  // recursively sort halves in parallel

  size_t mid = begin + size/2;

  // begins child processes for each recursive call
  Child left, right;
  left  = quicksort_subproc(arr, begin, mid, threshold);
  right = quicksort_subproc(arr, mid,   end, threshold);
 
  // waits for both children unconditionally to guarantee no zombies
  quicksort_wait(&left);
  quicksort_wait(&right);
 
  // checks if both children were successful 
  int left_success  = quicksort_check_success(&left);
  int right_success = quicksort_check_success(&right);

  if (!left_success || !right_success)
    fatal("a child process failed during sorting");

  // allocate temp array now, so we can avoid unnecessary work
  // if the malloc fails
  int64_t *temp_arr = (int64_t *) malloc(size * sizeof(int64_t));
  if (temp_arr == NULL)
    fatal("malloc() failed");

  // child processes completed successfully, so in theory
  // we should be able to merge their results
  merge(arr, begin, mid, end, temp_arr);

  // copy data back to main array
  for (size_t i = 0; i < size; i++)
    arr[begin + i] = temp_arr[i];

  // now we can free the temp array
  free(temp_arr);

  // success!
}

int main(int argc, char **argv) {
  // check for correct number of command line arguments
  if (argc != 3) {
    fprintf(stderr, "usage: %s <filename> <sequential threshold>\n", argv[0]);
    return 1;
  }

  // process command line arguments
  const char *filename = argv[1];
  char *end;
  size_t threshold = (size_t) strtoul(argv[2], &end, 10);
  if (end != argv[2] + strlen(argv[2])) {
    // report an error (threshold value is invalid)
    fprintf(stderr, "error: invalid threshold value\n");
    return 1;
  }

  // open the file
  int fd = open(filename, O_RDWR);
    if (fd < 0) {
      perror("open error"); 
      return 1; 
    }

  // use fstat to determine the size of the file
  struct stat statbuf;
  int rc = fstat( fd, &statbuf );
  if ( rc != 0 ) {
      // handle fstat error and exit
      perror("fstat error"); 
      return 1; 
  }
    
  // verify valid byte size 
  size_t file_size_in_bytes = (size_t) statbuf.st_size;
  if (file_size_in_bytes == 0 || file_size_in_bytes % sizeof(int64_t) != 0) {
    fprintf(stderr, "error: invalid file size\n");
    close(fd);
    return 1;
  }

  // map the file into memory using mmap
  int64_t *arr;
  arr = mmap( NULL, file_size_in_bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0 );
  close( fd ); // file can be closed now
  if ( arr == MAP_FAILED ) {
    perror("mmap");
    return 1;
  }

  // sort the data!
  merge_sort(arr, 0, file_size_in_bytes / sizeof(int64_t), threshold);

  // unmap and close the file
  if (munmap(arr, file_size_in_bytes) != 0) {
    perror("munmap");
    return 1;
  }

  // exit with a 0 exit code if sort was successful
  return 0; 
}
