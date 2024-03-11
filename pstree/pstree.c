#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define MAX_PRO 1024


typedef struct Process {
  int pid;
  char name[256];
  int ppid;
  struct Process *child[MAX_PRO];
  int childmount;
} Process;

Process *processes[MAX_PRO];
int process_count = 0;

void getVersion();
void readProcessInfo(Process *proc, int pid);
void buildProcessTree();
void printTree(int showpid, int level, Process *proc);
int compare(const void *a, const void *b);

int main(int argc, char *argv[]) {
  #define OPT(i) (argv[i][1])

  int showpid = 0, sortNumer = 0;

  for (int i = 1; i < argc; i++) {
    if (OPT(i) == 'V') {
      getVersion();
    } else if (OPT(i) == 'p') {
      showpid = 1;
    } else if (OPT(i) == 'n') {
      sortNumer = 1;
    }
  }

  buildProcessTree();

  if (sortNumer) {
    qsort(processes, process_count, sizeof(Process *), compare);
  }
  
  for(int i = 0; i < process_count; i++) {
    if(processes[i]->ppid == 0) {
      printTree(showpid, 0, processes[i]);
    }
  }

  for(int i = 0; i < process_count; i++) {
    free(processes[i]);
  }
  return EXIT_SUCCESS;
}

void getVersion() {
  printf("pstree 1.0\n");
}

void readProcessInfo(Process *proc, int pid) {
  FILE *fp;
  char path[256], line[256];

  sprintf(path, "/proc/%d/status", pid);
  fp = fopen(path, "r");
  if (fp != NULL) {
    while (fgets(line, sizeof(line), fp) != NULL) {
      if (strncmp(line, "Name:", 5) == 0) {
        sscanf(line, "Name:\t%s", proc->name);
      }
      else if (strncmp(line, "PPid:", 5) == 0) {
        sscanf(line, "PPid:\t%d", &proc->ppid);
      }
    }
  }

  fclose(fp);
}

void buildProcessTree() {
  DIR *dir=opendir("/proc");
  struct dirent *entry;

  if (dir!=NULL) {
    while ((entry = readdir(dir)) != NULL) {
      // entry->d_nameÊÇPID
      if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
        Process *proc = malloc(sizeof(Process));
        memset(proc, 0, sizeof(Process));
        proc->pid = atoi(entry->d_name);
        readProcessInfo(proc, proc->pid);
        processes[process_count++] = proc;
      }
    }

    closedir(dir);
  }
  //store the relations between proc
  for(int i=0; i < process_count; i++) {
     for (int j = 0; j < process_count; j++) {
        if (processes[i]->pid == processes[j]->ppid) {
          processes[i]->child[processes[i]->childmount++] = processes[j];
        }
        
     }
  }
}

int compare(const void* a, const void* b) {
    Process* procA = *(Process**)a;
    Process* procB = *(Process**)b;
    return procA->pid - procB->pid;
}

#define PRINT(proc, showpid) (showpid?(printf("%s(%d)\n", proc->name, proc->pid)):(printf("%s\n", proc->name)))  

void printTree(int showpid, int level, Process *proc) {
    for(int i = 0; i < level; i++) {
        printf("|    ");
    }
    if (level > 0) {
        printf("|----");
    }
    PRINT(proc, showpid);

    for(int i = 0; i < proc->childmount; i++ ) {
        printTree(showpid, level + 1, proc->child[i]);
    }
}
