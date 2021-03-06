/**
 * @file execute.c
 *
 * @brief Implements interface functions between Quash and the environment and
 * functions that interpret an execute commands.
 *
 * @note As you add things to this file you may want to change the method signature
 */

// had to add _GNU_SOURCE to suppress an error message
#define _GNU_SOURCE
#include "execute.h"
#include "command.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "quash.h"
#include "deque.h"

// reading and writing from pipes
#define WRITE_END 1
#define READ_END 0

// making process identifier queue
IMPLEMENT_DEQUE_STRUCT(pid_queue, pid_t);
IMPLEMENT_DEQUE(pid_queue, pid_t);
pid_queue p_q;

// declaring struct
struct theJob
{
  int jobID;
  char *cmd;
  pid_queue p_q;
  pid_t pid;
} theJob;

// creating job queue
int numOfJob = 1;
IMPLEMENT_DEQUE_STRUCT(job_queue, struct theJob);
IMPLEMENT_DEQUE(job_queue, struct theJob);
job_queue j_q;

int pipes[3][3];

bool helper;

// Remove this and all expansion calls to it
/**
 * @brief Note calls to any function that requires implementation
 */
#define IMPLEMENT_ME() \
  fprintf(stderr, "IMPLEMENT ME: %s(line %d): %s()\n", __FILE__, __LINE__, __FUNCTION__)

/***************************************************************************
 * Interface Functions
 ***************************************************************************/

// Return a string containing the current working directory.
char *get_current_directory(bool *should_free)
{
  // TODO: Get the current working directory. This will fix the prompt path.
  // HINT: This should be pretty simple
  // IMPLEMENT_ME();

  // Change this to true if necessary
  char *myDirectory = get_current_dir_name();
  *should_free = true;

  return myDirectory;
}

// Returns the value of an environment variable env_var
const char *lookup_env(const char *env_var)
{
  // TODO: Lookup environment variables. This is required for parser to be able
  // to interpret variables from the command line and display the prompt
  // correctly
  // HINT: This should be pretty simple
  // IMPLEMENT_ME();

  char *myEnv = getenv(env_var);

  // return myEnv

  // TODO: Remove warning silencers
  //(void) env_var; // Silence unused variable warning

  return myEnv;
}

// Check the status of background jobs
void check_jobs_bg_status()
{
  // TODO: Check on the statuses of all processes belonging to all background
  // jobs. This function should remove jobs from the jobs queue once all
  // processes belonging to a job have completed.
  // IMPLEMENT_ME();

  // TODO: Once jobs are implemented, uncomment and fill the following line
  // print_job_bg_complete(job_id, pid, cmd);

  // check if no job exists
  if (numOfJob == 0)
  {
    printf("No Jobs found...leaving...");
    return;
  }

  int numOfJob = length_job_queue(&j_q);

  // iterate over the number of jobs present
  // if jobs actually exist
  for (int i = 0; i < numOfJob; i++)
  {
    struct theJob currentJob = pop_front_job_queue(&j_q);

    int numOfPids = length_pid_queue(&currentJob.p_q);

    pid_t atFront = peek_front_pid_queue(&currentJob.p_q);

    for (int j = 0; j < numOfPids; j++)
    {
      pid_t currentPid = pop_front_pid_queue(&currentJob.p_q);

      int theStatus;

      if (waitpid(currentPid, &theStatus, 1) == 0)
        push_back_pid_queue(&currentJob.p_q, currentPid);
    }

    if (is_empty_pid_queue(&currentJob.p_q))
    {
      print_job_bg_complete(currentJob.jobID, atFront, currentJob.cmd);
    }

    else
    {
      push_back_job_queue(&j_q, currentJob);
    }
  }
}

// Prints the job id number, the process id of the first process belonging to
// the Job, and the command string associated with this job
// leaving as it is
void print_job(int job_id, pid_t pid, const char *cmd)
{
  printf("[%d]\t%8d\t%s\n", job_id, pid, cmd);
  fflush(stdout);
}

// Prints a start up message for background processes
// leaving as it is
void print_job_bg_start(int job_id, pid_t pid, const char *cmd)
{
  printf("Background job started: ");
  print_job(job_id, pid, cmd);
}

// Prints a completion message followed by the print job
// leaving as it is
void print_job_bg_complete(int job_id, pid_t pid, const char *cmd)
{
  printf("Completed: \t");
  print_job(job_id, pid, cmd);
}

/***************************************************************************
 * Functions to process commands
 ***************************************************************************/
// Run a program reachable by the path environment variable, relative path, or
// absolute path
void run_generic(GenericCommand cmd)
{
  // Execute a program with a list of arguments. The `args` array is a NULL
  // terminated (last string is always NULL) list of strings. The first element
  // in the array is the executable
  char *exec = cmd.args[0];
  char **args = cmd.args;

  // TODO: Remove warning silencers
  //(void)exec; // Silence unused variable warning
  //(void)args; // Silence unused variable warning

  // TODO: Implement run generic
  // IMPLEMENT_ME();
  execvp(exec, args);
  perror("ERROR: Failed to execute program");
}

