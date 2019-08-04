#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

//Customized
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/process.h"
#include "list.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "devices/input.h"
#include "threads/synch.h"
///Customized

static void syscall_handler (struct intr_frame *f);
static int fid_val=2;

//Customized
struct filedesc_elem
{
  int file_desc;
  struct file *file;
  struct list_elem elem;
  struct list_elem thread_elem;
};
///Customized

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  //Customized
  list_init(&fl);
  ///Customized
}

static void
syscall_handler (struct intr_frame *f) 
{
  //Customized
  int *sp=f->esp;
  if(sp<PHYS_BASE && !(*sp < SYS_HALT || *sp > SYS_INUMBER)) {
    if(((sp+1)<PHYS_BASE) && ((sp+2)<PHYS_BASE) && ((sp+3)<PHYS_BASE)) {
	switch(*sp)
        {
          case SYS_CREATE:
            f->eax = system_create(*(sp+1), *(sp+2));
            break;

          case SYS_OPEN:
            f->eax = system_open(*(sp+1));
	    break;

	  case SYS_SEEK:
            f->eax = system_seek(*(sp+1), *(sp+2));
	    break;

	  case SYS_FILESIZE:
            f->eax = system_filesize(*(sp+1));
	    break;

	  case SYS_TELL:
            f->eax = system_tell(*(sp+1));
	    break;

	  case SYS_READ:
            f->eax = system_read(*(sp+1), *(sp+2), *(sp+3));
	    break;

	  case SYS_WRITE:
            f->eax = system_write(*(sp+1), *(sp+2), *(sp+3));
	    break;

	  case SYS_EXEC:
            f->eax = system_exec(*(sp+1));
	    break;

	  case SYS_WAIT:
            f->eax = system_wait(*(sp+1));
	    break;

	  case SYS_REMOVE:
            f->eax = system_remove(*(sp+1));
	    break;

	  case SYS_CLOSE:
            f->eax = system_close(*(sp+1));
	    break;

	  case SYS_EXIT:
            f->eax = system_exit(*(sp+1));
	    break;
        }
      return 0;
    }
  }
  ///Customized
}

//Customized
int
system_create(char *file,int init_size)
{
  if(file)
    return filesys_create(file,init_size);
  else
    return system_exit(-1);
}

int
system_open(char *file)
{
  struct file *file1;
  struct filedesc_elem *file_desc_elem;
  if(file)
  {
    file1=filesys_open(file);
    if(file1)
    {
	file_desc_elem=(struct filedesc_elem *)malloc(sizeof(struct filedesc_elem));
  	if(file_desc_elem)
  	{
	     file_desc_elem->file=file1;
  	     file_desc_elem->file_desc=fid_val++;
             list_push_back(&fl, &file_desc_elem->elem);
  	     list_push_back(&thread_current()->files,&file_desc_elem->thread_elem);
  	     return file_desc_elem->file_desc;
	}
	else
        {
    	  file_close(file1);
          return -1;
  	}
     }
     else
      return -1;
  }
  else
    return -1;
  if(!(file<PHYS_BASE))
    system_exit(-1);
}

struct file *
find_file_by_file_desc(int file_desc)
{
  struct filedesc_elem *t=find_filedesc_elem_by_file_desc(file_desc);
  if(t)
    return t->file;
  else
    return NULL; 
}

int
system_seek(int file_desc,int position)
{
  struct file *file=find_file_by_file_desc(file_desc);
  if(file)
    file_seek(file,position);
  else
    return -1;
  return 0;
}

int
system_filesize(int file_desc)
{
  struct file *file=find_file_by_file_desc(file_desc);
  if(file)
  {
    int len=file_length(file);
    return len;
  }
  else
    return -1;
}

int
system_tell(int file_desc)
{
  struct file *file=find_file_by_file_desc(file_desc);
  if(file)
  {
    int val=file_tell(file);
    return val;
  }
  else
    return -1;
}

int
system_read(int file_desc,void *buf,int size)
{
  if(file_desc==STDOUT_FILENO)
    return -1;
  else if(!(buf<PHYS_BASE) || !((buf+size)<PHYS_BASE))
    system_exit(-1);
  else
  {
    struct file *file=find_file_by_file_desc(file_desc);
    if(file)
      return file_read(file,buf,size);
  }
}

struct filedesc_elem *
find_filedesc_elem_by_file_desc(int file_desc)
{
  struct list_elem *l=list_begin(&fl);
  while(l!=list_end(&fl))
  { 
    struct filedesc_elem *t=list_entry(l,struct filedesc_elem,elem);
    if(t->file_desc==file_desc)
      return t;
    l=list_next(l);
  } 
  return NULL;
}

int
system_write(int file_desc,void *buf,int len)
{
  if(file_desc==STDIN_FILENO)
    return -1;
  else if(file_desc==STDOUT_FILENO)
    putbuf(buf,len);
  else if(!(buf<PHYS_BASE) || !((buf+len)<PHYS_BASE))
    system_exit(-1);
  else
  {
    struct file * file=find_file_by_file_desc(file_desc);
    if(file)
      return file_write(file,buf,len);
  }
  return -1;
}

int
system_exec(char *command)
{
  return process_execute(command);
}

int
system_wait(int parent_id)
{
  int v = -1;
  struct thread *thr=get_thread_by_tid(parent_id);
  if(thr->return_s!=0xcdcdcdcd && thr->return_s!=-9999)
    return thr->return_s;
  if(thr->return_s==-9999)
    return v;
  sema_down(&thr->wait);
  v=thr->return_s;
  printf("%s: exit(%d)\n",thr->name,v);
  while(thr->status == THREAD_BLOCKED)
    thread_unblock(thr);
  thr->return_s=-9999;
  return v;
}

int
system_close(int file_desc)
{
  struct filedesc_elem *file=find_filedesc_elem_by_file_desc_in_process(file_desc);
  if(file)
  {
    list_remove(&file->elem);
    list_remove(&file->thread_elem);
    file_close(file->file);
    free(file);
  }
  return 0;
}

struct filedesc_elem *
find_filedesc_elem_by_file_desc_in_process(int file_desc)
{
  struct thread *t=thread_current();
  struct list_elem *l=list_begin(&t->files);
  while(l!=list_end(&t->files))
  {  
    struct filedesc_elem *t=list_entry(l,struct filedesc_elem,thread_elem);
    if(t->file_desc==file_desc)
      return t;
    l=list_next(l);
  } 
  return NULL;
}

int
system_remove(char *file)
{
  if(file<PHYS_BASE)
  {
    int val=filesys_remove(file);
    return val;
  }
  else
    system_exit(-1);
}

int
system_exit(int s)
{
  struct thread *thr=thread_current();
  while(!list_empty(&thr->files))
    system_close(list_entry(list_begin(&thr->files),struct filedesc_elem,thread_elem)->file_desc);
  thr->return_s=s;
  thread_exit();
  return -1;
}
