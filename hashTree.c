#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h> // for EINTR
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h> // for gettimeofday

typedef struct
{
  size_t index;           // The index of the current thread
  size_t blocksPerThread; // Total number of blocks
  size_t maxThreads;      // Maximum number of threads allowed
  uint8_t *fileInMemory;  // Pointer to the start of the memory-mapped file
} ThreadData, *ThreadDataPtr;

void Usage(char *);
uint32_t jenkins_one_at_a_time_hash(const uint8_t *, uint64_t);
void *thread_function(void *arg);

// block size
#define BSIZE 4096

int main(int argc, char **argv)
{
  int32_t fd;
  uint32_t nblocks;

  // input checking
  if (argc != 3)
    Usage(argv[0]);

  // open input file
  fd = open(argv[1], O_RDWR);
  if (fd == -1)
  {
    perror("open failed");
    exit(EXIT_FAILURE);
  }

  // * use fstat to get file size
  struct stat sb;
  if (fstat(fd, &sb) == -1)
  {
    perror("fstat");
    close(fd);
    exit(EXIT_FAILURE);
  }

  // * Mapping the file
  uint8_t *fileInMemory = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (fileInMemory == MAP_FAILED)
  {
    perror("mmap");
    close(fd);
    exit(EXIT_FAILURE);
  }

  // * Calculate blocks and blocks per thread
  int numberOfThreads = atoi(argv[2]);
  nblocks = sb.st_size / BSIZE;
  int blocksPerThread = nblocks / numberOfThreads;

  // * print number of threads and blocks per thread
  printf("num Threads = %u \n", numberOfThreads);
  printf("Blocks per Thread = %u \n", blocksPerThread);

  // * Initialize the root node
  ThreadData rootNode = {0};
  rootNode.index = 0;                         // changes on every call
  rootNode.blocksPerThread = blocksPerThread; // constant
  rootNode.maxThreads = numberOfThreads;      // constant
  rootNode.fileInMemory = fileInMemory;       // constant

  struct timeval start, end;
  double elapsedTime;

  // * Record the start time
  if (gettimeofday(&start, NULL) != 0)
  {
    perror("gettimeofday");
    exit(EXIT_FAILURE);
  }

  // * Calculate the hash
  int *hash = thread_function(&rootNode);

  // * Record the end time
  if (gettimeofday(&end, NULL) != 0)
  {
    perror("gettimeofday");
    exit(EXIT_FAILURE);
  }

  // * Calculate the elapsed time in microseconds
  elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;    // sec to ms
  elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0; // us to ms

  // * Print the final hash and the time taken
  printf("hash value = %u \n", *hash);
  printf("Time taken = %.3f milliseconds\n", elapsedTime);

  // * Cleanup
  munmap(fileInMemory, sb.st_size);
  free(hash);
  close(fd);
  return EXIT_SUCCESS;
}

void *thread_function(void *arg)
{
  // Use a pointer to access the data from the argument
  ThreadData data = *((ThreadData *)arg);

  // Determine if children threads should be created based on current index
  ThreadData leftNode = *((ThreadData *)arg);  // Copy parent data
  ThreadData rightNode = *((ThreadData *)arg); // Copy parent data
  int leftIndex = (2 * data.index) + 1;        // Calculate index for left child: 2i + 1
  int rightIndex = (2 * data.index) + 2;       // Calculate index for right child: 2i + 2

  int leftAllowed = leftIndex < data.maxThreads;
  int rightAllowed = rightIndex < data.maxThreads;

  // Create threads
  pthread_t p1, p2;

  // Create and configure data for the left child if allowed
  if (leftAllowed)
  {
    leftNode.index = leftIndex; // Set specific index for left child
    if (pthread_create(&p1, NULL, thread_function, &leftNode) != 0)
    {
      perror("pthread_create left");
      exit(EXIT_FAILURE);
    }
  }

  // Create and configure data for the right child if allowed
  if (rightAllowed)
  {
    rightNode.index = rightIndex; // Set specific index for right child
    if (pthread_create(&p2, NULL, thread_function, &rightNode) != 0)
    {
      perror("pthread_create right");
      exit(EXIT_FAILURE);
    }
  }

  // Calculate the hash of the current thread
  size_t startOffset = data.index * data.blocksPerThread * BSIZE; // to prevent overflow of the file
  uint8_t *start = data.fileInMemory + startOffset;
  size_t length = data.blocksPerThread * BSIZE;

  uint32_t hash = jenkins_one_at_a_time_hash(start, length);

  // wait for children to finish and get their hashes
  void *leftHashResult;
  void *rightHashResult;

  // if left thread exists, wait for it to finish
  if (leftAllowed)
    if (pthread_join(p1, &leftHashResult) != 0)
    {
      perror("pthread_join");
      exit(EXIT_FAILURE);
    }

  // if right thread exists, wait for it to finish
  if (rightAllowed)
    if (pthread_join(p2, &rightHashResult) != 0)
    {
      perror("pthread_join");
      exit(EXIT_FAILURE);
    }

  // Final hash memory allocation
  uint32_t *combinedHash = (uint32_t *)malloc(sizeof(uint32_t) * 1);
  uint8_t buffer[100]; // this should be 31. 10 for each hash and 1 for the null terminator

  // Concatenate the hashes
  int bufLen;
  if (leftAllowed && rightAllowed)
  {
    bufLen = sprintf((char *)buffer, "%u%u%u", hash, *(uint32_t *)leftHashResult, *(uint32_t *)rightHashResult);
    *combinedHash = jenkins_one_at_a_time_hash(buffer, bufLen);
    free(leftHashResult);
    free(rightHashResult);
  }
  else if (leftAllowed)
  {
    bufLen = sprintf((char *)buffer, "%u%u", hash, *(uint32_t *)leftHashResult);
    *combinedHash = jenkins_one_at_a_time_hash(buffer, bufLen);
    free(leftHashResult);
  }
  else
    *combinedHash = hash;

  return combinedHash;
}

uint32_t jenkins_one_at_a_time_hash(const uint8_t *key, uint64_t length)
{
  uint64_t i = 0;
  uint32_t hash = 0;

  while (i != length)
  {
    hash += key[i++];
    hash += hash << 10;
    hash ^= hash >> 6;
  }
  hash += hash << 3;
  hash ^= hash >> 11;
  hash += hash << 15;
  return hash;
}

void Usage(char *s)
{
  fprintf(stderr, "Usage: %s filename num_threads \n", s);
  exit(EXIT_FAILURE);
}