// Print strings
void run_echo(EchoCommand cmd)
{
  // Print an array of strings. The args array is a NULL terminated (last
  // string is always NULL) list of strings.
  // implementing echo - mark
  // IMPLEMENT_ME();

  char **str = cmd.args;

  for (int j = 0; str[j] != 0; j++)
  {
    printf("%s ",str[j]);
  }
  // TODO: Remove warning silencers
  //(void)str; // Silence unused variable warning

  // Flush the buffer before returning
  fflush(stdout);
}

// Sets an environment variable
void run_export(ExportCommand cmd)
{
  // Write an environment variable
  // implementing export - mark
  const char *env_var = cmd.env_var;
  const char *val = cmd.val;

  // TODO: Remove warning silencers
  //(void)env_var; // Silence unused variable warning
  //(void)val;     // Silence unused variable warning

  // TODO: Implement export.
  // HINT: This should be quite simple.
  // IMPLEMENT_ME();
  setenv(env_var, val, 1);
}

// Changes the current working directory
void run_cd(CDCommand cmd)
{
  // Get the directory name
  //  Check if the directory is valid
  if (cmd.dir == NULL)
  {
    perror("ERROR: Failed to get path");
    return;
  }
  // change to the old directory
  if (cmd.dir == -1)
  {
    perror("ERROR: ");
    return;
  }

  if (setenv("PWD", cmd.dir, 1) == -1)
  {
    perror("ERROR: ");
    return;
  }

  if (setenv("OLDPWD", lookup_env("PWD"), 1) == -1)
  {
    perror("ERROR: ");
    return;
  }

  // varCD = get_current_dir_name();
  // setenv("PWD", varCD, 1);
  // free(varCD);

  // //fflush(stdout);
  // TODO: Change directory

  // TODO: Update the PWD environment variable to be the new current working
  // directory and optionally update OLD_PWD environment variable to be the old
  // working directory.
  // IMPLEMENT_ME();
}

// Sends a signal to all processes contained in a job
void run_kill(KillCommand cmd)
{
  int signal = cmd.sig;
  int job_id = cmd.job;

  struct theJob currJob;

  // iterate of length of job q
  for (int m = 0; m < length_job_queue(&j_q); m++)
  {
    // pop job from the front of q
    currJob = pop_front_job_queue(&j_q);

    if (currJob.jobID == job_id)
    {
      pid_queue currPq = currJob.p_q;

      while (length_pid_queue(&currPq) != 0)
      {
        pid_t currentPid = pop_front_pid_queue(&currPq);
        // kill the job process
        kill(currentPid, signal);
      }
    }

    push_back_job_queue(&j_q, currJob);
  }

  // TODO: Remove warning silencers
  //(void)signal; // Silence unused variable warning
  //(void)job_id; // Silence unused variable warning
  // TODO: Kill all processes associated with a background job
  // IMPLEMENT_ME();
}

// Prints the current working directory to stdout
void run_pwd()
{
  // TODO: Print the current working directory
  // IMPLEMENT_ME();
  char currentWorkingDirectory[2000];

  printf("%s\n", getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory)));

  // Flush the buffer before returning
  fflush(stdout);
}

// Prints all background jobs currently in the job list to stdout
void run_jobs()
{
  // TODO: Print background jobs
  // IMPLEMENT_ME();
  int number = length_job_queue(&j_q);

  // iterate over jobs
  for (int i = 0; i < number ; i++)
  {
    struct theJob currJob = pop_front_job_queue(&j_q);
    // printing job
    print_job(currJob.jobID, currJob.pid, currJob.cmd);
    push_back_job_queue(&j_q, currJob);
  }
  // Flush the buffer before returning
  fflush(stdout);
}

