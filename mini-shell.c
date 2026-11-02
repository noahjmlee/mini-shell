#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <pthread.h>

#define buffer_size 32

// struct definition for jobs
typedef struct job {
  int pid;
  char* name;
  struct job* next;
} job;

// Header for jobs list
job* header = NULL;
// Mutex to avoid job list conflicts
pthread_mutex_t mutex;

// Adds a new job to the end of the list
void add_job(int pid, char* name) {
  // Create new job
  job* new_job = (job*)malloc(sizeof(job));
  // Emergency exit if malloc fails
  if (new_job == NULL) exit(1);

  new_job->pid = pid;
  new_job->name = malloc(strlen(name) + 1);
  if (new_job->name) {
    strcpy(new_job->name, name);
  }

  new_job->next = NULL;

  // If header doesn't exist yet, it's the new job
  if (header == NULL) {
    header = new_job;
  }
  // Otherwise, go to the end of the list and assign it
  else {
    job* current = header;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = new_job;
  }
}

// Deletes a job from the list
void del_job(int pid) {
  // Emergency exit if there are no jobs
  if (header == NULL) return;

  // Edge case if header is the target
  if (header->pid == pid) {
    job* temp = header;
    header = header->next;
    free(temp->name);
    free(temp);
    return;
  }

  // Searches list for matching pid
  job* current = header->next;
  job* prev = header;
  while (current != NULL) {
    // When the pid is found, delete it and free allocated memory
    if (current->pid == pid) {
      job* temp = current;
      prev->next = current->next;
      free(temp->name);
      free(temp);
      return;
    }
    current = current->next;
    prev = prev->next;
  }
}

// Prints the list of jobs
void print_jobs() {
  printf("PID\t\t\tProgram\n");
  job* current = header;
  while (current != NULL) {
    printf("%d\t\t\t%s\n", current->pid, current->name);
    current = current->next;
  }
}

// Determines if the user typed "exit"
bool check_exit (char* input) {
  return strcmp("exit\n", input) == 0;
}

// Determines if the user typed "jobs"
bool check_jobs (char* input) {
  return strcmp("jobs\n", input) == 0;
}

// Determines if there is an ampersand call to run program in background
bool check_ampersand(char* input) {
  bool end_ampersand = input[strlen(input) - 2] == '&';
  bool space_before = input[strlen(input) - 3] == ' ';
  return end_ampersand && space_before;
}

// Running program
void * run(void * in) {
  char * input = (char*)in;

  // Cleans up input to be run as a program
  char name[buffer_size];

  // Conditions to stop
  bool stop = false;
  int i = 0;

  // Checks to see if ran into a relevant character
  while (!stop && i < strlen(input)) {
    if (input[i] == ' ' || input[i] == '\n' || input[i] == '\0') {
      stop = true;
      // Adds null terminator to end of cleaned word
      name[i] = '\0';
    }
    else {
      name[i] = input[i];
      i++;
    }
  }

  int pid = fork();

  // Child
  if (pid == 0) {
    // Puts name in command form with ./ beginning
    // Technically unnecessary in my testing but included for thoroughness
    char command[] = "./";
    strcat(command, name);
    // Runs program
    char * args[] = {command, NULL};
    char * envp[] = {NULL};
    int r_value = execve(command, args, envp);

    // Emergency kills child if program doesn't exist
    if (r_value != 0) exit(0);
  }
  // Parent
  else {
    // Add job to list
    pthread_mutex_lock(&mutex);
    add_job(pid, name);
    pthread_mutex_unlock(&mutex);

    // Wait for job to finish
    int status = 0;
    waitpid(-1, &status, 0);

    // Delete job from list
    pthread_mutex_lock(&mutex);
    del_job(pid);
    pthread_mutex_unlock(&mutex);
  }
}

int main (int argc, char *argv[]) {
  // Create mutex
  pthread_mutex_init(&mutex, NULL);

  bool exit = false;
  while (!exit) {
    printf("mini-shell: ");

    // User input
    char buffer[buffer_size];
    fgets(buffer, buffer_size, stdin);
    exit = check_exit(buffer);
    bool jobs = check_jobs(buffer);
    bool background = check_ampersand(buffer);

    // Exit
    if (exit) {
      printf("Goodbye!\n");
    }
    // Jobs
    else if (jobs) {
      pthread_mutex_lock(&mutex);
      print_jobs();
      pthread_mutex_unlock(&mutex);
    }
    // Foreground
    else if (!background) {
      run(buffer);
    }
    // Background
    else {
      pthread_t thread;
      pthread_create(&thread, NULL, run, &buffer);
      pthread_detach(thread);
    }
  }

  // Destroy mutex
  pthread_mutex_destroy(&mutex);
}
