#Test Main function passing parameters

#include <stdio.h>
#include <assert.h>

int main(int argc, char *argv[])
{
  for (int i = 0; i < argc; i++) {
    assert(argv[i]);
    printf("argv[%d] = %c\n", i, argv[i][1]);
  }
  assert(!argv[argc]);
  return 0;
}