/***************************************************************************
 * Functions for command resolution and process setup
 ***************************************************************************/

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for child processes.
 *
 * This version of the function is tailored to commands that should be run in
 * the child process of a fork.
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void child_run_command(Command cmd)
{
  CommandType type = get_command_type(cmd);

  switch (type)
  {
  case GENERIC:
    run_generic(cmd.generic);
    break;

  case ECHO:
    run_echo(cmd.echo);
    break;

  case PWD:
    run_pwd();
    break;

  case JOBS:
    run_jobs();
    break;

  case EXPORT:
  case CD:
  case KILL:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief A dispatch function to resolve the correct @a Command variant
 * function for the quash process.
 *
 * This version of the function is tailored to commands that should be run in
 * the parent process (quash).
 *
 * @param cmd The Command to try to run
 *
 * @sa Command
 */
void parent_run_command(Command cmd)
{
  CommandType type = get_command_type(cmd);

  switch (type)
  {
  case EXPORT:
    run_export(cmd.export);
    break;

  case CD:
    run_cd(cmd.cd);
    break;

  case KILL:
    run_kill(cmd.kill);
    break;

  case GENERIC:
  case ECHO:
  case PWD:
  case JOBS:
  case EXIT:
  case EOC:
    break;

  default:
    fprintf(stderr, "Unknown command type: %d\n", type);
  }
}

/**
 * @brief Creates one new process centered around the @a Command in the @a
 * CommandHolder setting up redirects and pipes where needed
 *
 * @note Processes are not the same as jobs. A single job can have multiple
 * processes running under it. This function creates a process that is part of a
 * larger job.
 *
 * @note Not all commands should be run in the child process. A few need to
 * change the quash process in some way
 *
 * @param holder The CommandHolder to try to run
 *
 * @sa Command CommandHolder
 */
void create_process(CommandHolder holder, int r)
{
  // Read the flags field from the parser
  // given to us earlier
  bool p_in = holder.flags & PIPE_IN;
  bool p_out = holder.flags & PIPE_OUT;
  bool r_in = holder.flags & REDIRECT_IN;
  bool r_out = holder.flags & REDIRECT_OUT;
  bool r_app = holder.flags & REDIRECT_APPEND;

  // TODO: Remove warning silencers
  // commenting the silencers
  // (void)p_in;  // Silence unused variable warning
  // (void)p_out; // Silence unused variable warning
  // (void)r_in;  // Silence unused variable warning
  // (void)r_out; // Silence unused variable warning
  // (void)r_app; // Silence unused variable warning

  int read_end = (r - 1) % 2;
  int write_end = r % 2;

  if (p_out)
  {
    pipe(pipes[write_end]);
  }

  // using the fork system for creating new process that's called the child process
  pid_t pid = fork();

  push_back_pid_queue(&p_q, pid);
  if (pid == 0)
  {
    // we will be using pipes here now
    // pipes was created in the very top
    if (p_in)
    {
      // dup system creates a constructor for us
      dup2(pipes[read_end][READ_END], STDIN_FILENO);
      close(pipes[read_end][READ_END]);
    }
    if (p_out)
    {
      dup2(pipes[write_end][WRITE_END], STDOUT_FILENO);
      close(pipes[write_end][WRITE_END]);
    }
    if (r_in)
    {
      FILE *file = fopen(holder.redirect_in, "r");
      dup2(fileno(file), STDIN_FILENO);
    }
    if (r_out)
    {
      if (r_app)
      {
        FILE *file = fopen(holder.redirect_out, "a");
        dup2(fileno(file), STDOUT_FILENO);
      }
      else
      {
        FILE *file = fopen(holder.redirect_out, "w");
        dup2(fileno(file), STDOUT_FILENO);
      }
    }

    child_run_command(holder.cmd);
    exit(EXIT_SUCCESS);
  }
  else if (p_out)
  {
    close(pipes[write_end][WRITE_END]);
  }

  else
  {
    parent_run_command(holder.cmd);
  }
}
// Run a list of commands
void run_script(CommandHolder *holders)
{

  if (helper == 0)
  {
    j_q = new_job_queue(1);
    helper = 1;
  }

  p_q = new_pid_queue(1);

  if (holders == NULL)
    return;

  check_jobs_bg_status();

  if (get_command_holder_type(holders[0]) == EXIT &&
      get_command_holder_type(holders[1]) == EOC)
  {
    end_main_loop();
    return;
  }

  CommandType type;

  // Run all commands in the `holder` array
  for (int i = 0; (type = get_command_holder_type(holders[i])) != EOC; ++i)
    create_process(holders[i], i);

  if (!(holders[0].flags & BACKGROUND))
  {
    // Not a background Job
    // TODO: Wait for all processes under the job to complete
    // IMPLEMENT_ME();

    while (!is_empty_pid_queue(&p_q))
    {
      pid_t currentPid = pop_front_pid_queue(&p_q);
      int state;

      // returns the status on the immediate child process
      waitpid(currentPid, &state, 0);
    }

    destroy_pid_queue(&p_q);
  }
  else
  {
    // A background job.
    // TODO: Push the new job to the job queue
    // IMPLEMENT_ME();
    struct theJob currentJob;
    currentJob.jobID = numOfJob;

    numOfJob++;

    currentJob.p_q = p_q;
    currentJob.pid = peek_back_pid_queue(&p_q);
    currentJob.cmd = get_command_string();

    // TODO: Once jobs are implemented, uncomment and fill the following line
    // print_job_bg_start(job_id, pid, cmd);
    push_back_job_queue(&j_q, currentJob);
    print_job_bg_start(currentJob.jobID, currentJob.pid, currentJob.cmd);
  }
}
